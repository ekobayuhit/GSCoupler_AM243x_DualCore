#ifndef MDP_H
#define MDP_H

#include <stdint.h>
#include <stdbool.h>
#include <ecSlvApi.h>

/* Ensure MBXMEM is defined before we use it in callback signatures */
#if !(defined MBXMEM)
#define MBXMEM
#endif

/*
 * Lifecycle
 */
void MDP_Init(void);       /* Call before EC_API_SLV_init */
void MDP_CreateObjects(void); /* Call during population phase, before stack start */
void MDP_Cyclic(void);     /* Call every cycle when in OP */

/*
 * State machine hooks – call from the application's AL state-change handler
 */
void MDP_OnEnterInit(void);
void MDP_OnEnterPreOP(void);
bool MDP_OnEnterSafeOP(void);   /* returns false if module ident mismatch */
void MDP_OnEnterOP(void);

/*
 * HAL
 */
void HAL_Set_ptSubDevice(EC_API_SLV_SHandle_t *handle);

/*
 * CoE SDO callbacks – registered with the EtherCAT stack
 */
uint8_t EC_SLV_APP_MDP_SDO_Read(
    void *pContext_p, uint16_t index_p, uint8_t subIndex_p,
    uint32_t size_p, uint16_t MBXMEM *pData_p, uint8_t completeAccess_p);

uint8_t EC_SLV_APP_MDP_SDO_Write(
    void *pContext_p, uint16_t index_p, uint8_t subIndex_p,
    uint32_t size_p, uint16_t MBXMEM *pData_p, uint8_t completeAccess_p);

/*
 * Called by the webserver state-machine to validate F030 vs F050 on
 * PreOP→SafeOP.  Now delegated to MDP_OnEnterSafeOP(); this prototype
 * is kept for any callers that reference it directly.
 */
bool MDP_ValidateAndConfigure(void);


/**
 * @brief Checks the current state of the EtherCAT stack and prints the expected SM2 and SM3 bytes if the state is SafeOP (0x12).
 * 
 * @param curState 
 * @param lastState 
 */
void MDP_Check_State(EC_API_SLV_EEsmState_t curState, EC_API_SLV_EEsmState_t lastState);

/*
 * Called from EC_SLAVE_APP_mappingChangedHandler() whenever the master
 * (re)maps a 0x1600+i RxPDO or 0x1A00+i TxPDO.  Validates that the
 * mapping the master sent actually matches the module ident that was
 * configured for that slot (via 0xF030) and that the entries point at
 * the correct application object (0x2200+i / 0x2300+i).
 *
 * Returns EC_API_eERR_NONE (0) if the mapping is acceptable, or
 * EC_API_eERR_ABORT if it does not match — causing the stack to reject
 * the mapping change and keep the SubDevice from transitioning to
 * SafeOP/OP with an inconsistent configuration.
 */
uint32_t MDP_OnMappingChanged(
    uint16_t pdoIndex_p, uint8_t count_p,
    EC_API_SLV_PDO_SEntryMap_t *pPdoMap_p);

#endif /* MDP_H */