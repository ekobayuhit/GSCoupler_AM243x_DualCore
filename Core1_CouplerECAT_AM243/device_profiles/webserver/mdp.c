/**
@file    mdp.c
@brief   Modular Device Profile (MDP) Implementation for GP-EtherCAT-Coupler

ARCHITECTURE NOTE
-----------------
The TI ecSlvApi requires ALL CoE objects (including PDO mapping objects
0x1600/0x1A00 and application objects 0x2200/0x2300/0x2400/0x2500) to be
created during the one-shot population phase, before EC_API_SLV_init() is
called.

Four object families are registered per slot:
  0x2300+i  Digital outputs  (16 × BOOL,   RxPDO-mappable)  DO16
  0x2400+i  Analog  outputs  ( 8 × UINT16, RxPDO-mappable)  AOC8/AOV8
  0x2200+i  Digital inputs   (16 × BOOL,   TxPDO-mappable)  DI16
  0x2500+i  Analog  inputs   (12 × UINT16, TxPDO-mappable)  AIC8/AIV8/RTDY/RTDB

PDO entries are wired to the correct family based on the detected module ident
at MDP_CreateObjects() time.  MDP_OnMappingChanged() validates that the master
maps the correct object index for the configured ident.
*/

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <ecSlvApi.h>
#include <defines/ecSlvApiDef.h>
#include <ESL_os.h>
#include "project.h"
#include "ipc_shareMem.h"
#include "mdp.h"

/* =========================================================================
   WIRE DESCRIPTOR TABLE  –  single source of truth for per-module PDO layout
   ========================================================================= */
typedef struct {
    uint32_t ident;
    uint8_t  outEntries;    /* RxPDO entries (0 = no output)          */
    uint8_t  outBits;       /* bits per output entry                  */
    uint16_t outObjBase;    /* CoE object base for output data        */
    uint8_t  inEntries;     /* TxPDO entries (0 = no input)           */
    uint8_t  inBits;        /* bits per input entry                   */
    uint16_t inObjBase;     /* CoE object base for input data         */
    uint16_t ioBytes;       /* total bytes for I/O data               */
} MDP_WireDesc_t;

static const MDP_WireDesc_t s_wireTable[] = {
    /* ident             out  bits  outBase                    in  bits  inBase                   */
    { MODULE_IDENT_DO16, 16,  1,   OBJ_SLOT_DIG_OUTPUT_BASE,  0,  0,   0,                        2  },
    { MODULE_IDENT_DI16,  0,  0,   0,                         16,  1,   OBJ_SLOT_DIG_INPUT_BASE, 2  },
    { MODULE_IDENT_AIC8,  0,  0,   0,                          8, 16,   OBJ_SLOT_ANL_INPUT_BASE, 16 },
    { MODULE_IDENT_AIV8,  0,  0,   0,                          8, 16,   OBJ_SLOT_ANL_INPUT_BASE, 16 },
    { MODULE_IDENT_AOC8,  8, 16,   OBJ_SLOT_ANL_OUTPUT_BASE,   0,  0,   0,                       16 },
    { MODULE_IDENT_AOV8,  8, 16,   OBJ_SLOT_ANL_OUTPUT_BASE,   0,  0,   0,                       16 },
    { MODULE_IDENT_RTDY,  0,  0,   0,                         12, 16,   OBJ_SLOT_ANL_INPUT_BASE, 24 },
    { MODULE_IDENT_RTDB,  0,  0,   0,                         12, 16,   OBJ_SLOT_ANL_INPUT_BASE, 24 },
};
#define WIRE_TABLE_SIZE  (sizeof(s_wireTable) / sizeof(s_wireTable[0]))

/* =========================================================================
   DATA TYPES
   ========================================================================= */
typedef struct {
    uint32_t detected_ident;
    uint32_t configured_ident;
    uint16_t input_bytes;
    uint16_t output_bytes;
    uint16_t input_offset;
    uint16_t output_offset;
    bool     present;
    bool     configured;
} SlotInfo_t;

typedef struct {
    uint8_t  sub0;
    uint16_t channel[MAX_SLOT_CHANNELS];
} __attribute__((packed)) SlotInputData_t;

typedef struct {
    uint8_t  sub0;
    uint16_t channel[MAX_SLOT_CHANNELS];
} __attribute__((packed)) SlotOutputData_t;

typedef struct {
    uint8_t  count;
    uint32_t ident[MAX_SLOTS];
} MdpIdentList_t;

typedef struct {
    SlotInfo_t       slot[MAX_SLOTS];
    SlotInputData_t  input_data[MAX_SLOTS];
    SlotOutputData_t output_data[MAX_SLOTS];
    MdpIdentList_t   configured_list;
    MdpIdentList_t   detected_list;
    uint8_t          al_state;
    uint16_t         al_status_code;
    uint16_t         total_input_bytes;
    uint16_t         total_output_bytes;
} CouplerMDP_t;

/* =========================================================================
   GLOBAL STATE & HAL POINTERS
   ========================================================================= */
static CouplerMDP_t          g_mdp;
static EC_API_SLV_SHandle_t *ptSubDevice;

/* Forward declarations */
static uint32_t MDP_SDO_Read(uint16_t index, uint8_t subindex, uint8_t *buf,
                              uint32_t *size, uint8_t completeAccess);
static uint32_t MDP_SDO_Write(uint16_t index, uint8_t subindex,
                               const uint8_t *buf, uint32_t size,
                               uint8_t completeAccess);
static void     MDP_GetModuleSizes(uint32_t ident, uint16_t *input_bytes,
                                   uint16_t *output_bytes);
static void     MDP_DetectModules(void);

extern volatile ipc_data_t gSharedMem;

/* =========================================================================
   WIRE TABLE HELPERS
   ========================================================================= */
static const MDP_WireDesc_t* MDP_GetWireDesc(uint32_t ident)
{
    uint8_t i;
    for (i = 0; i < WIRE_TABLE_SIZE; i++) {
        if (s_wireTable[i].ident == ident) return &s_wireTable[i];
    }
    return NULL;
}

static uint8_t MDP_GetOutputSubCount(uint32_t ident)
{
    const MDP_WireDesc_t *w = MDP_GetWireDesc(ident);
    return (w != NULL) ? w->outEntries : 0u;
}

static uint8_t MDP_GetInputSubCount(uint32_t ident)
{
    const MDP_WireDesc_t *w = MDP_GetWireDesc(ident);
    return (w != NULL) ? w->inEntries : 0u;
}

/**
 * @brief Returns the CoE object base index for the data object that holds
 *        PDO process data for a given module ident and direction.
 *
 *  isOutput=true  → object written by master (RxPDO side)
 *  isOutput=false → object written by slave  (TxPDO side)
 *
 * Returns 0 if the module has no data in that direction.
 */
static uint16_t MDP_GetExpectedObjBase(uint32_t ident, bool isOutput)
{
    const MDP_WireDesc_t *w = MDP_GetWireDesc(ident);
    if (w == NULL) return 0u;
    return isOutput ? w->outObjBase : w->inObjBase;
}

/**
 * @brief Returns the bit-width of each PDO entry for a given module ident
 *        and direction.  Used to encode the mapping entry in SDO read and
 *        to compute SM byte sizes.
 */
static uint8_t MDP_GetEntryBits(uint32_t ident, bool isOutput)
{
    const MDP_WireDesc_t *w = MDP_GetWireDesc(ident);
    if (w == NULL) return 0u;
    return isOutput ? w->outBits : w->inBits;
}

