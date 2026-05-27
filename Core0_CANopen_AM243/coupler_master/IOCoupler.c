#include <stdint.h>
#include <stdbool.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/ClockP.h>

#include "IOCoupler.h"
#include "ODMaster.h"

/* -------------------------------------------------- */
/* CONFIG                                             */
/* -------------------------------------------------- */
#define SCAN_IO_TIMEOUT_MS          500   /* precise timeout per IO-SDO */
#define SCAN_IO_TIMEOUT_LONG_MS     1000

/* -------------------------------------------------- */
/* GLOBAL DATA                                        */
/* -------------------------------------------------- */

extern volatile ipc_data_t gSharedMem;
IO_RuntimeInfo IO_slave_data[MAX_IO_DEVICES];

/* Scan context */
static ScanState_t scanState = SCAN_IDLE;
static UNS8  currentNode = 1;

static uint32_t vendorId        = 0;
static uint32_t productCode     = 0;
static uint32_t revisionNumber  = 0;
static uint32_t serialNumber    = 0;
static char hw_version[GESPANT_HW_VER_SIZE] = {0};
static char fw_version[GESPANT_FW_VER_SIZE] = {0};

static UNS32 sdoSize   = 4;
static UNS32 abortCode = 0;

/* precise timeout deadline (ms) */
static uint64_t timeoutDeadlineMs = 0;

/* -------------------------------------------------- */
/* TIME HELPER                                        */
/* -------------------------------------------------- */
static uint64_t get_time_ms(void)
{
    return ClockP_getTimeUsec() / 1000;
}

/* -------------------------------------------------- */
/* TIMEOUT MACROS                                     */
/* -------------------------------------------------- */

#define START_TIMEOUT() \
    do { timeoutDeadlineMs = get_time_ms() + SCAN_IO_TIMEOUT_MS; } while (0)

#define START_TIMEOUT_LONG() \
    do { timeoutDeadlineMs = get_time_ms() + SCAN_IO_TIMEOUT_LONG_MS; } while (0)

#define TIMEOUT_EXPIRED() \
    (get_time_ms() > timeoutDeadlineMs)

/* -------------------------------------------------- */
/* HELPERS                                            */
/* -------------------------------------------------- */

ScanState_t get_scan_state(void)
{
    return scanState;
}

static bool verify_product_code(uint32_t productCode)
{
    switch (productCode) {
        case IO_DEVICE_TYPE_DO16:
        case IO_DEVICE_TYPE_DI16:
        case IO_DEVICE_TYPE_AIC8:
        case IO_DEVICE_TYPE_AIV8:
        case IO_DEVICE_TYPE_AOC8:
        case IO_DEVICE_TYPE_AOV8:
        case IO_DEVICE_TYPE_RTDY:
        case IO_DEVICE_TYPE_RTDB:
            return true;
        default:
            return false;
    }
}

static const char *IO_typename(uint32_t productCode)
{
    switch (productCode) {
        case IO_DEVICE_TYPE_DO16: return "DO16";
        case IO_DEVICE_TYPE_DI16: return "DI16";
        case IO_DEVICE_TYPE_AIC8: return "AIC8";
        case IO_DEVICE_TYPE_AIV8: return "AIV8";
        case IO_DEVICE_TYPE_AOC8: return "AOC8";
        case IO_DEVICE_TYPE_AOV8: return "AOV8";
        case IO_DEVICE_TYPE_RTDY: return "RTDY";
        case IO_DEVICE_TYPE_RTDB: return "RTDB";
        default: return "UNKNOWN";
    }
}

