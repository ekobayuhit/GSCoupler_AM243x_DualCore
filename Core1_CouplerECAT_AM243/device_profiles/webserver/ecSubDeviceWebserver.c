/*!
 *  \file ecSubDeviceWebserver.c
 *
 *  \brief
 *  EtherCAT<sup>&reg;</sup> SubDevice EoE Webserver Application
 *
 *  \author
 *  Texas Instruments Incorporated
 *
 *  \copyright
 *  Copyright (C) 2025 Texas Instruments Incorporated
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define SHOW_LOOPCOUNT 0
#define SHOW_ESCSTATUS 0

#define ENABLE_I2CLEDS 1

#define BIT2BYTE(x) (((x) + 7) >> 3)

/*-----------------------------------------------------------------------------------------
------
------     Includes
------
-----------------------------------------------------------------------------------------*/

#include "ecSubDeviceWebserver.h"
#include <defines/ecSlvApiDef.h>
#include "project.h"

#include <ESL_os.h>
#include <ESL_BOARD_config.h>
#include <ESL_vendor.h>

#include <ESL_gpioHelper.h>
#include <ESL_foeDemo.h>
#include <ESL_eoeDemo.h>
#include <ESL_soeDemo.h>
#include <ESL_eeprom.h>
#include <ESL_version.h>

#if !(defined FBTL_REMOTE) && !(defined DPRAM_REMOTE)
#include <CUST_PHY_base.h>
#endif

#include <ecSlvApi.h>

#include "ipc_shareMem.h"
#include "mdp.h"

#if !(defined MBXMEM)
#define MBXMEM
#endif

/*-----------------------------------------------------------------------------------------
------
------     local variables and constants
------
-----------------------------------------------------------------------------------------*/
#define I2C_IOEXP_ADDR                0x60 // The I2C address for GPIO expander

// object indices
#define SLS_OBJIDX_DIAGNOSIS          0x2018
#define SLS_OBJIDX_FSOE_DIG_IO_MODULE 0xA000
#define SLS_OBJIDX_FSOE_CONN_STATE    0x0800

#define FSOE_CONN_ST_RESET            100
#define FSOE_CONN_ST_SESSION          101
#define FSOE_CONN_ST_CONNECTION       102
#define FSOE_CONN_ST_PARAMETER        103
#define FSOE_CONN_ST_DATA             104
#define FSOE_CONN_ST_FAILSAFE         105

extern volatile ipc_data_t gSharedMem;

static void EC_SLV_APP_SS_applicationRun(void *pAppCtxt_p);

/*-----------------------------------------------------------------------------------------
------
------     application specific functions
------
-----------------------------------------------------------------------------------------*/

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Callback for diagnosis example. It Sends a new Diagnosis message with the received data.
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pApplicationCtxt_p  application instance
 *  \param[in]  index_p                 object index
 *  \param[in]  subindex_p             object subIndex
 *  \param[in]  size_p                  size of data buffer
 *  \param[in]  pData_p                 buffer to be read from
 *  \param[in]  completeAccess_p     using complete access
 *  \return      CoE ErrorCode
 *
 *  \remarks
 *
 *  <!-- Group: -->
 *
 *  \ingroup EC_SLV_APP
 *
 * */
static uint8_t EC_SLV_APP_writeDiagnosis(
    void            *pApplicationCtxt_p,
    uint16_t         index_p,
    uint8_t          subindex_p,
    uint32_t         size_p,
    uint16_t MBXMEM *pData_p,
    uint8_t          completeAccess_p)
{
    EC_API_EError_t              error        = (EC_API_EError_t)ABORT_NOERROR;
    EC_SLV_APP_SS_Application_t *pAppInstance = NULL;
    EC_API_SLV_SDIAG_parameter_t param
        = { DEFTYPE_UNSIGNED8, EC_API_SLV_DIAG_MSG_PARAM_TYPE_DATA, sizeof(uint8_t), (uint8_t *)pData_p };

    OSALUNREF_PARM(completeAccess_p);

    if ((pApplicationCtxt_p != NULL) && (pData_p != NULL))
    {
        OSAL_printf(
            "%s ==> Idx: 0x%04x:%d | Size: %d | Value: %d\r\n",
            __func__,
            index_p,
            subindex_p,
            size_p,
            pData_p[0]);

        /* @cppcheck_justify{misra-c2012-11.5} generic API requires cast */
        /* cppcheck-suppress misra-c2012-11.5 */
        pAppInstance = (EC_SLV_APP_SS_Application_t *)pApplicationCtxt_p;
        EC_API_SLV_DIAG_newMessage(
            pAppInstance->ptEcSlvApi,
            EC_API_SLV_DIAG_CODE_EMCY(pData_p[0]),
            EC_API_SLV_DIAG_MSG_TYPE_ERROR,
            pData_p[0],
            1,
            &param);
    }
    return (uint8_t)error;
}