/* =========================================================================
   HARDWARE ABSTRACTION LAYER (HAL)
   ========================================================================= */
uint32_t HAL_ReadSlotIdent(uint8_t slot_index)
{
    uint8_t productCode = 0;

    app_ipc_sharemem_lock();
    productCode = gSharedMem.IOCoupler_Devices.slaveInfo[slot_index].productCode;
    app_ipc_sharemem_unlock();

    switch (productCode) {
        case IO_DEVICE_TYPE_DO16: return MODULE_IDENT_DO16;
        case IO_DEVICE_TYPE_DI16: return MODULE_IDENT_DI16;
        case IO_DEVICE_TYPE_AIC8: return MODULE_IDENT_AIC8;
        case IO_DEVICE_TYPE_AIV8: return MODULE_IDENT_AIV8;
        case IO_DEVICE_TYPE_AOC8: return MODULE_IDENT_AOC8;
        case IO_DEVICE_TYPE_AOV8: return MODULE_IDENT_AOV8;
        case IO_DEVICE_TYPE_RTDY: return MODULE_IDENT_RTDY;
        case IO_DEVICE_TYPE_RTDB: return MODULE_IDENT_RTDB;
        default:                  return MODULE_IDENT_EMPTY;
    }
}

uint32_t HAL_ReadIOIdent(uint8_t productCode)
{
    switch (productCode) {
        case IO_DEVICE_TYPE_DO16: return MODULE_IDENT_DO16;
        case IO_DEVICE_TYPE_DI16: return MODULE_IDENT_DI16;
        case IO_DEVICE_TYPE_AIC8: return MODULE_IDENT_AIC8;
        case IO_DEVICE_TYPE_AIV8: return MODULE_IDENT_AIV8;
        case IO_DEVICE_TYPE_AOC8: return MODULE_IDENT_AOC8;
        case IO_DEVICE_TYPE_AOV8: return MODULE_IDENT_AOV8;
        case IO_DEVICE_TYPE_RTDY: return MODULE_IDENT_RTDY;
        case IO_DEVICE_TYPE_RTDB: return MODULE_IDENT_RTDB;
        default:                  return MODULE_IDENT_EMPTY;
    }
}

void HAL_Set_ptSubDevice(EC_API_SLV_SHandle_t *handle)
{
    ptSubDevice = handle;
}

static void HAL_SetSM2Size(uint16_t size_bytes)
{
    EC_API_EError_t error = (EC_API_EError_t)EC_API_SLV_setSyncManConfig(
        ptSubDevice, 2, EC_OUTPUT_START, size_bytes,
        EC_OUTPUT_CONTROLREG, EC_OUTPUT_ENABLE);
    if (error != EC_API_eERR_NONE) {
        OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
    }
    OSAL_printf("HAL_SetSM2Size: %u bytes result=0x%08X\r\n",
                (unsigned)size_bytes, (unsigned)error);
}

static void HAL_SetSM3Size(uint16_t size_bytes)
{
    EC_API_EError_t error = (EC_API_EError_t)EC_API_SLV_setSyncManConfig(
        ptSubDevice, 3, EC_INPUT_START, size_bytes,
        EC_INPUT_CONTROLREG, EC_INPUT_ENABLE);
    if (error != EC_API_eERR_NONE) {
        OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
    }
    OSAL_printf("HAL_SetSM3Size: %u bytes result=0x%08X\r\n",
                (unsigned)size_bytes, (unsigned)error);
}

/* =========================================================================
   SDO CALLBACK WRAPPERS  (called by the EtherCAT stack)
   ========================================================================= */
uint8_t EC_SLV_APP_MDP_SDO_Read(
    void *pContext_p, uint16_t index_p, uint8_t subIndex_p,
    uint32_t size_p, uint16_t MBXMEM *pData_p, uint8_t completeAccess_p)
{
    uint32_t actualSize = size_p;
    OSALUNREF_PARM(pContext_p);
    return (uint8_t)MDP_SDO_Read(index_p, subIndex_p, (uint8_t *)pData_p,
                                  &actualSize, completeAccess_p);
}

uint8_t EC_SLV_APP_MDP_SDO_Write(
    void *pContext_p, uint16_t index_p, uint8_t subIndex_p,
    uint32_t size_p, uint16_t MBXMEM *pData_p, uint8_t completeAccess_p)
{
    OSALUNREF_PARM(pContext_p);
    return (uint8_t)MDP_SDO_Write(index_p, subIndex_p, (const uint8_t *)pData_p,
                                   size_p, completeAccess_p);
}

/* =========================================================================
   OBJECT CREATION  –  called ONCE during application init, before stack start

   Object map
   ----------
   0xF000   Modular Device Profile  (VAR, UINT16, read-only)
   0xF008   Code Word               (VAR, UINT32, read-only)
   0xF010   Module Profile List     (RECORD: sub0=UINT8, sub1..N=UINT32)
   0xF030   Configured Module Ident List (RECORD: sub0=UINT8, sub1..N=UINT32, r/w)
   0xF050   Detected  Module Ident List  (RECORD: sub0=UINT8, sub1..N=UINT32, r/o)

   Per slot i (0 .. MAX_SLOTS-1):
   0x2300+i  Digital output data   (RECORD: 16 × BOOL,   RxPDO-mappable)
   0x2400+i  Analog  output data   (RECORD:  8 × UINT16, RxPDO-mappable)
   0x2200+i  Digital input  data   (RECORD: 16 × BOOL,   TxPDO-mappable)
   0x2500+i  Analog  input  data   (RECORD: 12 × UINT16, TxPDO-mappable)
   0x1600+i  RxPDO wired to the correct output object for the detected module
   0x1A00+i  TxPDO wired to the correct input  object for the detected module
   ========================================================================= */