static void IOCoupler_put_slave_info(IO_SlaveInfo *info)
{
    app_ipc_sharemem_lock();
    if (gSharedMem.IOCoupler_Devices.numberOfSlaves >= MAX_IO_DEVICES)
        return;

    switch(info->productCode) {
        // --- OUTPUT DEVICES ---
        case IO_DEVICE_TYPE_DO16:
            gSharedMem.IOCoupler_Devices.numberofDO++;
            gSharedMem.IOCoupler_Devices.numberOfOutputSlaves++;
            info->output_index = gSharedMem.IOCoupler_Devices.numberOfOutputSlaves;
            info->input_index = 0;
            break;

        case IO_DEVICE_TYPE_AOC8:
            gSharedMem.IOCoupler_Devices.numberofAOC++;        
            gSharedMem.IOCoupler_Devices.numberOfOutputSlaves++;
            info->output_index = gSharedMem.IOCoupler_Devices.numberOfOutputSlaves;
            info->input_index = 0;
            break;

        case IO_DEVICE_TYPE_AOV8:
            gSharedMem.IOCoupler_Devices.numberofAOV++;        
            gSharedMem.IOCoupler_Devices.numberOfOutputSlaves++;
            info->output_index = gSharedMem.IOCoupler_Devices.numberOfOutputSlaves;
            info->input_index = 0;
            break;

        // --- INPUT DEVICES ---
        case IO_DEVICE_TYPE_DI16:
            gSharedMem.IOCoupler_Devices.numberofDI++;
            gSharedMem.IOCoupler_Devices.numberOfInputSlaves++;
            info->input_index = gSharedMem.IOCoupler_Devices.numberOfInputSlaves;
            info->output_index = 0;
            break;

        case IO_DEVICE_TYPE_AIC8:
            gSharedMem.IOCoupler_Devices.numberofAIC++;
            gSharedMem.IOCoupler_Devices.numberOfInputSlaves++;
            info->input_index = gSharedMem.IOCoupler_Devices.numberOfInputSlaves;
            info->output_index = 0;
            break;

        case IO_DEVICE_TYPE_AIV8:
            gSharedMem.IOCoupler_Devices.numberofAIV++;
            gSharedMem.IOCoupler_Devices.numberOfInputSlaves++;
            info->input_index = gSharedMem.IOCoupler_Devices.numberOfInputSlaves;
            info->output_index = 0;
            break;

        case IO_DEVICE_TYPE_RTDY:
            gSharedMem.IOCoupler_Devices.numberofRTDY++;
            gSharedMem.IOCoupler_Devices.numberOfInputSlaves++;
            info->input_index = gSharedMem.IOCoupler_Devices.numberOfInputSlaves;
            info->output_index = 0;
            break;

        case IO_DEVICE_TYPE_RTDB:
            gSharedMem.IOCoupler_Devices.numberofRTDB++;
            gSharedMem.IOCoupler_Devices.numberOfInputSlaves++;
            info->input_index = gSharedMem.IOCoupler_Devices.numberOfInputSlaves;
            info->output_index = 0;
            break;

        default: 
            break;
    }

    gSharedMem.IOCoupler_Devices.slaveInfo[gSharedMem.IOCoupler_Devices.numberOfSlaves++] = *info;
    app_ipc_sharemem_unlock();
}

static void IOCoupler_update_slave_info_hw_fw(uint8_t nodeId, const char *hw_ver, const char *fw_ver)
{
    app_ipc_sharemem_lock();
    // DebugP_log("Updating HW/FW %u for node %u | hwver %s | fwver %s\r\n", IOCoupler_Devices.slaveInfo[nodeId-1].nodeId, nodeId, hw_ver, fw_ver);
    for (uint8_t i = 0; i < gSharedMem.IOCoupler_Devices.numberOfSlaves; i++) {
        if (gSharedMem.IOCoupler_Devices.slaveInfo[i].nodeId == nodeId) {
            // DebugP_log("Found slave info for node %u, updating HW/FW\r\n", nodeId);
            if(hw_ver){
                strncpy(
                    (char *)gSharedMem.IOCoupler_Devices.slaveInfo[i].hw_version,
                    hw_ver,
                    GESPANT_HW_VER_SIZE
                );
            }
            if(fw_ver){
                strncpy(
                    (char *)gSharedMem.IOCoupler_Devices.slaveInfo[i].fw_version,
                    fw_ver,
                    GESPANT_FW_VER_SIZE
                );
            }
        }
    }
    app_ipc_sharemem_unlock();
}