static EC_API_EError_t EC_SLV_APP_SS_populateSlaveInfo(
    EC_SLV_APP_SS_Application_t *pApplicationInstance_p)
{
    EC_API_EError_t       error = EC_API_eERR_INVALID;
    EC_API_SLV_SHandle_t *ptSubDevice;

    if (!pApplicationInstance_p)
    {
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }
    ptSubDevice = pApplicationInstance_p->ptEcSlvApi;

    error = (EC_API_EError_t)EC_API_SLV_setDeviceType(ptSubDevice, EC_API_SLV_eDT_MODULAR_DEVICE);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    error = (EC_API_EError_t)EC_API_SLV_setVendorId(ptSubDevice, ECAT_VENDORID);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    error = (EC_API_EError_t)EC_API_SLV_setRevisionNumber(ptSubDevice, EC_REVISION);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    error = (EC_API_EError_t)EC_API_SLV_setSerialNumber(ptSubDevice, 0x00000000);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    error = (EC_API_EError_t)EC_API_SLV_setProductCode(ptSubDevice, ECAT_PRODUCTCODE_WEBSERVER);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    error = (EC_API_EError_t)EC_API_SLV_setProductName(ptSubDevice, ECAT_PRODUCTNAME_WEBSERVER);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    error = (EC_API_EError_t)EC_API_SLV_setGroupType(ptSubDevice, "EtherCAT Toolkit");
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    error = (EC_API_EError_t)EC_API_SLV_setHwVersion(ptSubDevice, "R01");
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    error = ESL_setSWVersion(ptSubDevice);
    if (error != EC_API_eERR_NONE)
    {
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    /* Former Project.h */
    
    error = (EC_API_EError_t)EC_API_SLV_setPDOSize(ptSubDevice, EC_MAX_PD_LEN, EC_MAX_PD_LEN);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    error = (EC_API_EError_t)EC_API_SLV_setPDICfg(ptSubDevice, ESC_EE_PDI_CONTROL, ESC_EE_PDI_CONFIG);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    error = (EC_API_EError_t)EC_API_SLV_setBootStrapMailbox(
        ptSubDevice,
        EC_BOOTSTRAP_MBXOUT_START,
        EC_BOOTSTRAP_MBXOUT_DEF_LENGTH,
        EC_BOOTSTRAP_MBXIN_START,
        EC_BOOTSTRAP_MBXIN_DEF_LENGTH);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    error = (EC_API_EError_t)EC_API_SLV_setStandardMailbox(
        ptSubDevice,
        EC_MBXOUT_START,
        EC_MBXOUT_DEF_LENGTH,
        EC_MBXIN_START,
        EC_MBXIN_DEF_LENGTH);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    // error = (EC_API_EError_t)EC_API_SLV_setSyncManConfig(
    //     ptSubDevice,
    //     0,
    //     EC_MBXOUT_START,
    //     EC_MBXOUT_DEF_LENGTH,
    //     EC_MBXOUT_CONTROLREG,
    //     EC_MBXOUT_ENABLE);
    // if (error != EC_API_eERR_NONE)
    // {
    //     OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
    //     /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
    //     /* cppcheck-suppress misra-c2012-15.1 */
    //     goto Exit;
    // }

    // error = (EC_API_EError_t)EC_API_SLV_setSyncManConfig(
    //     ptSubDevice,
    //     1,
    //     EC_MBXIN_START,
    //     EC_MBXIN_DEF_LENGTH,
    //     EC_MBXIN_CONTROLREG,
    //     EC_MBXIN_ENABLE);
    // if (error != EC_API_eERR_NONE)
    // {
    //     OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
    //     /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
    //     /* cppcheck-suppress misra-c2012-15.1 */
    //     goto Exit;
    // }

    // error = (EC_API_EError_t)EC_API_SLV_setSyncManConfig(
    //     ptSubDevice,
    //     2,
    //     EC_OUTPUT_START,
    //     EC_OUTPUT_DEF_LENGTH,
    //     EC_OUTPUT_CONTROLREG,
    //     EC_OUTPUT_ENABLE);
    // if (error != EC_API_eERR_NONE)
    // {
    //     OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
    //     /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
    //     /* cppcheck-suppress misra-c2012-15.1 */
    //     goto Exit;
    // }

    // error = (EC_API_EError_t)EC_API_SLV_setSyncManConfig(
    //     ptSubDevice,
    //     3,
    //     EC_INPUT_START,
    //     EC_INPUT_DEF_LENGTH,
    //     EC_INPUT_CONTROLREG,
    //     EC_INPUT_ENABLE);
    // if (error != EC_API_eERR_NONE)
    // {
    //     OSAL_printf("%s:%d Error code: 0x%08x\r\n", __func__, __LINE__, error);
    //     /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
    //     /* cppcheck-suppress misra-c2012-15.1 */
    //     goto Exit;
    // }

    /* /Former Project.h */

    error = EC_API_eERR_NONE;
Exit:
    return error;
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Create FSoE connection diagnosis object.
 *
 *  \details
 *  This object shows the usage of ENUM objects.
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pApplicationInstance_p  Application instance
 *  \return      ErrorCode
 *
 *  <!-- Example: -->
 *
 *  \par Example
 *  \code{.c}
 *  // required variables
 *  EC_API_EError_t  retVal = 0;
 *  EC_SLV_APP_Sapplication_t* pApplicationInstance_p;
 *
 *  // the Call
 *  retVal = EC_SLV_APP_populateFSoEObject(pApplicationInstance_p);
 *  \endcode
 *
 *  <!-- Group: -->
 *
 *  \ingroup EC_SLV_APP
 *
 * */
static EC_API_EError_t EC_SLV_APP_populateFSoEObject(EC_SLV_APP_SS_Application_t *pApplicationInstance)
{
    EC_API_EError_t       error   = EC_API_eERR_INVALID;
    EC_API_SLV_SHandle_t *ptSubDevice = NULL;

    if (!pApplicationInstance)
    {
        goto Exit;
    }

    ptSubDevice = pApplicationInstance->ptEcSlvApi;

    // Create ENUM object
    error = (EC_API_EError_t)EC_API_SLV_CoE_odAddEnum(
        ptSubDevice,
        SLS_OBJIDX_FSOE_CONN_STATE,
        &pApplicationInstance->pt0800EnumObj);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("Object SLS_OBJIDX_FSOE_CONN_STATE Error code: 0x%08x\r\n", error);
        goto Exit;
    }

    error = (EC_API_EError_t)EC_API_SLV_CoE_configEnum(
        ptSubDevice,
        pApplicationInstance->pt0800EnumObj,
        FSOE_CONN_ST_RESET,
        "Reset");
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("Object 0x0800 SubIndex 1 Error code: 0x%08x\r\n", error);
        goto Exit;
    }
    error = (EC_API_EError_t)EC_API_SLV_CoE_configEnum(
        ptSubDevice,
        pApplicationInstance->pt0800EnumObj,
        FSOE_CONN_ST_SESSION,
        "Session");
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("Object 0x0800 SubIndex 2 Error code: 0x%08x\r\n", error);
        goto Exit;
    }
    error = (EC_API_EError_t)EC_API_SLV_CoE_configEnum(
        ptSubDevice,
        pApplicationInstance->pt0800EnumObj,
        FSOE_CONN_ST_CONNECTION,
        "Connection");
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("Object 0x0800 SubIndex 3 Error code: 0x%08x\r\n", error);
        goto Exit;
    }
    error = (EC_API_EError_t)EC_API_SLV_CoE_configEnum(
        ptSubDevice,
        pApplicationInstance->pt0800EnumObj,
        FSOE_CONN_ST_PARAMETER,
        "Parameter");
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("Object 0x0800 SubIndex 4 Error code: 0x%08x\r\n", error);
        goto Exit;
    }
    error = (EC_API_EError_t)EC_API_SLV_CoE_configEnum(
        ptSubDevice,
        pApplicationInstance->pt0800EnumObj,
        FSOE_CONN_ST_DATA,
        "Data");
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("Object 0x0800 SubIndex 5 Error code: 0x%08x\r\n", error);
        goto Exit;
    }
    error = (EC_API_EError_t)EC_API_SLV_CoE_configEnum(
        ptSubDevice,
        pApplicationInstance->pt0800EnumObj,
        FSOE_CONN_ST_FAILSAFE,
        "FailSafe");
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("Object 0x0800 SubIndex 6 Error code: 0x%08x\r\n", error);
        goto Exit;
    }

    // Create FSoE Diagnosis object
    error = (EC_API_EError_t)EC_API_SLV_CoE_odAddRecord(
        ptSubDevice,
        SLS_OBJIDX_FSOE_DIG_IO_MODULE,
        "FSoE Digital IO Module",
        NULL,
        NULL,
        NULL,
        NULL,
        &pApplicationInstance->ptA000RecObj);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("Object 0xA000 Error code: 0x%08x\r\n", error);
        goto Exit;
    }

    error = (EC_API_EError_t)EC_API_SLV_CoE_configRecordSubIndex(
        ptSubDevice,
        pApplicationInstance->ptA000RecObj,
        1,
        "Connection State",
        SLS_OBJIDX_FSOE_CONN_STATE,
        16,
        ACCESS_READ);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("Object 0xA000 SubIndex 1 Error code: 0x%08x\r\n", error);
        goto Exit;
    }
    error = (EC_API_EError_t)EC_API_SLV_CoE_configRecordSubIndex(
        ptSubDevice,
        pApplicationInstance->ptA000RecObj,
        2,
        "Connection Diagnosis",
        DEFTYPE_UNSIGNED16,
        16,
        ACCESS_READ);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("Object 0xA000 SubIndex 1 Error code: 0x%08x\r\n", error);
        goto Exit;
    }

    error = EC_API_eERR_NONE;