void MDP_CreateObjects(void)
{
    uint8_t  i, sub;
    char     name[64];
    uint32_t error;
    EC_API_SLV_SCoE_Object_t   *ptObj      = NULL;
    EC_API_SLV_SCoE_ObjEntry_t *ptObjEntry = NULL;
    EC_API_SLV_Pdo_t           *ptPdo      = NULL;

    /* ------------------------------------------------------------------
       0xF000 – Modular Device Profile
       ------------------------------------------------------------------ */
    EC_API_SLV_CoE_odAddVariable(
        ptSubDevice, OBJ_F000, "Modular Device Profile",
        DEFTYPE_UNSIGNED16, 16, ACCESS_READ,
        EC_SLV_APP_MDP_SDO_Read, NULL, NULL, NULL);

    /* ------------------------------------------------------------------
       0xF010 – Module Profile List
       ------------------------------------------------------------------ */
    error = (uint32_t)EC_API_SLV_CoE_odAddRecord(
        ptSubDevice, OBJ_F010, "Module Profile List",
        EC_SLV_APP_MDP_SDO_Read, NULL, NULL, NULL, &ptObj);
    if (error == EC_API_eERR_NONE) {
        EC_API_SLV_CoE_configRecordSubIndex(ptSubDevice, ptObj, 0,
            "Number of entries", DEFTYPE_UNSIGNED8, 8, ACCESS_READ);
        for (i = 1; i <= MAX_SLOTS; i++) {
            snprintf(name, sizeof(name), "Module %u Profile", (unsigned)i);
            EC_API_SLV_CoE_configRecordSubIndex(ptSubDevice, ptObj, i, name,
                DEFTYPE_UNSIGNED32, 32, ACCESS_READ);
        }
    }

    /* ------------------------------------------------------------------
       0xF030 – Configured Module Ident List  (master writes here in PreOP)
       ------------------------------------------------------------------ */
    error = (uint32_t)EC_API_SLV_CoE_odAddRecord(
        ptSubDevice, OBJ_F030, "Configured Module Ident List",
        EC_SLV_APP_MDP_SDO_Read, NULL,
        EC_SLV_APP_MDP_SDO_Write, NULL, &ptObj);
    if (error == EC_API_eERR_NONE) {
        EC_API_SLV_CoE_configRecordSubIndex(ptSubDevice, ptObj, 0,
            "Number of entries", DEFTYPE_UNSIGNED8, 8, ACCESS_READWRITE);
        for (i = 1; i <= MAX_SLOTS; i++) {
            snprintf(name, sizeof(name), "Module %u Ident", (unsigned)i);
            EC_API_SLV_CoE_configRecordSubIndex(ptSubDevice, ptObj, i, name,
                DEFTYPE_UNSIGNED32, 32, ACCESS_READWRITE);
        }
    }

    /* ------------------------------------------------------------------
       0xF050 – Detected Module Ident List  (read-only)
       ------------------------------------------------------------------ */
    error = (uint32_t)EC_API_SLV_CoE_odAddRecord(
        ptSubDevice, OBJ_F050, "Detected Module Ident List",
        EC_SLV_APP_MDP_SDO_Read, NULL, NULL, NULL, &ptObj);
    if (error == EC_API_eERR_NONE) {
        EC_API_SLV_CoE_configRecordSubIndex(ptSubDevice, ptObj, 0,
            "Number of entries", DEFTYPE_UNSIGNED8, 8, ACCESS_READ);
        for (i = 1; i <= MAX_SLOTS; i++) {
            snprintf(name, sizeof(name), "Module %u Ident", (unsigned)i);
            EC_API_SLV_CoE_configRecordSubIndex(ptSubDevice, ptObj, i, name,
                DEFTYPE_UNSIGNED32, 32, ACCESS_READ);
        }
    }

    /* ------------------------------------------------------------------
       Per-slot objects + PDO wiring
       ------------------------------------------------------------------ */
    for (i = 0; i < MAX_SLOTS; i++) {

        /* ---- 0x2300+i : Digital outputs — 16 × BOOL ---- */
        snprintf(name, sizeof(name), "Slot%u DigOut", (unsigned)i);
        error = (uint32_t)EC_API_SLV_CoE_odAddRecord(
            ptSubDevice, OBJ_SLOT_DIG_OUTPUT_BASE + i, name,
            EC_SLV_APP_MDP_SDO_Read, NULL,
            EC_SLV_APP_MDP_SDO_Write, NULL, &ptObj);
        if (error == EC_API_eERR_NONE) {
            for (sub = 1; sub <= 16u; sub++) {
                snprintf(name, sizeof(name), "Ch%u", (unsigned)sub);
                EC_API_SLV_CoE_configRecordSubIndex(ptSubDevice, ptObj, sub,
                    name, DEFTYPE_BOOLEAN, 1,
                    ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING);
            }
        }

        /* ---- 0x2400+i : Analog outputs — 8 × UINT16 ---- */
        snprintf(name, sizeof(name), "Slot%u AnlOut", (unsigned)i);
        error = (uint32_t)EC_API_SLV_CoE_odAddRecord(
            ptSubDevice, OBJ_SLOT_ANL_OUTPUT_BASE + i, name,
            EC_SLV_APP_MDP_SDO_Read, NULL,
            EC_SLV_APP_MDP_SDO_Write, NULL, &ptObj);
        if (error == EC_API_eERR_NONE) {
            for (sub = 1; sub <= 8u; sub++) {
                snprintf(name, sizeof(name), "Ch%u", (unsigned)sub);
                EC_API_SLV_CoE_configRecordSubIndex(ptSubDevice, ptObj, sub,
                    name, DEFTYPE_UNSIGNED16, 16,
                    ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING);
            }
        }

        /* ---- 0x2200+i : Digital inputs — 16 × BOOL ---- */
        snprintf(name, sizeof(name), "Slot%u DigIn", (unsigned)i);
        error = (uint32_t)EC_API_SLV_CoE_odAddRecord(
            ptSubDevice, OBJ_SLOT_DIG_INPUT_BASE + i, name,
            EC_SLV_APP_MDP_SDO_Read, NULL, NULL, NULL, &ptObj);
        if (error == EC_API_eERR_NONE) {
            for (sub = 1; sub <= 16u; sub++) {
                snprintf(name, sizeof(name), "Ch%u", (unsigned)sub);
                EC_API_SLV_CoE_configRecordSubIndex(ptSubDevice, ptObj, sub,
                    name, DEFTYPE_BOOLEAN, 1,
                    ACCESS_READ | OBJACCESS_TXPDOMAPPING);
            }
        }

        /* ---- 0x2500+i : Analog inputs — 12 × UINT16 (RTD max) ---- */
        snprintf(name, sizeof(name), "Slot%u AnlIn", (unsigned)i);
        error = (uint32_t)EC_API_SLV_CoE_odAddRecord(
            ptSubDevice, OBJ_SLOT_ANL_INPUT_BASE + i, name,
            EC_SLV_APP_MDP_SDO_Read, NULL, NULL, NULL, &ptObj);
        if (error == EC_API_eERR_NONE) {
            for (sub = 1; sub <= 12u; sub++) {
                snprintf(name, sizeof(name), "Ch%u", (unsigned)sub);
                EC_API_SLV_CoE_configRecordSubIndex(ptSubDevice, ptObj, sub,
                    name, DEFTYPE_UNSIGNED16, 16,
                    ACCESS_READ | OBJACCESS_TXPDOMAPPING);
            }
        }

        /* ---- Wire PDOs based on detected module ident ---- */
        uint32_t              detIdent = g_mdp.detected_list.ident[i];
        const MDP_WireDesc_t *wire     = MDP_GetWireDesc(detIdent);

        /* 0x1600+i : RxPDO — output (master→slave) */
        snprintf(name, sizeof(name), "RxPDO Slot%u", (unsigned)i);
        error = (uint32_t)EC_API_SLV_PDO_create(
            ptSubDevice, name, OBJ_RXPDO_BASE + i, &ptPdo);
        if (error == EC_API_eERR_NONE && ptPdo != NULL &&
            wire != NULL && wire->outEntries > 0u)
        {
            for (sub = 1u; sub <= wire->outEntries; sub++) {
                if (EC_API_SLV_CoE_getObjectEntry(ptSubDevice,
                        wire->outObjBase + i, sub, &ptObjEntry) == EC_API_eERR_NONE)
                {
                    snprintf(name, sizeof(name), "Slot%u Out%u", (unsigned)i, (unsigned)sub);
                    EC_API_SLV_PDO_createEntry(ptSubDevice, ptPdo, name, ptObjEntry);
                }
            }
        }

        /* 0x1A00+i : TxPDO — input (slave→master) */
        snprintf(name, sizeof(name), "TxPDO Slot%u", (unsigned)i);
        error = (uint32_t)EC_API_SLV_PDO_create(
            ptSubDevice, name, OBJ_TXPDO_BASE + i, &ptPdo);
        if (error == EC_API_eERR_NONE && ptPdo != NULL &&
            wire != NULL && wire->inEntries > 0u)
        {
            for (sub = 1u; sub <= wire->inEntries; sub++) {
                if (EC_API_SLV_CoE_getObjectEntry(ptSubDevice,
                        wire->inObjBase + i, sub, &ptObjEntry) == EC_API_eERR_NONE)
                {
                    snprintf(name, sizeof(name), "Slot%u In%u", (unsigned)i, (unsigned)sub);
                    EC_API_SLV_PDO_createEntry(ptSubDevice, ptPdo, name, ptObjEntry);
                }
            }
        }

        OSAL_printf("MDP_CreateObjects: slot %u ident=0x%08X "
                    "RxPDO(%u entries) TxPDO(%u entries)\r\n",
                    (unsigned)i, detIdent,
                    (unsigned)(wire ? wire->outEntries : 0),
                    (unsigned)(wire ? wire->inEntries  : 0));
    }
}