static void IOCoupler_print_result(void)
{
    DebugP_log("\n--- IO Coupler Scan Results ---\r\n");
    DebugP_log("Total Slaves Found: %u\r\n", gSharedMem.IOCoupler_Devices.numberOfSlaves);

    if (gSharedMem.IOCoupler_Devices.numberOfSlaves == 0) {
        DebugP_log("No IO slaves detected on the bus.\r\n");
    } else {
        for (uint8_t i = 0; i < gSharedMem.IOCoupler_Devices.numberOfSlaves; i++) {

            const IO_SlaveInfo *slave = (IO_SlaveInfo *)&gSharedMem.IOCoupler_Devices.slaveInfo[i];
            char sn_str[32];

            snprintf(sn_str, sizeof(sn_str), "%08X", (unsigned int)slave->serialNumber);

            DebugP_log("[Slot-%02u] ID: 0x%02X | Type: %-6s | Rev: %u | SN: %s | HW: %s | FW: %s\r\n",
                i+1,
                slave->nodeId,
                IO_typename(slave->productCode),
                slave->revisionNumber,
                sn_str, 
                slave->hw_version, 
                slave->fw_version);
        }
    }
    DebugP_log("-------------------------------\r\n");
}

/* -------------------------------------------------- */
/* SDO CALLBACKS                                      */
/* -------------------------------------------------- */

static void SDO_Vendor_Callback(CO_Data *d, UNS8 nodeId)
{
    if (getReadResultNetworkDict(d, nodeId,
        &vendorId, &sdoSize, &abortCode) == SDO_FINISHED)
    {
        scanState = SCAN_READ_PRODUCT;
    }

    closeSDOtransfer(d, nodeId, SDO_CLIENT);
}

static void SDO_Product_Callback(CO_Data *d, UNS8 nodeId)
{
    if (getReadResultNetworkDict(d, nodeId,
        &productCode, &sdoSize, &abortCode) == SDO_FINISHED)
    {
        scanState = SCAN_READ_REVISION;
    }

    closeSDOtransfer(d, nodeId, SDO_CLIENT);
}

static void SDO_Revision_Callback(CO_Data *d, UNS8 nodeId)
{
    if (getReadResultNetworkDict(d, nodeId,
        &revisionNumber, &sdoSize, &abortCode) == SDO_FINISHED)
    {
        scanState = SCAN_READ_SERIAL;
    }

    closeSDOtransfer(d, nodeId, SDO_CLIENT);
}

static void SDO_Serial_Callback(CO_Data *d, UNS8 nodeId)
{
    if (getReadResultNetworkDict(d, nodeId,
        &serialNumber, &sdoSize, &abortCode) == SDO_FINISHED)
    {
        if (vendorId == GESPANT_VENDOR_ID &&
            verify_product_code(productCode))
        {
            IO_SlaveInfo info = {0};
            info.nodeId        = nodeId;
            info.vendorId      = vendorId;
            info.productCode   = productCode;
            info.revisionNumber= revisionNumber;
            info.serialNumber  = serialNumber;
            IOCoupler_put_slave_info(&info);
        }
        scanState = SCAN_READ_HW_VER;
    }

    closeSDOtransfer(d, nodeId, SDO_CLIENT);
}

static void SDO_HwVer_Callback(CO_Data *d, UNS8 nodeId)
{
    if (getReadResultNetworkDict(d, nodeId,
        &hw_version, &sdoSize, &abortCode) == SDO_FINISHED)
    {
        // DebugP_log("Node %u: HW Ver: %s\r\n", nodeId, hw_version);
        IOCoupler_update_slave_info_hw_fw(nodeId, hw_version, NULL);
        scanState = SCAN_READ_FW_VER;
    }
    // DebugP_log("SDO HW Ver callback sdosize: %u | abortCode: %u\r\n", sdoSize, abortCode);
    closeSDOtransfer(d, nodeId, SDO_CLIENT);
}

static void SDO_FwVer_Callback(CO_Data *d, UNS8 nodeId)
{
    sdoSize = GESPANT_FW_VER_SIZE;
    if (getReadResultNetworkDict(d, nodeId,
        &fw_version, &sdoSize, &abortCode) == SDO_FINISHED)
    {
        // DebugP_log("Node %u: FW Ver: %s\r\n", nodeId, fw_version);
        IOCoupler_update_slave_info_hw_fw(nodeId, NULL, fw_version);      
    }
    // DebugP_log("SDO FW Ver callback sdosize: %u | abortCode: %u\r\n", sdoSize, abortCode);
    closeSDOtransfer(d, nodeId, SDO_CLIENT);
    scanState = SCAN_READ_VENDOR;
    currentNode++;
}
/* -------------------------------------------------- */
/* ASYNC SCAN TASK                                    */
/* Call periodically (e.g. every 1–10 ms)             */
/* -------------------------------------------------- */