Exit:
    return error;
}

#if !(defined DPRAM_REMOTE) && !(defined FBTL_REMOTE)
/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Reset / UnReset Ethernet PHY
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pCtxt_p      application instance
 *  \param[in]  phyIdx_p     PHY index (0/1)
 *  \param[in]  reset_p      true: reset, false: run
 *
 *  <!-- Example: -->
 *
 *  \par Example
 *  \code{.c}
 *  // required variables
 *  void* pApp;
 *
 *  // the Call
 *  \endcode
 *  void EC_SLV_APP_SS_boardPhyReset(pApp, 0, true);
 *
 *  <!-- References: -->
 *
 *  <!-- Group: -->
 *
 *  \ingroup EC_SLV_APP
 *
 * */
static OSAL_FUNC_UNUSED void EC_SLV_APP_SS_boardPhyReset(void *pCtxt_p, uint8_t phyIdx_p, bool reset_p)
{
    /* @cppcheck_justify{misra-c2012-11.5} generic API requires cast */
    /* cppcheck-suppress misra-c2012-11.5 */
    EC_SLV_APP_SS_Application_t *pApplicationInstance = (EC_SLV_APP_SS_Application_t *)pCtxt_p;

    if (NULL == pApplicationInstance)
    {
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    ESL_BOARD_OS_phyReset(
        pApplicationInstance->gpioHandle,
        pApplicationInstance->selectedPruInstance,
        phyIdx_p,
        reset_p);

Exit:
    return;
}
#endif

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Stimulate board status LED callback
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pCallContext_p        application instance handle.
 *  \param[in]  runLed_p                true: run LED on, false: off
 *  \param[in]  errLed_p                true: error LED on, false: off
 *
 *  <!-- Example: -->
 *
 *  \par Example
 *  \code{.c}
 *  // required variables
 *  void* pAppL;
 *
 *  // the Call
 *  EC_SLV_APP_SS_appBoardStatusLed(pAppL, NULL, true, false);
 *  \endcode
 *
 *  <!-- Group: -->
 *
 *  \ingroup EC_SLV_APP
 *
 * */
static void EC_SLV_APP_SS_appBoardStatusLed(void *pCallContext_p, bool runLed_p, bool errLed_p)
{
    /* @cppcheck_justify{misra-c2012-11.5} generic API requires cast */
    /* cppcheck-suppress misra-c2012-11.5 */
    EC_SLV_APP_SS_Application_t *pApplicationInstance = (EC_SLV_APP_SS_Application_t *)pCallContext_p;

    if (NULL == pApplicationInstance)
    {
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    ESL_BOARD_OS_statusLED(
        pApplicationInstance->gpioHandle,
        pApplicationInstance->selectedPruInstance,
        runLed_p,
        errLed_p);

Exit:
    return;
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Register board related functions
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pAppInstance_p        Application instance handle
 *
 *  <!-- Example: -->
 *
 *  \par Example
 *  \code{.c}
 *  // required variables
 *  EC_API_EError_t retVal;
 *
 *  retVal = EC_SLV_APP_SS_populateBoardFunctions *pAppInstance;
 *
 *  // the Call
 *  EC_SLV_APP_SS_initBoardFunctions(pAppInstance);
 *  \endcode
 *
 *  <!-- Group: -->
 *
 *  \ingroup EC_SLV_APP_WEBSERVER
 *
 * */
static EC_API_EError_t EC_SLV_APP_SS_populateBoardFunctions(
    EC_SLV_APP_SS_Application_t *pApplicationInstance)
{
    EC_API_EError_t error = EC_API_eERR_INVALID;

    if (!pApplicationInstance)
    {
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    EC_API_SLV_cbRegisterBoardStatusLed(
        pApplicationInstance->ptEcSlvApi,
        EC_SLV_APP_SS_appBoardStatusLed,
        pApplicationInstance);

    error = EC_API_eERR_NONE;
Exit:
    return error;
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Initialize board related functions
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pAppInstance_p        Application instance handle
 *
 *  <!-- Example: -->
 *
 *  \par Example
 *  \code{.c}
 *  // required variables
 *  EC_SLV_APP_Application_t *pAppInstance;
 *
 *  // the Call
 *  EC_SLV_APP_SS_initBoardFunctions(pAppInstance);
 *  \endcode
 *
 *  <!-- Group: -->
 *
 *  \ingroup EC_SLV_APP_WEBSERVER
 *
 * */
void EC_SLV_APP_SS_initBoardFunctions(EC_SLV_APP_SS_Application_t *pAppInstance_p)
{
    if (!pAppInstance_p)
    {
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    /* open gpio instance */
    pAppInstance_p->gpioHandle = ESL_GPIO_init();

#if !(defined DPRAM_REMOTE) && !(defined FBTL_REMOTE)
    /* Configure manual MDIO mode workaround if configured in SysConfig or similar
     * This is to enable the TI workaround for errata i2329. The activation is
     * detected and provisioned in this call.
     */
    ESL_OS_manualMdioConfig(pAppInstance_p->ptEcSlvApi);

#if (!(defined SOC_AM263PX) && !(defined SOC_AM261X))
    /* configure Phy Reset Pin */
    ESL_BOARD_OS_configureResets(pAppInstance_p->gpioHandle, pAppInstance_p->selectedPruInstance);
#endif // !SOC_AM263PX && !SOC_AM261X
#else
    OSALUNREF_PARM(pAppInstance_p);
#endif
#if !(defined SOC_AM263PX)
    /* configure LED Pin */
    ESL_BOARD_OS_initStatusLED(pAppInstance_p->gpioHandle, pAppInstance_p->selectedPruInstance);
#endif //SOC_AM263PX

    ESL_GPIO_apply(pAppInstance_p->gpioHandle);

Exit:
    return;
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Register board related functions, which do not use stack handle
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pAppInstance_p        Application instance handle
 *
 *  <!-- Example: -->
 *
 *  \par Example
 *  \code{.c}
 *  // required variables
 *  EC_SLV_APP_Application_t *pAppInstance;
 *
 *  // the Call
 *  EC_SLV_APP_registerStacklessBoardFunctions(pAppInstance);
 *  \endcode
 *
 *  <!-- Group: -->
 *
 *  \ingroup EC_SLV_APP_WEBSERVER
 *
 * */
void EC_SLV_APP_SS_registerStacklessBoardFunctions(EC_SLV_APP_SS_Application_t *pAppInstance_p)
{
    if (!pAppInstance_p)
    {
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

#if !(defined DPRAM_REMOTE) && !(defined FBTL_REMOTE)
    ESL_BOARD_OS_registerPhys(pAppInstance_p->ptEcSlvApi, pAppInstance_p->selectedPruInstance);

    CUST_PHY_CBregisterLibDetect(CUST_PHY_detect, pAppInstance_p);
#if (!(defined SOC_AM263PX) && !(defined SOC_AM261X))
    CUST_PHY_CBregisterReset(EC_SLV_APP_SS_boardPhyReset, pAppInstance_p);
#endif // !SOC_AM263PX && !SOC_AM261X
#endif
Exit:
    return;
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Change on PDO assignment configuration
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]  pContext_p          The pointer to the EtherCAT API instance.
 *  \param[in]  pRxPdoAssignMap_p   pointer to SM2 PDO reconfigure assignments.
 *  \param[in]  pTxPdoAssignMap_p   pointer to SM3 PDO reconfigure assignments.
 *  \return     DTK error code
 *
 *  <!-- Group: -->
 *
 *  \ingroup EC_SLV_APP
 *
 * */
static uint32_t EC_SLAVE_APP_assignmentChangedHandler(
    void                                *pContext_p,
    EC_API_SLV_PDO_SReconfigAssignMap_t *pRxPdoAssignMap_p,
    EC_API_SLV_PDO_SReconfigAssignMap_t *pTxPdoAssignMap_p)
{
    uint32_t error = EC_API_eERR_NONE;
    OSALUNREF_PARM(pContext_p);

    OSAL_printf("**************************************\r\n");
    if (pRxPdoAssignMap_p->pdoAssignmentChanged)
    {
        if (pRxPdoAssignMap_p->pPdoIndexArray != NULL)
        {
            uint8_t idx;
            OSAL_printf("New assignments for SyncManager 2:\r\n");
            for (idx = 0; idx <pRxPdoAssignMap_p->pdoCount; idx++)
            {
                OSAL_printf("PDO: 0x%04x\r\n", pRxPdoAssignMap_p->pPdoIndexArray[idx]);
            }
        }
    }
    if (pTxPdoAssignMap_p->pdoAssignmentChanged)
    {
        if (pTxPdoAssignMap_p->pPdoIndexArray != NULL)
        {
            uint8_t idx;
            OSAL_printf("New assignments for SyncManager 3:\r\n");
            for (idx = 0; idx <pTxPdoAssignMap_p->pdoCount; idx++)
            {
                OSAL_printf("PDO: 0x%04x\r\n", pTxPdoAssignMap_p->pPdoIndexArray[idx]);
            }
        }
    }
    OSAL_printf("**************************************\r\n");

    if (pRxPdoAssignMap_p->pdoCount == 0u && pTxPdoAssignMap_p->pdoCount == 0u)
    {
        error = EC_API_eERR_ABORT;
    }
    return error;
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  Change on PDO mapping configuration
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]        pContext_p             The pointer to the EtherCAT API instance.
 *  \param[in]        pdoIndex_p             PDO index number.
 *  \param[in]        count_p                 Number of objects mapped as PDO.
 *  \param[in]        pPdoMap_p              PDO mapping entries.
 *  \return      DTK error code
 *
 *  <!-- Group: -->
 *
 *  \ingroup EC_SLV_APP
 *
 * */
static uint32_t EC_SLAVE_APP_mappingChangedHandler(
    void                       *pContext_p,
    uint16_t                    pdoIndex_p,
    uint8_t                     count_p,
    EC_API_SLV_PDO_SEntryMap_t *pPdoMap_p)
{
    uint32_t error = EC_API_eERR_NONE;
    OSALUNREF_PARM(pContext_p);
    if (pPdoMap_p != NULL)
    {
        uint8_t idx;
        // OSAL_printf("**************************************\r\n");
        // OSAL_printf("New mapping for PDO 0x%04x: \r\n", pdoIndex_p);
        // for (idx = 0; idx < count_p; idx++)
        // {
        //     OSAL_printf("0x%04x:%d:0x%02x\r\n", pPdoMap_p[idx].index, pPdoMap_p[idx].subIndex,pPdoMap_p[idx].size );
        // }
        // OSAL_printf("**************************************\r\n");
    }
    if (count_p == 0u)
    {
        error = EC_API_eERR_ABORT;
    }

    /* MDP: validate that this PDO's new mapping matches the module ident
       configured for the corresponding slot via 0xF030, and mark the
       slot configured if so. Only takes effect for slots/PDOs we don't
       already know are bad. */
    if (error == EC_API_eERR_NONE)
    {
        error = MDP_OnMappingChanged(pdoIndex_p, count_p, pPdoMap_p);
    }

    return error;
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  AoE read request handler
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]        pContext_p              The pointer to the EtherCAT API instance.
 *  \param[in]        port_p                  AMS port.
 *  \param[in]        index_p                 16 bit index value from IndexOffset.
 *  \param[in]        subIndex_p              8 bit subIndex value from IndexOffset.
 *  \param[in]        completeAccess_p        CoE Complete Access flag.
 *  \param[in,out]    pLength_p               Request data length.
 *  \param[in]        pData_p                 Pointer to data.
 *  \return           ADS error code          32-bit error code
 *
 *  <!-- Group: -->
 *
 *  \ingroup EC_SLV_APP
 *
 * */
static uint32_t EC_SLV_APP_AoE_readRequest(
    void     *pContext_p,
    uint16_t  port_p,
    uint16_t  index_p,
    uint8_t   subIndex_p,
    bool      completeAccess_p,
    uint32_t *pLength_p,
    uint16_t *pData_p)
{
    uint32_t adsError = ERR_NOERROR;

    EC_API_SLV_SHandle_t       *pEcSlvApi    = (EC_API_SLV_SHandle_t *)pContext_p;
    EC_API_SLV_SCoE_ObjEntry_t *pObjectEntry = NULL;
    EC_API_SLV_SCoE_Object_t   *pObject      = NULL;
    uint32_t                    length       = 0;

    OSALUNREF_PARM(port_p);

    if ((NULL != pLength_p) && (NULL != pData_p))
    {
        uint32_t dtkError;
        if (completeAccess_p == false)
        {
            dtkError = EC_API_SLV_CoE_getObjectEntry(pEcSlvApi, index_p, subIndex_p, &pObjectEntry);
            if (dtkError == EC_API_eERR_NONE)
            {
                EC_API_SLV_CoE_getObjectEntryLength(pEcSlvApi, pObjectEntry, &length);
                if (length <= *pLength_p)
                {
                    *pLength_p = length;
                    dtkError
                        = EC_API_SLV_CoE_getObjectEntryData(pEcSlvApi, pObjectEntry, length, pData_p);
                    if (dtkError != EC_API_eERR_NONE)
                    {
                        adsError = ADSERR_DEVICE_ERROR;
                    }
                }
                else
                {
                    adsError = ADSERR_DEVICE_INVALIDSIZE;
                }
            }
            else
            {
                adsError = ADSERR_DEVICE_NOTFOUND;
            }
        }
        else
        {
            dtkError = EC_API_SLV_CoE_getObject(pEcSlvApi, index_p, &pObject);
            if (dtkError == EC_API_eERR_NONE)
            {
                if (subIndex_p == 0u)
                {
                    EC_API_SLV_CoE_getObjectLength(pEcSlvApi, pObject, &length);
                    if (length <= *pLength_p)
                    {
                        *pLength_p = length;
                        EC_API_SLV_CoE_getObjectEntryCount(pEcSlvApi, pObject, (uint8_t *)&pData_p[0]);
                        dtkError
                            = EC_API_SLV_CoE_getObjectData(pEcSlvApi, pObject, length, &pData_p[1]);
                        if (dtkError != EC_API_eERR_NONE)
                        {
                            adsError = ADSERR_DEVICE_ERROR;
                        }
                    }
                    else
                    {
                        adsError = ADSERR_DEVICE_INVALIDSIZE;
                    }
                }
                else if (subIndex_p == 1u)
                {
                    EC_API_SLV_CoE_getObjectLength(pEcSlvApi, pObject, &length);
                    if (length <= *pLength_p)
                    {
                        *pLength_p = length;
                        dtkError = EC_API_SLV_CoE_getObjectData(pEcSlvApi, pObject, length, pData_p);
                        if (dtkError != EC_API_eERR_NONE)
                        {
                            adsError = ADSERR_DEVICE_ERROR;
                        }
                    }
                    else
                    {
                        adsError = ADSERR_DEVICE_INVALIDSIZE;
                    }
                }
                else
                {
                    adsError = ADSERR_DEVICE_INVALIDPARM;
                }
            }
            else
            {
                adsError = ADSERR_DEVICE_NOTFOUND;
            }
        }
    }
    else
    {
        adsError = ADSERR_DEVICE_INVALIDPARM;
    }
    return adsError;
}

/*!
 *  <!-- Description: -->
 *
 *  \brief
 *  AoE write request handler
 *
 *  <!-- Parameters and return values: -->
 *
 *  \param[in]        pContext_p             The pointer to the EtherCAT API instance.
 *  \param[in]        port_p                 AMS port.
 *  \param[in]        index_p                16 bit index value from IndexOffset.
 *  \param[in]        subIndex_p             8 bit subIndex value from IndexOffset.
 *  \param[in]        completeAccess_p       CoE Complete Access flag.
 *  \param[in,out]    pLength_p              Request data length.
 *  \param[in]        pData_p                Pointer to data.
 *  \return           ADS error code         32-bit error code
 *
 *  <!-- Group: -->
 *
 *  \ingroup EC_SLV_APP
 *
 * */
static uint32_t EC_SLV_APP_AoE_writeRequest(
    void     *pContext_p,
    uint16_t  port_p,
    uint16_t  index_p,
    uint8_t   subIndex_p,
    bool      completeAccess_p,
    uint32_t *pLength_p,
    uint16_t *pData_p)
{
    uint32_t adsError = ERR_NOERROR;

    EC_API_SLV_SHandle_t       *pEcSlvApi    = (EC_API_SLV_SHandle_t *)pContext_p;
    EC_API_SLV_SCoE_ObjEntry_t *pObjectEntry = NULL;
    EC_API_SLV_SCoE_Object_t   *pObject      = NULL;

    OSALUNREF_PARM(port_p);

    if (pData_p != NULL)
    {
        uint32_t dtkError;
        if (completeAccess_p == false)
        {
            dtkError = EC_API_SLV_CoE_getObjectEntry(pEcSlvApi, index_p, subIndex_p, &pObjectEntry);
            if (dtkError == EC_API_eERR_NONE)
            {
                dtkError
                    = EC_API_SLV_CoE_setObjectEntryData(pEcSlvApi, pObjectEntry, *pLength_p, pData_p);
                if (dtkError != EC_API_eERR_NONE)
                {
                    adsError = ADSERR_DEVICE_ERROR;
                }
            }
            else
            {
                adsError = ADSERR_DEVICE_NOTFOUND;
            }
        }
        else
        {
            dtkError = EC_API_SLV_CoE_getObject(pEcSlvApi, index_p, &pObject);
            if (dtkError == EC_API_eERR_NONE)
            {
                if (subIndex_p == 0u)
                {
                    dtkError = EC_API_SLV_CoE_setObjectData(
                        pEcSlvApi,
                        pObject,
                        subIndex_p,
                        *pLength_p,
                        &pData_p[1]);
                    if (dtkError != EC_API_eERR_NONE)
                    {
                        adsError = ADSERR_DEVICE_ERROR;
                    }
                }
                else if (subIndex_p == 1u)
                {
                    dtkError
                        = EC_API_SLV_CoE_setObjectData(pEcSlvApi, pObject, subIndex_p, *pLength_p, pData_p);
                    if (dtkError != EC_API_eERR_NONE)
                    {
                        adsError = ADSERR_DEVICE_ERROR;
                    }
                }
                else
                {
                    adsError = ADSERR_DEVICE_INVALIDPARM;
                }
            }
            else
            {
                adsError = ADSERR_DEVICE_NOTFOUND;
            }
        }
    }
    else
    {
        adsError = ADSERR_DEVICE_NOMEMORY;
    }
    return adsError;
}

void EC_SLV_APP_SS_applicationInit(EC_SLV_APP_SS_Application_t *pAppInstance_p)
{
    EC_API_EError_t error;

    if (!pAppInstance_p)
    {
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    // Initialize SDK
    pAppInstance_p->ptEcSlvApi = EC_API_SLV_new();
    if (!pAppInstance_p->ptEcSlvApi)
    {
        OSAL_error(__func__, __LINE__, OSAL_CONTAINER_NOMEMORY, true, 0);
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    error = EC_SLV_APP_SS_populateBoardFunctions(pAppInstance_p);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("Populate board functions Error code: 0x%08x\r\n", error);
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    error = EC_SLV_APP_SS_populateSlaveInfo(pAppInstance_p);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("Create SubDevice Info Error code: 0x%08x\r\n", error);
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    /////////////////////////////////////////////////////////
    //////////  Generate application OBD                /////////
    /////////////////////////////////////////////////////////

    // Initialize MDP and pass the stack handle to the HAL
    HAL_Set_ptSubDevice(pAppInstance_p->ptEcSlvApi);
    MDP_Init();
    MDP_CreateObjects();

    error = EC_SLV_APP_populateFSoEObject(pAppInstance_p);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("Create FSoE connection state object error code: 0x%08x\r\n", error);
        goto Exit;
    }

    /////////////////////////////////////////////////////////
    //////////          Define Application PDOs         /////////
    /////////////////////////////////////////////////////////

    /*PDO Mapping Changes*/
    EC_API_SLV_PDO_registerAssignmentChanges(
        pAppInstance_p->ptEcSlvApi,
        EC_SLAVE_APP_assignmentChangedHandler,
        pAppInstance_p);
    EC_API_SLV_PDO_registerMappingChanges(
        pAppInstance_p->ptEcSlvApi,
        EC_SLAVE_APP_mappingChangedHandler,
        pAppInstance_p);

    /*AoE*/
    EC_API_SLV_AoE_cbRegisterReadRequestHandler(
        pAppInstance_p->ptEcSlvApi,
        EC_SLV_APP_AoE_readRequest,
        pAppInstance_p->ptEcSlvApi);
    EC_API_SLV_AoE_cbRegisterWriteRequestHandler(
        pAppInstance_p->ptEcSlvApi,
        EC_SLV_APP_AoE_writeRequest,
        pAppInstance_p->ptEcSlvApi);
    /*EoE*/

    EC_API_SLV_EoE_cbRegisterReceiveHandler(
        pAppInstance_p->ptEcSlvApi,
        EC_SLV_APP_EoE_SS_receiveHandler,
        pAppInstance_p);
    EC_API_SLV_EoE_cbRegisterSettingIndHandler(
        pAppInstance_p->ptEcSlvApi,
        EC_SLV_APP_EoE_SS_settingIndHandler,
        pAppInstance_p);

    /*FoE*/
    EC_API_SLV_FoE_cbRegisterOpenFileHandler(
        pAppInstance_p->ptEcSlvApi,
        EC_SLV_APP_FoE_fileOpen,
        pAppInstance_p->ptEcSlvApi);
    EC_API_SLV_FoE_cbRegisterReadFileHandler(
        pAppInstance_p->ptEcSlvApi,
        EC_SLV_APP_FoE_fileRead,
        pAppInstance_p->ptEcSlvApi);
    EC_API_SLV_FoE_cbRegisterWriteFileHandler(
        pAppInstance_p->ptEcSlvApi,
        EC_SLV_APP_FoE_fileWrite,
        pAppInstance_p->ptEcSlvApi);
    EC_API_SLV_FoE_cbRegisterCloseFileHandler(
        pAppInstance_p->ptEcSlvApi,
        EC_SLV_APP_FoE_fileClose,
        pAppInstance_p->ptEcSlvApi);

    EC_API_SLV_FoE_cbRegisterStartBLHandler(
        pAppInstance_p->ptEcSlvApi,
        EC_SLV_APP_FoE_startBL,
        pAppInstance_p->ptEcSlvApi);
    EC_API_SLV_FoE_cbRegisterStopBLHandler(
        pAppInstance_p->ptEcSlvApi,
        EC_SLV_APP_FoE_stopBL,
        pAppInstance_p->ptEcSlvApi);

    /*Diagnosis support */
    EC_API_SLV_DIAG_enable(pAppInstance_p->ptEcSlvApi);

#if !(defined DPRAM_REMOTE) && !(defined FBTL_REMOTE)                                              \
    && !(defined OSAL_FREERTOS_JACINTO) /* first omit flash */
    EC_API_SLV_EEPROM_cbRegisterInit(
        pAppInstance_p->ptEcSlvApi,
        EC_SLV_APP_EEP_init,
        pAppInstance_p->ptEcSlvApi);
    /* @cppcheck_justify{misra-c2012-11.6} void cast required for signature */
    /* cppcheck-suppress misra-c2012-11.6 */
    EC_API_SLV_EEPROM_cbRegisterWrite(
        pAppInstance_p->ptEcSlvApi,
        EC_SLV_APP_EEP_write, EEPROM_MAGIC_KEY);
    /* @cppcheck_justify{misra-c2012-11.6} void cast required for signature */
    /* cppcheck-suppress misra-c2012-11.6 */
    EC_API_SLV_EEPROM_cbRegisterRead(
        pAppInstance_p->ptEcSlvApi,
        EC_SLV_APP_EEP_read, EEPROM_MAGIC_KEY);
#endif
    error = EC_API_SLV_cbRegisterUserApplicationRun(
        pAppInstance_p->ptEcSlvApi,
        EC_SLV_APP_SS_applicationRun,
        pAppInstance_p);

    error = (EC_API_EError_t)EC_API_SLV_init(pAppInstance_p->ptEcSlvApi);
    if (error != EC_API_eERR_NONE)
    {
        OSAL_printf("SubDevice Init Error Code: 0x%08x\r\n", error);
        /* @cppcheck_justify{misra-c2012-15.1} goto is used to assure single point of exit */
        /* cppcheck-suppress misra-c2012-15.1 */
        goto Exit;
    }

    pAppInstance_p->state   = EC_API_SLV_eESM_init;
    pAppInstance_p->msec    = 0;
    pAppInstance_p->trigger = 1000; /* 1000 ms */

    pAppInstance_p->prev = ESL_OS_clockGet();

    error = EC_API_SLV_run(pAppInstance_p->ptEcSlvApi);
    if (EC_API_eERR_NONE != error)
    {
      OSAL_printf("%s:%d:0x%x\r\n", __func__, __LINE__, error);
    }

Exit:
    return;
}

/// \cond DO_NOT_DOCUMENT
#if (defined SHOW_ESCSTATUS) && (SHOW_ESCSTATUS == 1)
static void EC_SLV_APP_escStatusAnalysis(EC_SLV_APP_SS_Application_t *pAppInstance_p)
{
    static uint16_t lastPortState = 0;
    uint16_t        portState;

    static uint16_t lastCounters[8] = { 0 };
    uint16_t        counters[8]     = { 0 };
    uint8_t         cntIdx          = 0;
    uint32_t        error;

    error = EC_API_SLV_readWordEscRegister(pAppInstance_p->ptEcSlvApi, 0x110, &portState);
    if (EC_API_eERR_NONE != error)
    {
        OSAL_printf("%s:%d:E=0x%x\r\n", __func__, __LINE__, error);
    }
    if (lastPortState != portState)
    {
        OSAL_printf("PortState 0x%04x->0x%04x\r\n", lastPortState, portState);
    }
    lastPortState = portState;

    for (cntIdx = 0; cntIdx < 0x0e; cntIdx += 2)
    {
        error = EC_API_SLV_readWordEscRegister(
            pAppInstance_p->ptEcSlvApi,
            (0x300 | cntIdx),
            &counters[cntIdx >> 1]);

        if (EC_API_eERR_NONE != error)
        {
            OSAL_printf("%s:%d:E=0x%x\r\n", __func__, __LINE__, error);
        }
    }

    if (0 != OSAL_MEMORY_memcmp(counters, lastCounters, sizeof(counters)))
    {
        for (cntIdx = 0; cntIdx < 8; ++cntIdx)
        {
            OSAL_printf("Counter 0x%04x: %04x\r\n", 0x300 | (cntIdx * 2), counters[cntIdx]);
        }
    }
    OSAL_MEMORY_memcpy(lastCounters, counters, sizeof(counters));
}
#endif
/// \endcond

static void EC_SLV_APP_SS_applicationRun(void *pAppCtxt_p)
{
    /* @cppcheck_justify{misra-c2012-11.5} generic API requires cast */
    /* cppcheck-suppress misra-c2012-11.5 */
    EC_SLV_APP_SS_Application_t *pApplicationInstance = (EC_SLV_APP_SS_Application_t *)pAppCtxt_p;
    /* @cppcheck_justify{threadsafety-threadsafety} thread is not reentrant */
    /* cppcheck-suppress threadsafety-threadsafety */
    static uint8_t lastLed = 0;
    /* @cppcheck_justify{threadsafety-threadsafety} thread is not reentrant */
    /* cppcheck-suppress threadsafety-threadsafety */
    static EC_API_SLV_EEsmState_t lastState = EC_API_SLV_eESM_uninit; /* last known stack state to
                                                                         notify changes */
    EC_API_SLV_EEsmState_t curState = EC_API_SLV_eESM_uninit;         /* current stack state */

    /* @cppcheck_justify{threadsafety-threadsafety} thread is not reentrant */
    /* cppcheck-suppress threadsafety-threadsafety */
    uint16_t        alErrorCode = 0;
    uint32_t        error;

#if (defined SHOW_LOOPCOUNT) && (SHOW_LOOPCOUNT == 1)
    static uint32_t loops = 0;
#endif

    if (NULL == pApplicationInstance)
    {
        OSAL_error(__func__, __LINE__, OSAL_ERR_InvalidState, true, 1, "App Instance = NULL");
    }
    else
    {
#if (defined ENABLE_I2CLEDS) && (ENABLE_I2CLEDS == 1)
        /* be sure we own I2C in this thread */
        if (NULL == pApplicationInstance->ioexpLedHandle)
        {
            pApplicationInstance->ioexpLedHandle = ESL_OS_ioexp_leds_init();
        }
#endif

        error = EC_API_SLV_getState(pApplicationInstance->ptEcSlvApi, &curState, &alErrorCode);
        if (EC_API_eERR_NONE != error)
        {
            OSAL_printf("%s:%d:E=0x%x\r\n", __func__, __LINE__, error);
        }

        // if (curState != lastState)
        // {
        //     OSAL_printf("State change: 0x%x -> 0x%x\r\n", lastState, curState);
        //     pApplicationInstance->prev = ESL_OS_clockGet();
        // }

        if (curState != lastState) {
            OSAL_printf( "State change: 0x%x -> 0x%x\r\n ", lastState, curState);
            pApplicationInstance->prev = ESL_OS_clockGet();
            
            MDP_Check_State(curState, lastState);

            // --- MDP STATE MACHINE HOOKS ---
            if (curState == EC_API_SLV_eESM_preop) {
                MDP_OnEnterPreOP();
            } else if (curState == EC_API_SLV_eESM_safeop) {
                // Validate F030 vs F050
                if (!MDP_ValidateAndConfigure()) {
                    // Validation failed! Force back to PREOP with error code 0x0030
                    OSAL_printf("MDP Validation Failed! Forcing back to PREOP.\r\n");
                    
                    // Use EC_API_SLV_setState to atomically reject and set error
                    EC_API_SLV_setState(
                        pApplicationInstance->ptEcSlvApi,
                        EC_API_SLV_eESM_preop,
                        AL_STATUS_MODULE_IDENT_MISMATCH  // 0x0030
                    );
                } else {
                    OSAL_printf("Successfully entered SAFEOP.\r\n");
                } 
            } else if (curState == EC_API_SLV_eESM_op) {
                MDP_OnEnterOP();
            } else if (curState == EC_API_SLV_eESM_init) {
                MDP_OnEnterInit();
            }
        }

        lastState = curState;

        if (EC_API_SLV_eESM_op == curState)
        {
            MDP_Cyclic(); 
        }
        else
        {
            clock_t now = ESL_OS_clockGet();

            if (pApplicationInstance->prev > now)
            {
                pApplicationInstance->prev = now;
            }
            pApplicationInstance->diff = ESL_OS_clockDiff(pApplicationInstance->prev, &now);

            if (pApplicationInstance->ioexpLedHandle && pApplicationInstance->diff)
            {
#if (defined SHOW_LOOPCOUNT) && (SHOW_LOOPCOUNT == 1)
                OSAL_printf("LoopCount 0x%08x\r\n", loops);
#endif

#if (defined ENABLE_I2CLEDS) && (ENABLE_I2CLEDS == 1)
                ESL_OS_ioexp_leds_write(pApplicationInstance->ioexpLedHandle, lastLed);
#else
                OSAL_printf("Update I2C to 0x%x\r\n", lastLed);
#endif

                pApplicationInstance->prev = now;

                if (0u == lastLed)
                {
                    lastLed = 1u;
                }
                else
                {
                    lastLed = (lastLed << 1u);
                }
            }
        }

#if (defined SHOW_ESCSTATUS) && (SHOW_ESCSTATUS == 1)
        EC_SLV_APP_escStatusAnalysis(pApplicationInstance);
#endif
#if (defined SHOW_LOOPCOUNT) && (SHOW_LOOPCOUNT == 1)
        loops++;
#endif

    }
}