/* =========================================================================
   MODULE PROPERTY LOOKUP
   ========================================================================= */
static void MDP_GetModuleSizes(uint32_t ident, uint16_t *input_bytes,
                                uint16_t *output_bytes)
{
    const MDP_WireDesc_t *w = MDP_GetWireDesc(ident);
    if (w == NULL) {
        *input_bytes  = 0;
        *output_bytes = 0;
        return;
    }
    /* byte size = ceil(entries × bitsPerEntry / 8) */
    *output_bytes = (w->outEntries > 0u)
        ? (uint16_t)(((uint32_t)w->outEntries * w->outBits + 7u) / 8u) : 0u;
    *input_bytes  = (w->inEntries  > 0u)
        ? (uint16_t)(((uint32_t)w->inEntries  * w->inBits  + 7u) / 8u) : 0u;
}

/* =========================================================================
   MODULE DETECTION
   ========================================================================= */
static void MDP_DetectModules(void)
{
    uint8_t count = 0;
    uint8_t i;
    for (i = 0; i < MAX_SLOTS; i++) {
        uint32_t ident = HAL_ReadSlotIdent(i);
        g_mdp.slot[i].detected_ident = ident;
        g_mdp.slot[i].present        = (ident != MODULE_IDENT_EMPTY);
        g_mdp.detected_list.ident[i] = ident;

        if (g_mdp.slot[i].present) {
            g_mdp.input_data[i].sub0  = MDP_GetInputSubCount(ident);
            g_mdp.output_data[i].sub0 = MDP_GetOutputSubCount(ident);
            count = (uint8_t)(i + 1u);
        } else {
            g_mdp.input_data[i].sub0  = 0;
            g_mdp.output_data[i].sub0 = 0;
        }
    }
    g_mdp.detected_list.count = count;
}

/* =========================================================================
   VALIDATE & CONFIGURE  (PreOP → SafeOP)
   ========================================================================= */
bool MDP_ValidateAndConfigure(void)
{
    uint16_t input_offset  = 0;
    uint16_t output_offset = 0;
    uint8_t  cfg_count     = g_mdp.configured_list.count;
    uint8_t  i;

    OSAL_printf("MDP_ValidateAndConfigure: Validating MDP Configuration...\r\n");

    for (i = 0; i < MAX_SLOTS; i++) {
        uint32_t cfg_ident = (i < cfg_count)
                           ? g_mdp.configured_list.ident[i]
                           : MODULE_IDENT_EMPTY;
        uint32_t det_ident = g_mdp.slot[i].detected_ident;

        g_mdp.slot[i].configured_ident = cfg_ident;
        g_mdp.slot[i].configured       = (cfg_ident != MODULE_IDENT_EMPTY);

        if (cfg_ident != MODULE_IDENT_EMPTY && cfg_ident != det_ident) {
            OSAL_printf("MDP_ValidateAndConfigure: Slot %u ident mismatch "
                        "(cfg=0x%08X det=0x%08X)\r\n", i, cfg_ident, det_ident);
            g_mdp.al_status_code = AL_STATUS_MODULE_IDENT_MISMATCH;
            return false;
        }

        uint16_t in_bytes = 0, out_bytes = 0;
        MDP_GetModuleSizes(cfg_ident, &in_bytes, &out_bytes);

        g_mdp.slot[i].input_bytes   = in_bytes;
        g_mdp.slot[i].output_bytes  = out_bytes;
        g_mdp.slot[i].input_offset  = input_offset;
        g_mdp.slot[i].output_offset = output_offset;

        input_offset  += in_bytes;
        output_offset += out_bytes;
    }

    g_mdp.total_input_bytes  = input_offset;
    g_mdp.total_output_bytes = output_offset;

    if (output_offset > 0u) HAL_SetSM2Size(output_offset);
    if (input_offset  > 0u) HAL_SetSM3Size(input_offset);

    g_mdp.al_status_code = AL_STATUS_NO_ERROR;
    OSAL_printf("MDP_ValidateAndConfigure: OK  in=%u out=%u bytes\r\n",
                input_offset, output_offset);
    return true;
}

/* =========================================================================
   PDO MAPPING-CHANGED HOOK
   ========================================================================= */
uint32_t MDP_OnMappingChanged(
    uint16_t pdoIndex_p, uint8_t count_p,
    EC_API_SLV_PDO_SEntryMap_t *pPdoMap_p)
{
    uint8_t  slot;
    bool     isOutputPdo;
    uint32_t cfgIdent;
    uint8_t  expectedCount;
    uint16_t expectedObjIndex;
    uint8_t  idx;

    /* Identify slot and direction */
    if (pdoIndex_p >= OBJ_RXPDO_BASE &&
        pdoIndex_p <  (OBJ_RXPDO_BASE + MAX_SLOTS))
    {
        slot        = (uint8_t)(pdoIndex_p - OBJ_RXPDO_BASE);
        isOutputPdo = true;
    }
    else if (pdoIndex_p >= OBJ_TXPDO_BASE &&
             pdoIndex_p <  (OBJ_TXPDO_BASE + MAX_SLOTS))
    {
        slot        = (uint8_t)(pdoIndex_p - OBJ_TXPDO_BASE);
        isOutputPdo = false;
    }
    else
    {
        return (uint32_t)EC_API_eERR_NONE;   /* not an MDP PDO */
    }

    cfgIdent = g_mdp.configured_list.ident[slot];

    /* Empty slot: master must map zero entries */
    if (cfgIdent == MODULE_IDENT_EMPTY) {
        if (count_p != 0u) {
            OSAL_printf("MDP_OnMappingChanged: slot %u unconfigured but "
                        "PDO 0x%04X maps %u entries\r\n",
                        (unsigned)slot, pdoIndex_p, (unsigned)count_p);
            return (uint32_t)EC_API_eERR_ABORT;
        }
        return (uint32_t)EC_API_eERR_NONE;
    }

    expectedCount    = isOutputPdo ? MDP_GetOutputSubCount(cfgIdent)
                                   : MDP_GetInputSubCount(cfgIdent);
    expectedObjIndex = MDP_GetExpectedObjBase(cfgIdent, isOutputPdo) + slot;

    if (count_p != expectedCount) {
        OSAL_printf("MDP_OnMappingChanged: slot %u ident=0x%08X expects %u "
                    "entries on PDO 0x%04X, got %u\r\n",
                    (unsigned)slot, cfgIdent, (unsigned)expectedCount,
                    pdoIndex_p, (unsigned)count_p);
        return (uint32_t)EC_API_eERR_ABORT;
    }

    if (pPdoMap_p != NULL) {
        for (idx = 0; idx < count_p; idx++) {
            if (pPdoMap_p[idx].index != expectedObjIndex) {
                OSAL_printf("MDP_OnMappingChanged: slot %u entry %u maps "
                            "object 0x%04X, expected 0x%04X\r\n",
                            (unsigned)slot, (unsigned)idx,
                            pPdoMap_p[idx].index, expectedObjIndex);
                return (uint32_t)EC_API_eERR_ABORT;
            }
        }
    }

    g_mdp.slot[slot].configured = true;

    OSAL_printf("MDP_OnMappingChanged: slot %u PDO 0x%04X mapping OK "
                "(%u entries, ident=0x%08X)\r\n",
                (unsigned)slot, pdoIndex_p, (unsigned)count_p, cfgIdent);

    /* Recompute SM2/SM3 sizes from all configured slots */
    {
        uint16_t rxBytes = 0u;
        uint16_t txBytes = 0u;

        for (idx = 0; idx < MAX_SLOTS; idx++) {
            uint32_t ident = g_mdp.configured_list.ident[idx];
            if (ident == MODULE_IDENT_EMPTY) continue;

            uint8_t outSubs  = MDP_GetOutputSubCount(ident);
            uint8_t outBits  = MDP_GetEntryBits(ident, true);
            uint8_t inSubs   = MDP_GetInputSubCount(ident);
            uint8_t inBits   = MDP_GetEntryBits(ident, false);

            if (outSubs > 0u)
                rxBytes += (uint16_t)(((uint32_t)outSubs * outBits + 7u) / 8u);
            if (inSubs > 0u)
                txBytes += (uint16_t)(((uint32_t)inSubs  * inBits  + 7u) / 8u);
        }

        OSAL_printf("MDP_OnMappingChanged: SM sizes — SM2(out)=%u SM3(in)=%u bytes\r\n",
                    (unsigned)rxBytes, (unsigned)txBytes);

        if (rxBytes > 0u) HAL_SetSM2Size(rxBytes);
        if (txBytes > 0u) HAL_SetSM3Size(txBytes);
    }

    return (uint32_t)EC_API_eERR_NONE;
}