void IOCoupler_scan_task(CO_Data *d)
{
    switch (scanState)
    {
        case SCAN_IDLE:
            DebugP_log("Scan started!\r\n");
            
            app_ipc_sharemem_lock();
            gSharedMem.IOCoupler_Devices.numberOfSlaves = 0;
            app_ipc_sharemem_unlock();

            currentNode = 1;
            scanState = SCAN_READ_VENDOR;
            break;

        case SCAN_READ_VENDOR:
            if (currentNode > MAX_IO_DEVICES) {
                scanState = SCAN_DONE;
                IOCoupler_print_result();
                break;
            }
            // DebugP_log("scan state: SCAN_READ_VENDOR, node %u\r\n", currentNode);

            vendorId = productCode = revisionNumber = serialNumber = 0;
            sdoSize = 4;
            abortCode = 0;

            if (readNetworkDictCallback(
                    d, currentNode,
                    0x1018, 1, uint32,
                    SDO_Vendor_Callback, 0) == 0)
            {
                START_TIMEOUT();
                scanState = SCAN_WAIT;
            } else {
                currentNode++;
            }
            break;

        case SCAN_READ_PRODUCT:
            // DebugP_log("scan state: SCAN_READ_PRODUCT, node %u\r\n", currentNode);
            if (readNetworkDictCallback(
                    d, currentNode,
                    0x1018, 2, uint32,
                    SDO_Product_Callback, 0) == 0)
            {
                START_TIMEOUT_LONG();
                scanState = SCAN_WAIT;
            } else {
                scanState = SCAN_READ_VENDOR;
                currentNode++;
            }
            break;

        case SCAN_READ_REVISION:
            // DebugP_log("scan state: SCAN_READ_REVISION, node %u\r\n", currentNode);
            if (readNetworkDictCallback(
                    d, currentNode,
                    0x1018, 3, uint32,
                    SDO_Revision_Callback, 0) == 0)
            {
                START_TIMEOUT_LONG();
                scanState = SCAN_WAIT;
            } else {
                scanState = SCAN_READ_VENDOR;
                currentNode++;
            }
            break;

        case SCAN_READ_SERIAL:
            // DebugP_log("scan state: SCAN_READ_SERIAL, node %u\r\n", currentNode);
            if (readNetworkDictCallback(
                    d, currentNode,
                    0x1018, 4, uint32,
                    SDO_Serial_Callback, 0) == 0)
            {
                START_TIMEOUT_LONG();
                scanState = SCAN_WAIT;
            } else {
                scanState = SCAN_READ_VENDOR;
                currentNode++;
            }
            break;
        
        case SCAN_READ_HW_VER:
            // DebugP_log("scan state: SCAN_READ_HW_VER, node %u\r\n", currentNode);
            if (readNetworkDictCallback(
                    d, currentNode,
                    0x1009, 0, visible_string,
                    SDO_HwVer_Callback, 0) == 0)
            {
                START_TIMEOUT_LONG();
                scanState = SCAN_WAIT;
            } else {
                scanState = SCAN_READ_VENDOR;
                currentNode++;
            }
            break;
        
        case SCAN_READ_FW_VER:
            // DebugP_log("scan state: SCAN_READ_FW_VER, node %u\r\n", currentNode);
            if (readNetworkDictCallback(
                    d, currentNode,
                    0x100A, 0, visible_string,
                    SDO_FwVer_Callback, 0) == 0)
            {
                START_TIMEOUT_LONG();
                scanState = SCAN_WAIT;
            } else {
                scanState = SCAN_READ_VENDOR;
                currentNode++;
            }
            break;

        case SCAN_WAIT:
            if (TIMEOUT_EXPIRED()) {
                // DebugP_log("SDO timeout node %u\r\n", currentNode);
                // DebugP_log("Check timer value %u\r\n", get_time_ms());
                closeSDOtransfer(d, currentNode, SDO_CLIENT);
                scanState = SCAN_READ_VENDOR;
                currentNode++;
            }
            break;

        case SCAN_DONE:
            /* idle */
            break;
    }
}