/* =========================================================================
   CoE SDO READ HANDLER
   ========================================================================= */
static uint32_t MDP_SDO_Read(uint16_t index, uint8_t subindex,
                              uint8_t *buf, uint32_t *size,
                              uint8_t completeAccess)
{
    OSAL_printf("MDP_SDO_Read: index=0x%04X subindex=%u size=%u ca=%u\r\n",
                index, subindex, (unsigned)*size, (unsigned)completeAccess);

    if (completeAccess != 0u) {
        uint32_t offset;
        uint8_t  i;

        if (index == OBJ_F010) {
            /* Count highest populated slot (SubIndex 0 = highest valid subindex) */
            uint8_t num = 0;
            for (i = 0; i < MAX_SLOTS; i++) {
                if (g_mdp.slot[i].configured) num = i + 1u;
            }

            uint32_t required = 1u + ((uint32_t)num * 4u);
            if (*size < required) return 0x06070010u; /* buffer too small */

            offset = 0;
            buf[offset++] = num;                                      /* SubIndex 0: count */
            for (i = 0; i < num; i++) {
                uint32_t val = g_mdp.slot[i].configured
                               ? g_mdp.configured_list.ident[i]
                               : 0u;
                memcpy(&buf[offset], &val, 4u);
                offset += 4u;
            }
            *size = offset;
            return 0u;
        }

        if (index == OBJ_F030) {
            offset = 0;
            buf[offset++] = g_mdp.configured_list.count;
            for (i = 0; i < MAX_SLOTS; i++) {
                uint32_t val = g_mdp.configured_list.ident[i];
                memcpy(&buf[offset], &val, 4); offset += 4;
            }
            *size = offset; return 0;
        }

        if (index == OBJ_F050) {
            OSAL_printf("MDP_SDO_Read: Detected list count=%u\r\n",
                        (unsigned)g_mdp.detected_list.count);
            offset = 0;
            buf[offset++] = g_mdp.detected_list.count;
            for (i = 0; i < MAX_SLOTS; i++) {
                OSAL_printf("MDP_SDO_Read: Detected list slot %u ident=0x%08X\r\n",
                            (unsigned)i, g_mdp.detected_list.ident[i]);
                uint32_t val = g_mdp.detected_list.ident[i];
                memcpy(&buf[offset], &val, 4); offset += 4;
            }
            *size = offset; return 0;
        }

        /* ---- Digital input data: 0x2200+i ---- */
        if (index >= OBJ_SLOT_DIG_INPUT_BASE &&
            index <  (OBJ_SLOT_DIG_INPUT_BASE + MAX_SLOTS))
        {
            uint8_t slot = (uint8_t)(index - OBJ_SLOT_DIG_INPUT_BASE);
            offset = 0;
            buf[offset++] = g_mdp.input_data[slot].sub0;
            for (i = 0; i < 16u; i++) {
                uint16_t val = g_mdp.input_data[slot].channel[i];
                memcpy(&buf[offset], &val, 2); offset += 2;
            }
            *size = offset; return 0;
        }

        /* ---- Analog input data: 0x2500+i ---- */
        if (index >= OBJ_SLOT_ANL_INPUT_BASE &&
            index <  (OBJ_SLOT_ANL_INPUT_BASE + MAX_SLOTS))
        {
            uint8_t slot = (uint8_t)(index - OBJ_SLOT_ANL_INPUT_BASE);
            offset = 0;
            buf[offset++] = g_mdp.input_data[slot].sub0;
            for (i = 0; i < 12u; i++) {
                uint16_t val = g_mdp.input_data[slot].channel[i];
                memcpy(&buf[offset], &val, 2); offset += 2;
            }
            *size = offset; return 0;
        }

        /* ---- Digital output data: 0x2300+i ---- */
        if (index >= OBJ_SLOT_DIG_OUTPUT_BASE &&
            index <  (OBJ_SLOT_DIG_OUTPUT_BASE + MAX_SLOTS))
        {
            uint8_t slot = (uint8_t)(index - OBJ_SLOT_DIG_OUTPUT_BASE);
            offset = 0;
            buf[offset++] = g_mdp.output_data[slot].sub0;
            for (i = 0; i < 16u; i++) {
                uint16_t val = g_mdp.output_data[slot].channel[i];
                memcpy(&buf[offset], &val, 2); offset += 2;
            }
            *size = offset; return 0;
        }

        /* ---- Analog output data: 0x2400+i ---- */
        if (index >= OBJ_SLOT_ANL_OUTPUT_BASE &&
            index <  (OBJ_SLOT_ANL_OUTPUT_BASE + MAX_SLOTS))
        {
            uint8_t slot = (uint8_t)(index - OBJ_SLOT_ANL_OUTPUT_BASE);
            offset = 0;
            buf[offset++] = g_mdp.output_data[slot].sub0;
            for (i = 0; i < 8u; i++) {
                uint16_t val = g_mdp.output_data[slot].channel[i];
                memcpy(&buf[offset], &val, 2); offset += 2;
            }
            *size = offset; return 0;
        }

        /* ---- RxPDO mapping: 0x1600+i ---- */
        if (index >= OBJ_RXPDO_BASE &&
            index <  (OBJ_RXPDO_BASE + MAX_SLOTS))
        {
            uint8_t  slot     = (uint8_t)(index - OBJ_RXPDO_BASE);
            uint32_t ident    = g_mdp.slot[slot].detected_ident;
            uint8_t  outSubs  = MDP_GetOutputSubCount(ident);
            uint8_t  outBits  = MDP_GetEntryBits(ident, true);
            uint16_t outBase  = MDP_GetExpectedObjBase(ident, true);

            offset = 0;
            buf[offset++] = outSubs;
            for (uint8_t sub = 1u; sub <= MAX_SLOT_CHANNELS; sub++) {
                uint32_t mapping = 0;
                if (sub <= outSubs) {
                    mapping = ((uint32_t)(outBase + slot) << 16) |
                              ((uint32_t)sub              <<  8) |
                              (uint32_t)outBits;
                }
                memcpy(&buf[offset], &mapping, 4); offset += 4;
            }
            *size = offset; return 0;
        }

        /* ---- TxPDO mapping: 0x1A00+i ---- */
        if (index >= OBJ_TXPDO_BASE &&
            index <  (OBJ_TXPDO_BASE + MAX_SLOTS))
        {
            uint8_t  slot    = (uint8_t)(index - OBJ_TXPDO_BASE);
            uint32_t ident   = g_mdp.slot[slot].detected_ident;
            uint8_t  inSubs  = MDP_GetInputSubCount(ident);
            uint8_t  inBits  = MDP_GetEntryBits(ident, false);
            uint16_t inBase  = MDP_GetExpectedObjBase(ident, false);

            offset = 0;
            buf[offset++] = inSubs;
            for (uint8_t sub = 1u; sub <= MAX_SLOT_CHANNELS; sub++) {
                uint32_t mapping = 0;
                if (sub <= inSubs) {
                    mapping = ((uint32_t)(inBase + slot) << 16) |
                              ((uint32_t)sub             <<  8) |
                              (uint32_t)inBits;
                }
                memcpy(&buf[offset], &mapping, 4); offset += 4;
            }
            *size = offset; return 0;
        }
    }

    /* =====================================================================
       Normal per-subindex path
       ===================================================================== */
    if (index == OBJ_F010) {
        uint8_t num = 0;
        for (uint8_t i = 0; i < MAX_SLOTS; i++) {
            if (g_mdp.slot[i].configured) num = i + 1u;
        }

        if (subindex == 0u) {
            buf[0] = num;
            *size  = 1u;
            return 0u;
        }
        if (subindex <= num) {
            uint32_t val = g_mdp.slot[subindex - 1u].configured
                           ? g_mdp.configured_list.ident[subindex - 1u]
                           : 0u;
            memcpy(buf, &val, 4u);
            *size = 4u;
            return 0u;
        }
        return 0x06090011u; /* subindex does not exist */
    }

    if (index == OBJ_F010) {
        if (subindex == 0) { buf[0] = MAX_SLOTS; *size = 1; return 0; }
        if (subindex <= MAX_SLOTS) {
            uint32_t val = 0; memcpy(buf, &val, 4); *size = 4; return 0;
        }
        return 0x06090011u;
    }

    if (index == OBJ_F030) {
        if (subindex == 0) {
            buf[0] = g_mdp.configured_list.count; *size = 1; return 0;
        }
        if (subindex >= 1u && subindex <= MAX_SLOTS) {
            uint32_t val = g_mdp.configured_list.ident[subindex - 1u];
            memcpy(buf, &val, 4); *size = 4; return 0;
        }
        return 0x06090011u;
    }

    if (index == OBJ_F050) {
        if (subindex == 0) {
            buf[0] = g_mdp.detected_list.count; *size = 1; return 0;
        }
        if (subindex >= 1u && subindex <= MAX_SLOTS) {
            uint32_t val = g_mdp.detected_list.ident[subindex - 1u];
            memcpy(buf, &val, 4); *size = 4; return 0;
        }
        return 0x06090011u;
    }

    /* ---- Digital input data: 0x2200+i ---- */
    if (index >= OBJ_SLOT_DIG_INPUT_BASE &&
        index <  (OBJ_SLOT_DIG_INPUT_BASE + MAX_SLOTS))
    {
        uint8_t slot = (uint8_t)(index - OBJ_SLOT_DIG_INPUT_BASE);
        if (subindex == 0) { buf[0] = g_mdp.input_data[slot].sub0; *size = 1; return 0; }
        if (subindex >= 1u && subindex <= 16u) {
            uint16_t val = g_mdp.input_data[slot].channel[subindex - 1u];
            memcpy(buf, &val, 2); *size = 2; return 0;
        }
        return 0x06090011u;
    }

    /* ---- Analog input data: 0x2500+i ---- */
    if (index >= OBJ_SLOT_ANL_INPUT_BASE &&
        index <  (OBJ_SLOT_ANL_INPUT_BASE + MAX_SLOTS))
    {
        uint8_t slot = (uint8_t)(index - OBJ_SLOT_ANL_INPUT_BASE);
        if (subindex == 0) { buf[0] = g_mdp.input_data[slot].sub0; *size = 1; return 0; }
        if (subindex >= 1u && subindex <= 12u) {
            uint16_t val = g_mdp.input_data[slot].channel[subindex - 1u];
            memcpy(buf, &val, 2); *size = 2; return 0;
        }
        return 0x06090011u;
    }

    /* ---- Digital output data: 0x2300+i ---- */
    if (index >= OBJ_SLOT_DIG_OUTPUT_BASE &&
        index <  (OBJ_SLOT_DIG_OUTPUT_BASE + MAX_SLOTS))
    {
        uint8_t slot = (uint8_t)(index - OBJ_SLOT_DIG_OUTPUT_BASE);
        if (subindex == 0) { buf[0] = g_mdp.output_data[slot].sub0; *size = 1; return 0; }
        if (subindex >= 1u && subindex <= 16u) {
            uint16_t val = g_mdp.output_data[slot].channel[subindex - 1u];
            memcpy(buf, &val, 2); *size = 2; return 0;
        }
        return 0x06090011u;
    }

    /* ---- Analog output data: 0x2400+i ---- */
    if (index >= OBJ_SLOT_ANL_OUTPUT_BASE &&
        index <  (OBJ_SLOT_ANL_OUTPUT_BASE + MAX_SLOTS))
    {
        uint8_t slot = (uint8_t)(index - OBJ_SLOT_ANL_OUTPUT_BASE);
        if (subindex == 0) { buf[0] = g_mdp.output_data[slot].sub0; *size = 1; return 0; }
        if (subindex >= 1u && subindex <= 8u) {
            uint16_t val = g_mdp.output_data[slot].channel[subindex - 1u];
            memcpy(buf, &val, 2); *size = 2; return 0;
        }
        return 0x06090011u;
    }

    /* ---- RxPDO mapping: 0x1600+i ---- */
    if (index >= OBJ_RXPDO_BASE &&
        index <  (OBJ_RXPDO_BASE + MAX_SLOTS))
    {
        uint8_t  slot    = (uint8_t)(index - OBJ_RXPDO_BASE);
        uint32_t ident   = g_mdp.slot[slot].detected_ident;
        uint8_t  outSubs = MDP_GetOutputSubCount(ident);
        uint8_t  outBits = MDP_GetEntryBits(ident, true);
        uint16_t outBase = MDP_GetExpectedObjBase(ident, true);

        if (subindex == 0) { buf[0] = outSubs; *size = 1; return 0; }
        if (subindex >= 1u && subindex <= outSubs) {
            uint32_t mapping = ((uint32_t)(outBase + slot) << 16) |
                               ((uint32_t)subindex         <<  8) |
                               (uint32_t)outBits;
            memcpy(buf, &mapping, 4); *size = 4; return 0;
        }
        if (subindex <= MAX_SLOT_CHANNELS) {
            uint32_t zero = 0; memcpy(buf, &zero, 4); *size = 4; return 0;
        }
        return 0x06090011u;
    }

    /* ---- TxPDO mapping: 0x1A00+i ---- */
    if (index >= OBJ_TXPDO_BASE &&
        index <  (OBJ_TXPDO_BASE + MAX_SLOTS))
    {
        uint8_t  slot   = (uint8_t)(index - OBJ_TXPDO_BASE);
        uint32_t ident  = g_mdp.slot[slot].detected_ident;
        uint8_t  inSubs = MDP_GetInputSubCount(ident);
        uint8_t  inBits = MDP_GetEntryBits(ident, false);
        uint16_t inBase = MDP_GetExpectedObjBase(ident, false);

        if (subindex == 0) { buf[0] = inSubs; *size = 1; return 0; }
        if (subindex >= 1u && subindex <= inSubs) {
            uint32_t mapping = ((uint32_t)(inBase + slot) << 16) |
                               ((uint32_t)subindex        <<  8) |
                               (uint32_t)inBits;
            memcpy(buf, &mapping, 4); *size = 4; return 0;
        }
        if (subindex <= MAX_SLOT_CHANNELS) {
            uint32_t zero = 0; memcpy(buf, &zero, 4); *size = 4; return 0;
        }
        return 0x06090011u;
    }

    return 0x06020000u;  /* Object does not exist */
}

/* =========================================================================
   CoE SDO WRITE HANDLER
   ========================================================================= */
static uint32_t MDP_SDO_Write(uint16_t index, uint8_t subindex,
                               const uint8_t *buf, uint32_t size,
                               uint8_t completeAccess)
{
    OSAL_printf("MDP_SDO_Write: index=0x%04X sub=%u size=%u ca=%u\r\n",
                index, (unsigned)subindex, (unsigned)size, (unsigned)completeAccess);

    /* ---- 0xF030 : Configured Module Ident List ---- */
    if (index == OBJ_F030) {
        if (g_mdp.al_state != AL_STATE_PREOP) return 0x08000022u;

        if (completeAccess != 0u) {
            uint8_t  new_count;
            uint32_t avail_idents;
            uint8_t  idents_in_msg;
            uint8_t  i;

            if (size < 2u) return 0x06070010u;

            new_count = buf[0];
            if (new_count > MAX_SLOTS) return 0x06090030u;

            /* count is 2 bytes (uint16_t little-endian) */
            avail_idents  = (size - 2u) / 4u;
            idents_in_msg = (uint8_t)((avail_idents < (uint32_t)new_count)
                                      ? avail_idents : new_count);

            OSAL_printf("MDP_SDO_Write: raw payload [%u bytes]:", (unsigned)size);
            for (uint8_t d = 0; d < size && d < 32u; d++) {
                OSAL_printf(" %02X", buf[d]);
            }
            OSAL_printf("\r\n");

            for (i = 0; i < idents_in_msg; i++) {
                uint32_t ident;
                memcpy(&ident, &buf[2u + ((uint32_t)i * 4u)], 4);
                g_mdp.configured_list.ident[i] = ident;
                g_mdp.slot[i].configured       = (ident != MODULE_IDENT_EMPTY);
                OSAL_printf("MDP_SDO_Write: F030[%u] = 0x%08X\r\n",
                            (unsigned)i, ident);
            }
            for (i = idents_in_msg; i < MAX_SLOTS; i++) {
                g_mdp.configured_list.ident[i] = MODULE_IDENT_EMPTY;
                g_mdp.slot[i].configured       = false;
            }
            g_mdp.configured_list.count = new_count;

            OSAL_printf("MDP_SDO_Write: F030 done, count=%u (%u idents in payload)\r\n",
                        (unsigned)new_count, (unsigned)idents_in_msg);
            return 0;
        }

        /* Non-complete-access: individual subindex */
        if (subindex == 0) {
            if (size < 1u) return 0x06070010u;
            uint8_t new_count = buf[0];
            if (new_count > MAX_SLOTS) return 0x06090030u;
            for (uint8_t i = new_count; i < MAX_SLOTS; i++) {
                g_mdp.configured_list.ident[i] = MODULE_IDENT_EMPTY;
                g_mdp.slot[i].configured       = false;
            }
            g_mdp.configured_list.count = new_count;
            return 0;
        }
        if (subindex >= 1u && subindex <= MAX_SLOTS) {
            if (size < 4u) return 0x06070010u;
            uint32_t ident;
            memcpy(&ident, buf, 4);
            g_mdp.configured_list.ident[subindex - 1u] = ident;
            g_mdp.slot[subindex - 1u].configured       = (ident != MODULE_IDENT_EMPTY);
            return 0;
        }
        return 0x06090011u;
    }

    /* ---- 0x2300+i : Digital output data (SDO write path) ---- */
    if (index >= OBJ_SLOT_DIG_OUTPUT_BASE &&
        index <  (OBJ_SLOT_DIG_OUTPUT_BASE + MAX_SLOTS))
    {
        uint8_t slot = (uint8_t)(index - OBJ_SLOT_DIG_OUTPUT_BASE);
        if (subindex >= 1u && subindex <= 16u) {
            if (size < 2u) return 0x06070010u;
            memcpy(&g_mdp.output_data[slot].channel[subindex - 1u], buf, 2);
            return 0;
        }
        return 0x06090011u;
    }

    /* ---- 0x2400+i : Analog output data (SDO write path) ---- */
    if (index >= OBJ_SLOT_ANL_OUTPUT_BASE &&
        index <  (OBJ_SLOT_ANL_OUTPUT_BASE + MAX_SLOTS))
    {
        uint8_t slot = (uint8_t)(index - OBJ_SLOT_ANL_OUTPUT_BASE);
        if (subindex >= 1u && subindex <= 8u) {
            if (size < 2u) return 0x06070010u;
            memcpy(&g_mdp.output_data[slot].channel[subindex - 1u], buf, 2);
            return 0;
        }
        return 0x06090011u;
    }

    /* Read-only objects */
    if (index == OBJ_F000 ||
        index == OBJ_F010 || 
        index == OBJ_F050)
    {
        return 0x06010002u;  /* Write to read-only object */
    }

    return 0x06020000u;  /* Object does not exist */
}