/*****************************************************************************/
void ODMaster_Init_PDO(CO_Data *d){
    UNS32 cobid = 0;
    UNS32 Transmission_type = 0;
    UNS32 size32 = sizeof(UNS32);
    UNS32 size8 = sizeof(UNS8);

    DebugP_log("[IO_COUPLER] ODMaster_Init_PDO\r\n");
    for(int i = 0; i < MAX_IO_DEVICES; i++){
        cobid = 0x180 + i;
        /* Disable Master_ RPDO */
        cobid = COBID_DISABLE(cobid);
        writeLocalDict(
            d, 
            GET_COBID_INPUT_PDO_COM(i),    //index
            1,                          //sub-index
            &cobid, 
            &size32, 
            RW);
        
        /* Conf transmission type */
        Transmission_type = INPUT_TRANSMISSION_TYPE;
        writeLocalDict(
            d, 
            GET_COBID_INPUT_PDO_COM(i),    //index
            2,                          //sub-index
            &Transmission_type, 
            &size8, 
            RW);
    }
    for(int i = 0; i < MAX_IO_DEVICES; i++){
        cobid = 0x200 + i;
        /* Disable Master_ TPDO */
        cobid = COBID_DISABLE(cobid);
        writeLocalDict(
            d, 
            GET_COBID_OUTPUT_PDO_COM(i),    //index
            1,                          //sub-index
            &cobid, 
            &size32, 
            RW);

        /* Conf transmission type */
        Transmission_type = OUTPUT_TRANSMISSION_TYPE;
        writeLocalDict(
            d, 
            GET_COBID_OUTPUT_PDO_COM(i),    //index
            2,                          //sub-index
            &Transmission_type, 
            &size8, 
            RW);
    }
}

void _ODMaster_configure_PDO(CO_Data *d, IO_SlaveInfo *io_info)
{
    UNS32 cobid = 0;   
    UNS32 size32 = sizeof(UNS32);
    UNS32 size8 = sizeof(UNS8);
    UNS8  data_index = 0;
    UNS8  mapCount;
    UNS32 mapEntry;
    UNS32 offset_index;
    UNS32 errorCode;
    const indextable *ptrTable;
    ODCallback_t *Callback;

    if(io_info->productCode == IO_DEVICE_TYPE_DI16){
        // Slave TPDOx → Master_ RPDOx
        offset_index = io_info->input_index - 1;
        cobid = 0x180 + io_info->nodeId;
        data_index = 1;

        /* Disable Master_ RPDO */
        cobid = COBID_DISABLE(cobid);
        errorCode = writeLocalDict(d, GET_COBID_INPUT_PDO_COM(offset_index), 1, &cobid, &size32, RW);

        /* Clear mapping */
        mapCount = 0;
        errorCode = writeLocalDict(d, GET_COBID_INPUT_PDO_MAP(offset_index), 0, &mapCount, &size8, RW);

        /* Map DI object */
        mapEntry = GET_OD_INPUT_MAP(offset_index, data_index);
        errorCode = writeLocalDict(d, GET_COBID_INPUT_PDO_MAP(offset_index), data_index, &mapEntry, &size32, RW);

        /* Set mapping count */
        mapCount = (IO_DIGITAL_MODULE_BYTESIZE/2);//Per 2 byte
        errorCode = writeLocalDict(d, GET_COBID_INPUT_PDO_MAP(offset_index), 0, &mapCount, &size8, RW);

        /* Enable Master_ RPDO */
        cobid = COBID_ENABLE(cobid);
        errorCode = writeLocalDict(d, GET_COBID_INPUT_PDO_COM(offset_index), 1, &cobid, &size32, RW);

        /* Link DI PDO data pointer */
        ptrTable = (*d->scanIndexOD)(
            GET_COBID_INPUT_PDO_DATA(offset_index),
            &errorCode,
            &Callback
        );

        if (ptrTable &&
            ptrTable->bSubCount > 1 &&
            ptrTable->pSubindex[1].pObject)
        {
            IO_slave_data[io_info->nodeId-1].d_ptr = ptrTable->pSubindex[1].pObject;
        }
        else
        {
            IO_slave_data[io_info->nodeId-1].d_ptr = NULL; /* fail-safe */
        }
    }else if(io_info->productCode == IO_DEVICE_TYPE_DO16){
        // Master_ TPDOx → Slave RPDOx
        offset_index = io_info->output_index - 1;
        cobid = 0x200 + io_info->nodeId;
        data_index = 1;
        
        /* Disable TPDO */
        cobid = COBID_DISABLE(cobid);
        errorCode = writeLocalDict(d, GET_COBID_OUTPUT_PDO_COM(offset_index), 1, &cobid, &size32, RW);

        /* Clear mapping */
        mapCount = 0;
        errorCode = writeLocalDict(d, GET_COBID_OUTPUT_PDO_MAP(offset_index), 0, &mapCount, &size8, RW);

        /* Map DO object */
        mapEntry = GET_OD_OUTPUT_MAP(offset_index, data_index);
        errorCode = writeLocalDict(d, GET_COBID_OUTPUT_PDO_MAP(offset_index), data_index, &mapEntry, &size32, RW);

        /* Set mapping count */
        mapCount = (IO_DIGITAL_MODULE_BYTESIZE/2);//Per 2 byte
        errorCode = writeLocalDict(d, GET_COBID_OUTPUT_PDO_MAP(offset_index), 0, &mapCount, &size8, RW);

        /* Enable TPDO */
        cobid = COBID_ENABLE(cobid);
        errorCode = writeLocalDict(d, GET_COBID_OUTPUT_PDO_COM(offset_index), 1, &cobid, &size32, RW);

        /* Link DO PDO data pointer */
        ptrTable = (*d->scanIndexOD)(
            GET_COBID_OUTPUT_PDO_DATA(offset_index),
            &errorCode,
            &Callback
        );

        if (ptrTable &&
            ptrTable->bSubCount > 1 &&
            ptrTable->pSubindex[1].pObject)
        {
            IO_slave_data[io_info->nodeId-1].d_ptr = ptrTable->pSubindex[1].pObject;
        }
        else
        {
            IO_slave_data[io_info->nodeId-1].d_ptr = NULL; /* fail-safe */
        }
    }
    else if(io_info->productCode == IO_DEVICE_TYPE_AIC8 || io_info->productCode == IO_DEVICE_TYPE_AIV8)
    {
        // Slave TPDOx → Master_ RPDOx
        offset_index = io_info->input_index - 1;
        cobid = 0x180 + io_info->nodeId;
        data_index = 1;

        /* Disable Master_ RPDO */
        cobid = COBID_DISABLE(cobid);
        errorCode = writeLocalDict(d, GET_COBID_INPUT_PDO_COM(offset_index), 1, &cobid, &size32, RW);

        /* Clear mapping */
        mapCount = 0;
        errorCode = writeLocalDict(d, GET_COBID_INPUT_PDO_MAP(offset_index), 0, &mapCount, &size8, RW);

        /* Link PDO data pointer */
        ptrTable = (*d->scanIndexOD)(
            GET_COBID_INPUT_PDO_DATA(offset_index),
            &errorCode,
            &Callback
        );

        /* Map Input object */
        for(int i = 0; i < IO_ANALOG_CHANNEL_NUM; i++){
            mapEntry = GET_OD_INPUT_MAP(offset_index, data_index);
            errorCode = writeLocalDict(d, GET_COBID_INPUT_PDO_MAP(offset_index), data_index, &mapEntry, &size32, RW);
            

            if (ptrTable &&
                ptrTable->bSubCount > 1 &&
                ptrTable->pSubindex[data_index].pObject)
            {
                IO_slave_data[io_info->nodeId-1].a_ptr[i] = ptrTable->pSubindex[data_index].pObject;
            }
            else
            {
                IO_slave_data[io_info->nodeId-1].a_ptr[i] = NULL; /* fail-safe */
            }

            data_index++;
        }

        /* Set mapping count */
        mapCount = (IO_ANALOG_MODULE_BYTESIZE/2);//Per 2 byte
        errorCode = writeLocalDict(d, GET_COBID_INPUT_PDO_MAP(offset_index), 0, &mapCount, &size8, RW);

        /* Enable Master_ RPDO */
        cobid = COBID_ENABLE(cobid);
        errorCode = writeLocalDict(d, GET_COBID_INPUT_PDO_COM(offset_index), 1, &cobid, &size32, RW);
    }
    else if(io_info->productCode == IO_DEVICE_TYPE_AOC8 || io_info->productCode == IO_DEVICE_TYPE_AOV8)
    {
        // Master_ TPDOx → Slave RPDOx
        offset_index = io_info->output_index - 1;
        cobid = 0x200 + io_info->nodeId;
        data_index = 1;
        
        /* Disable TPDO */
        cobid = COBID_DISABLE(cobid);
        errorCode = writeLocalDict(d, GET_COBID_OUTPUT_PDO_COM(offset_index), 1, &cobid, &size32, RW);

        /* Clear mapping */
        mapCount = 0;
        errorCode = writeLocalDict(d, GET_COBID_OUTPUT_PDO_MAP(offset_index), 0, &mapCount, &size8, RW);

        /* Link PDO data pointer */
        ptrTable = (*d->scanIndexOD)(
            GET_COBID_OUTPUT_PDO_DATA(offset_index),
            &errorCode,
            &Callback
        );
        
        /* Map Output object */
        for(int i = 0; i < IO_ANALOG_CHANNEL_NUM; i++){
            mapEntry = GET_OD_OUTPUT_MAP(offset_index, data_index);
            errorCode = writeLocalDict(d, GET_COBID_OUTPUT_PDO_MAP(offset_index), data_index, &mapEntry, &size32, RW);
            

            if (ptrTable &&
                ptrTable->bSubCount > 1 &&
                ptrTable->pSubindex[data_index].pObject)
            {
                IO_slave_data[io_info->nodeId-1].a_ptr[i] = ptrTable->pSubindex[data_index].pObject;
            }
            else
            {
                IO_slave_data[io_info->nodeId-1].a_ptr[i] = NULL; /* fail-safe */
            }

            data_index++;
        }

        /* Set mapping count */
        mapCount = (IO_ANALOG_MODULE_BYTESIZE/2);//Per 2 byte
        errorCode = writeLocalDict(d, GET_COBID_OUTPUT_PDO_MAP(offset_index), 0, &mapCount, &size8, RW);

        /* Enable TPDO */
        cobid = COBID_ENABLE(cobid);
        errorCode = writeLocalDict(d, GET_COBID_OUTPUT_PDO_COM(offset_index), 1, &cobid, &size32, RW);
    }
    else if(io_info->productCode == IO_DEVICE_TYPE_RTDY || io_info->productCode == IO_DEVICE_TYPE_RTDB)
    {
        // Slave TPDOx → Master_ RPDOx
        offset_index = io_info->input_index - 1;
        cobid = 0x180 + io_info->nodeId;
        data_index = 1;

        /* Disable Master_ RPDO */
        cobid = COBID_DISABLE(cobid);
        errorCode = writeLocalDict(d, GET_COBID_INPUT_PDO_COM(offset_index), 1, &cobid, &size32, RW);

        /* Clear mapping */
        mapCount = 0;
        errorCode = writeLocalDict(d, GET_COBID_INPUT_PDO_MAP(offset_index), 0, &mapCount, &size8, RW);

        /* Link PDO data pointer */
        ptrTable = (*d->scanIndexOD)(
            GET_COBID_INPUT_PDO_DATA(offset_index),
            &errorCode,
            &Callback
        );

        /* Map Input object */
        for(int i = 0; i < IO_ANALOG_CHANNEL_NUM; i++){
            mapEntry = GET_OD_INPUT_MAP(offset_index, data_index);
            errorCode = writeLocalDict(d, GET_COBID_INPUT_PDO_MAP(offset_index), data_index, &mapEntry, &size32, RW);
            
            if (ptrTable &&
                ptrTable->bSubCount > 1 &&
                ptrTable->pSubindex[data_index].pObject)
            {
                IO_slave_data[io_info->nodeId-1].a_ptr[i] = ptrTable->pSubindex[data_index].pObject;
            }
            else
            {
                IO_slave_data[io_info->nodeId-1].a_ptr[i] = NULL; /* fail-safe */
            }

            data_index++;
        }

        /* Set mapping count */
        mapCount = (IO_ANALOG_MODULE_BYTESIZE/2);//Per 2 byte
        errorCode = writeLocalDict(d, GET_COBID_INPUT_PDO_MAP(offset_index), 0, &mapCount, &size8, RW);

        /* Enable Master_ RPDO */
        cobid = COBID_ENABLE(cobid);
        errorCode = writeLocalDict(d, GET_COBID_INPUT_PDO_COM(offset_index), 1, &cobid, &size32, RW);
    }
}