/* =========================================================================
   STATE MACHINE CALLBACKS
   ========================================================================= */
void MDP_OnEnterInit(void)
{
    g_mdp.al_state       = AL_STATE_INIT;
    g_mdp.al_status_code = ALSTATUSCODE_NOERROR;
    g_mdp.total_input_bytes  = 0;
    g_mdp.total_output_bytes = 0;
    memset(&g_mdp.configured_list, 0, sizeof(g_mdp.configured_list));
    for (uint8_t i = 0; i < MAX_SLOTS; i++) {
        g_mdp.slot[i].configured       = false;
        g_mdp.slot[i].configured_ident = MODULE_IDENT_EMPTY;
    }
}

void MDP_OnEnterPreOP(void)
{
    g_mdp.al_state = AL_STATE_PREOP;
    MDP_DetectModules();
    OSAL_printf("MDP_OnEnterPreOP: %u modules detected.\r\n",
                g_mdp.detected_list.count);
}

bool MDP_OnEnterSafeOP(void)
{
    g_mdp.al_state = AL_STATE_SAFEOP;
    return MDP_ValidateAndConfigure();
}

void MDP_OnEnterOP(void)
{
    g_mdp.al_state = AL_STATE_OP;
}

void MDP_Init(void)
{
    memset(&g_mdp, 0, sizeof(g_mdp));
    g_mdp.al_state            = AL_STATE_INIT;
    g_mdp.detected_list.count = MAX_SLOTS;
    for (uint8_t i = 0; i < MAX_SLOTS; i++) {
        g_mdp.detected_list.ident[i] = MODULE_IDENT_EMPTY;
    }
}

/* =========================================================================
   CYCLIC PROCESS DATA
   ========================================================================= */
static void MDP_ProcessOutputs(void)
{
    app_ipc_sharemem_lock();

    EC_API_SLV_getOutputData(ptSubDevice, g_mdp.total_output_bytes,
                             (uint8_t *)gSharedMem.buff_ecat_out);

    uint16_t offset = 0;
    for (uint16_t slot = 0; slot < MAX_SLOTS; slot++) {
        uint16_t bytes = g_mdp.slot[slot].output_bytes;

        if (!g_mdp.slot[slot].configured) {
            offset += bytes;
            continue;
        }

        /* slot IS the slaveInfo index — same mapping as HAL_ReadSlotIdent() */
        IO_SlaveInfo *s    = (IO_SlaveInfo *)&gSharedMem.IOCoupler_Devices.slaveInfo[slot];
        uint16_t offset_io = (uint16_t)s->offset_index;
        uint32_t ident     = g_mdp.configured_list.ident[slot];

        if (bytes > 0u && (offset + bytes) <= g_mdp.total_output_bytes) {
            if (ident == MODULE_IDENT_DO16) {
                /* EtherCAT RxPDO: 2-byte packed bitmask → buff_out: same 2-byte packed format */
                memcpy(&gSharedMem.buff_out[offset_io],
                       &gSharedMem.buff_ecat_out[offset],
                       IO_DIGITAL_MODULE_BYTESIZE);          /* 2 bytes */
                // OSAL_printf("MDP_ProcessOutputs: slot %u DO16 output mask=0x%04X (data=0x%04X)\r\n",
                //             (unsigned)slot,
                //             (unsigned)(*(uint16_t *)&gSharedMem.buff_out[offset_io]),
                //             (unsigned)(*(uint16_t *)&gSharedMem.buff_ecat_out[offset]));
            }
            else if (ident == MODULE_IDENT_AOC8 || ident == MODULE_IDENT_AOV8) {
                /* EtherCAT RxPDO: 16-byte block (8×uint16) → buff_out: same layout */
                memcpy(&gSharedMem.buff_out[offset_io],
                       &gSharedMem.buff_ecat_out[offset],
                       IO_ANALOG_MODULE_BYTESIZE);           /* 16 bytes */
                // OSAL_printf("MDP_ProcessOutputs: slot %u AOC8/AOV8 output data=0x%04X\r\n",
                //             (unsigned)slot,
                //             (unsigned)(*(uint16_t *)&gSharedMem.buff_ecat_out[offset]));
            }
        }

        offset += bytes;
    }

    app_ipc_sharemem_unlock();
}

static void MDP_ProcessInputs(void)
{
    app_ipc_sharemem_lock();

    uint16_t offset = 0;

    for (uint16_t slot = 0; slot < MAX_SLOTS; slot++) {
        uint16_t bytes = g_mdp.slot[slot].input_bytes;

        if (!g_mdp.slot[slot].configured) {
            offset += bytes;
            continue;
        }

        /* slot IS the slaveInfo index — same mapping as HAL_ReadSlotIdent() */
        IO_SlaveInfo *s    = (IO_SlaveInfo *)&gSharedMem.IOCoupler_Devices.slaveInfo[slot];
        uint16_t offset_io = (uint16_t)s->offset_index;
        uint32_t ident     = g_mdp.configured_list.ident[slot];

        if (bytes > 0u && (offset + bytes) <= g_mdp.total_input_bytes) {
            if (ident == MODULE_IDENT_DI16) {
                /* buff_in: 2-byte packed bitmask → EtherCAT TxPDO: same format */
                memcpy(&gSharedMem.buff_ecat_in[offset],
                       &gSharedMem.buff_in[offset_io],
                       IO_DIGITAL_MODULE_BYTESIZE);          /* 2 bytes */
                // OSAL_printf("MDP_ProcessInputs: slot %u DI16 input mask=0x%04X data=0x%04X\r\n",
                //             (unsigned)slot,
                //             (unsigned)(*(uint16_t *)&gSharedMem.buff_in[offset_io]),
                //             (unsigned)(*(uint16_t *)&gSharedMem.buff_ecat_in[offset]));
            }
            else if (ident == MODULE_IDENT_AIC8 || ident == MODULE_IDENT_AIV8) {
                /* buff_in: 16-byte block (8×uint16) → EtherCAT TxPDO: same layout */
                memcpy(&gSharedMem.buff_ecat_in[offset],
                       &gSharedMem.buff_in[offset_io],
                       IO_ANALOG_MODULE_BYTESIZE);           /* 16 bytes */
                // OSAL_printf("MDP_ProcessInputs: slot %u AIC8/AIV8 input data=0x%04X\r\n",
                //             (unsigned)slot,
                //             (unsigned)(*(uint16_t *)&gSharedMem.buff_ecat_in[offset]));
            }
            else if (ident == MODULE_IDENT_RTDY || ident == MODULE_IDENT_RTDB) {
                /* buff_in: 24-byte block (12×uint16) → EtherCAT TxPDO: same layout */
                memcpy(&gSharedMem.buff_ecat_in[offset],
                       &gSharedMem.buff_in[offset_io],
                       IO_RTD_MODULE_BYTESIZE);
            }
        }

        offset += bytes;
    }

    /* Push completed input image to EtherCAT stack AFTER unlocking */
    EC_API_SLV_setInputData(ptSubDevice, g_mdp.total_input_bytes,
                            (uint8_t *)gSharedMem.buff_ecat_in);

    app_ipc_sharemem_unlock();
}
void MDP_Check_State(EC_API_SLV_EEsmState_t curState, EC_API_SLV_EEsmState_t lastState)
{
    if (curState == 0x12 && lastState == 0x2) {
        uint16_t rxBytes = 0u;
        uint16_t txBytes = 0u;
        for (uint8_t i = 0; i < MAX_SLOTS; i++) {
            uint32_t ident = g_mdp.configured_list.ident[i];
            if (ident == MODULE_IDENT_EMPTY) continue;
            uint8_t outSubs = MDP_GetOutputSubCount(ident);
            uint8_t outBits = MDP_GetEntryBits(ident, true);
            uint8_t inSubs  = MDP_GetInputSubCount(ident);
            uint8_t inBits  = MDP_GetEntryBits(ident, false);
            if (outSubs > 0u) rxBytes += (uint16_t)(((uint32_t)outSubs * outBits + 7u) / 8u);
            if (inSubs  > 0u) txBytes += (uint16_t)(((uint32_t)inSubs  * inBits  + 7u) / 8u);
        }
        OSAL_printf("SafeOP entry: expected SM2(out)=%u SM3(in)=%u bytes\r\n",
                    (unsigned)rxBytes, (unsigned)txBytes);
    }
}

void MDP_Cyclic(void)
{
    if (g_mdp.al_state != AL_STATE_OP) return;
    MDP_ProcessOutputs();
    MDP_ProcessInputs();
}