void ODMaster_configure_PDO(CO_Data *d)
{
    UNS32 size32 = sizeof(UNS32);
    UNS32 size8  = sizeof(UNS8);
    UNS8  entry  = gSharedMem.IOCoupler_Devices.numberOfSlaves;
    UNS32 _Slave_Prod_Heartbeat_T=GET_SLAVE_PROD_HEARTBEAT_CONSUMER;
    UNS32 sync_value = 0;   /* 0 = SYNC disabled */

    DebugP_log("[IO_COUPLER] ODMaster_configure_PDO\r\n");
    if (INPUT_TRANSMISSION_TYPE == 0xFF &&
        OUTPUT_TRANSMISSION_TYPE == 0xFF)
    {
        writeLocalDict(
            d,
            0x1006,     /* Communication cycle period */
            0x00,
            &sync_value,  /* pointer to data */
            &size32,
            RW
        );
    }

    /* set number of consumer heartbeat entries */
    writeLocalDict(
        d,
        0x1016,
        0x00,
        &entry,
        &size8,
        RW
    );

    // DebugP_log(" ---------ODMaster_configure_PDO------------\r\n");
    for (int i = 0; i < entry; i++) {
        _ODMaster_configure_PDO(d, (IO_SlaveInfo *)&gSharedMem.IOCoupler_Devices.slaveInfo[i]);

        UNS8 nodeId = gSharedMem.IOCoupler_Devices.slaveInfo[i].nodeId;

        /* Heartbeat time (ms) | Node-ID */
        UNS32 Master__Cons_Heartbeat_T =
            ((UNS16)nodeId<< 16) | (UNS16)_Slave_Prod_Heartbeat_T ;

        /* subindex must be sequential: 1..N */
        UNS8 sub = i + 1;

        /* Configure Consumer Heartbeat Time */
        writeLocalDict(
            d,
            0x1016,
            sub,
            &Master__Cons_Heartbeat_T,
            &size32,
            RW
        );
    }
    // DebugP_log(" -------------------------------------------\r\n");

    if (gSharedMem.IOCoupler_Devices.numberOfInputSlaves > 0) {
        int offset = gSharedMem.IOCoupler_Devices.numberOfInputSlaves - 1;
        
        d->lastIndex->PDO_RCV     = d->firstIndex->PDO_RCV + offset;
        d->lastIndex->PDO_RCV_MAP = d->firstIndex->PDO_RCV_MAP + offset;
    } else {
        d->lastIndex->PDO_RCV     = d->firstIndex->PDO_RCV;
        d->lastIndex->PDO_RCV_MAP = d->firstIndex->PDO_RCV_MAP;
    }

    if (gSharedMem.IOCoupler_Devices.numberOfOutputSlaves > 0) {
        int offset = gSharedMem.IOCoupler_Devices.numberOfOutputSlaves - 1;
        
        d->lastIndex->PDO_TRS     = d->firstIndex->PDO_TRS + offset;
        d->lastIndex->PDO_TRS_MAP = d->firstIndex->PDO_TRS_MAP + offset;
    } else {
        d->lastIndex->PDO_TRS     = d->firstIndex->PDO_TRS;
        d->lastIndex->PDO_TRS_MAP = d->firstIndex->PDO_TRS_MAP;
    }
}

void IOCoupler_reset(CO_Data *d){
    app_ipc_sharemem_lock();
    memset((IOCoupler_Device *)&gSharedMem.IOCoupler_Devices, 0, sizeof(gSharedMem.IOCoupler_Devices));
    app_ipc_sharemem_unlock();

    ODMaster_Init_PDO(d);
    scanState = SCAN_IDLE;
}