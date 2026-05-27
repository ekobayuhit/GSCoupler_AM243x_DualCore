/*
 *  Copyright (C) 2021 Texas Instruments Incorporated
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

/* This example demonstrates the CAN message transmission and reception in
 * digital loop back mode with the following configuration.
 *
 * CAN FD Message Format.
 * Message ID Type is Standard, Msg Id 0xC0.
 * MCAN is configured in Interrupt Mode.
 * MCAN Interrupt Line Number 0.
 * Arbitration Bit Rate 1Mbps.
 * Data Bit Rate 5Mbps.
 * Buffer mode is used for Tx and RX to store message in message RAM.
 *
 * Message is transmitted and received back internally using internal loopback
 * mode. When the received message id and the data matches with the transmitted
 * one, then the example is completed.
 *
 */

#include <stdio.h>
#include <string.h>
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/HwiP.h>
#include <kernel/dpl/AddrTranslateP.h>
#include <kernel/dpl/SemaphoreP.h>
#include <drivers/mcan.h>
#include "ti_drivers_config.h"
#include "ti_drivers_open_close.h"
#include "ti_board_open_close.h"

#include "candrv.h"

#define MCAN_RX_QUEUE_LENGTH    1000
#define MCAN_RX_ITEM_SIZE       sizeof( Message )

/* Semaphore to indicate transfer completion */
static SemaphoreP_Object gMcanTxDoneSem, gMcanRxDoneSem;
static uint32_t          gMcanBaseAddr;
static HwiP_Object       gMcanHwiObject;

static StaticQueue_t McanRx_xStaticQueue;

/* Static Function Declarations */
static void    App_mcanConfig();
static void    App_mcanInitMsgRamConfigParams(
               MCAN_MsgRAMConfigParams *msgRAMConfigParams);
static void    App_mcanCompareMsg(MCAN_TxBufElement *txMsg,
                                  MCAN_RxBufElement *rxMsg);

QueueHandle_t xMcanRxQueue;
static Bool can_lock = 0;

uint8_t McanRxQueueData[ MCAN_RX_QUEUE_LENGTH * MCAN_RX_ITEM_SIZE ];


#if APP_MCAN_MODE == MCAN_MODE_INTERRUPT
static void    App_mcanIntrISR(void *arg);
static void canIntcfg(void);
static void App_mcanEnableIntr(void);
#endif

app_filter_t app_filter[] = {
    {MCANOPEN_STDFILTER_HEARTBEAT, 0x701}, // 0x700 + nodeId
    {MCANOPEN_STDFILTER_SDO,       0x581}, // 0x580 + nodeId
    {MCANOPEN_STDFILTER_EMCY,      0x081}, // 0x080 + nodeId
    {MCANOPEN_STDFILTER_TPDO1,     0x181}, // 0x180 + nodeId
    {MCANOPEN_STDFILTER_TPDO2,     0x281}  // 0x280 + nodeId
};

void canInit(void){
    int32_t status = SystemP_SUCCESS;

    /* Construct Tx/Rx Semaphore objects */
    status = SemaphoreP_constructBinary(&gMcanTxDoneSem, 0);
    DebugP_assert(SystemP_SUCCESS == status);
    status = SemaphoreP_constructBinary(&gMcanRxDoneSem, 0);
    DebugP_assert(SystemP_SUCCESS == status);

    /* Assign MCAN instance address */
    gMcanBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(APP_MCAN_BASE_ADDR);

	xMcanRxQueue = xQueueCreateStatic( MCAN_RX_QUEUE_LENGTH,
                                 MCAN_RX_ITEM_SIZE,
                                 McanRxQueueData,
                                 &McanRx_xStaticQueue );
	configASSERT( xMcanRxQueue );
    
    /* Configure MCAN module */
    App_mcanConfig();
}

void canDeinit(void){
    SemaphoreP_destruct(&gMcanTxDoneSem);
    SemaphoreP_destruct(&gMcanRxDoneSem);
}

int canSend(CAN_PORT port, Message *m)
{
    (void)port;
    
    int32_t status = SystemP_SUCCESS;
    uint32_t txStatus, bitPos;
    MCAN_ProtocolStatus protStatus;
    MCAN_TxBufElement txMsg;

#if APP_DEBUG_MCAN_TX
    int count_data = 0;
    char data[2*8 + 1];
#endif

    if(can_lock){
        return -1;
    }
    can_lock = 1;
    MCAN_initTxBufElement(&txMsg);
    
    txMsg.id = ((m->cob_id & MCAN_STD_ID_MASK) << MCAN_STD_ID_SHIFT);
    txMsg.dlc = m->len;
    txMsg.fdf = FALSE; /* CAN FD Frame Format */
    txMsg.xtd = FALSE; /* Extended id not configured */
    memcpy(txMsg.data, m->data, txMsg.dlc);

#if APP_DEBUG_MCAN_TX
    memset(data, 0, sizeof(data));
    for (int i = 0; i < txMsg.dlc; i++)
    {
        sprintf(&data[i*2], "%02x", txMsg.data[i]);
    }
    data[txMsg.dlc*2] = '\0';
    DebugP_log("[MCAN] canSend no:%d id: 0x%x dlc %d data: 0x%s\r\n",
        count_data++,
        m->cob_id,
        m->len,
        data);
#endif

#if (APP_MCAN_MODE == MCAN_MODE_POLLING)
    /* Write message to Msg RAM */
    MCAN_writeMsgRam(gMcanBaseAddr, MCAN_MEM_TYPE_FIFO, 0, &txMsg);

    /* Add request for transmission, This function will trigger transmission */
    status = MCAN_txBufAddReq(gMcanBaseAddr, 0);
    DebugP_assert(status == CSL_PASS);

    bitPos = (1U << 0);
    /* Poll for Tx completion */
    do
    {
        txStatus = MCAN_getTxBufTransmissionStatus(gMcanBaseAddr);
    }while((txStatus & bitPos) != bitPos);

    MCAN_getProtocolStatus(gMcanBaseAddr, &protStatus);
    /* Checking for Tx Errors */
    if (((MCAN_ERR_CODE_NO_ERROR != protStatus.lastErrCode) ||
            (MCAN_ERR_CODE_NO_CHANGE != protStatus.lastErrCode)) &&
        ((MCAN_ERR_CODE_NO_ERROR != protStatus.dlec) ||
            (MCAN_ERR_CODE_NO_CHANGE != protStatus.dlec)) &&
        (0U != protStatus.pxe))
    {
        DebugP_assert(FALSE);
    }
#endif
#if (APP_MCAN_MODE == MCAN_MODE_INTERRUPT)
        uint8_t bufNum = 0U;
        /* Enable Transmission interrupt for the selected buf num,
         * If FIFO is used, then need to send FIFO start index until FIFO count */
        status = MCAN_txBufTransIntrEnable(gMcanBaseAddr, bufNum, (uint32_t)TRUE);
        DebugP_assert(status == CSL_PASS);

        /* Write message to Msg RAM */
        MCAN_writeMsgRam(gMcanBaseAddr, MCAN_MEM_TYPE_BUF, bufNum, &txMsg);

        /* Add request for transmission, This function will trigger transmission */
        status = MCAN_txBufAddReq(gMcanBaseAddr, bufNum);
        DebugP_assert(status == CSL_PASS);

        /* Wait for Tx completion */
        SemaphoreP_pend(&gMcanTxDoneSem, SystemP_WAIT_FOREVER);

        MCAN_getProtocolStatus(gMcanBaseAddr, &protStatus);
        /* Checking for Tx Errors */
        if (((MCAN_ERR_CODE_NO_ERROR != protStatus.lastErrCode) ||
             (MCAN_ERR_CODE_NO_CHANGE != protStatus.lastErrCode)) &&
            ((MCAN_ERR_CODE_NO_ERROR != protStatus.dlec) ||
             (MCAN_ERR_CODE_NO_CHANGE != protStatus.dlec)) &&
            (0U != protStatus.pxe))
        {
             DebugP_assert(FALSE);
        }
#endif
    can_lock = 0;
	return 0;
}

void canRecv_Task(void *args)
{
    // CO_Data* d = args;
    
    static Message m = Message_Initializer;		// contain a CAN message
    MCAN_ErrCntStatus       errCounter;
#if (APP_MCAN_MODE == MCAN_MODE_INTERRUPT)        
    MCAN_RxNewDataStatus    newDataStatus;
    uint32_t bufNum, fifoNum, bitPos = 0U;
#endif
    MCAN_RxFIFOStatus fifoStatus;
    MCAN_RxBufElement rxMsg;

#if APP_DEBUG_MCAN_RX
    int count_data = 1;
    char data[2*8 + 1];
#endif
    
    // DebugP_log("[MCAN] canRecv_Task started ...\r\n");

    while(1){
#if (APP_MCAN_MODE == MCAN_MODE_POLLING)
        for(int i = 0U; i < 2; i++){
            fifoStatus.num = i;
        
            /* Checking for Rx Errors */
            MCAN_getErrCounters(gMcanBaseAddr, &errCounter);
            if ((errCounter.recErrCnt >= REC_THRESHOLD) &&
                (errCounter.canErrLogCnt >= CEL_THRESHOLD))
            {
                DebugP_log("ERROR: REC=%d, TEC=%d, CEL=%d\n",
                            errCounter.recErrCnt,
                            errCounter.transErrLogCnt,
                            errCounter.canErrLogCnt);

                DebugP_assert(0); // only assert when exceeding tolerance
            }
            
            MCAN_getRxFIFOStatus(gMcanBaseAddr, &fifoStatus);
            if (fifoStatus.fillLvl > 0)
            {
                MCAN_readMsgRam(gMcanBaseAddr, MCAN_MEM_TYPE_FIFO, fifoStatus.getIdx,
                                fifoStatus.num, &rxMsg);
            (void) MCAN_writeRxFIFOAck(gMcanBaseAddr, fifoStatus.num,
                                        fifoStatus.getIdx);
                
                if (rxMsg.dlc > 8){
                    continue;
                }

                m.cob_id = (rxMsg.id >> APP_MCAN_STD_ID_SHIFT) & 0x7FF;
                m.rtr = rxMsg.rtr;
                m.len = rxMsg.dlc;
                memcpy(m.data, rxMsg.data, (m.len <= 8) ? m.len : 8);
                
                xQueueSend(xMcanRxQueue, &m, 0);
                // canDispatch(d, &m);
            }
#if APP_DEBUG_MCAN_RX
            memset(data, 0, sizeof(data));
            for (int i = 0; i < m.len; i++)
            {
                sprintf(&data[i*2], "%02x", m.data[i]);
            }
            data[m.len*2] = '\0';
            DebugP_log("[MCAN] canRecv no:%d id: 0x%x dlc %d data: 0x%s\r\n",
                    count_data++,
                    m.cob_id,
                    m.len,
                    data);
#endif
        }
        vTaskDelay(1);
#endif  /* MCAN_MODE_POLLING */

#if (APP_MCAN_MODE == MCAN_MODE_INTERRUPT)        
        /* Wait for Rx completion */
        SemaphoreP_pend(&gMcanRxDoneSem, SystemP_WAIT_FOREVER);
        
        DebugP_log("[MCAN] Recv \r\n");
        
        /* Checking for Rx Errors */
        MCAN_getErrCounters(gMcanBaseAddr, &errCounter);
        DebugP_assert((0U == errCounter.recErrCnt) &&
                      (0U == errCounter.canErrLogCnt));

        /* Get the new data staus, indicates buffer num which received message */
        MCAN_getNewDataStatus(gMcanBaseAddr, &newDataStatus);
        MCAN_clearNewDataStatus(gMcanBaseAddr, &newDataStatus);

        /* Select buffer and fifo number, Buffer is used in this app */
        bufNum = 0U;
        fifoNum = MCAN_RX_FIFO_NUM_0;

        bitPos = (1U << bufNum);
        if (bitPos == (newDataStatus.statusLow & bitPos))
        {
            MCAN_readMsgRam(gMcanBaseAddr, MCAN_MEM_TYPE_BUF, bufNum, fifoNum, &rxMsg);

            m.cob_id = (rxMsg.id >> APP_MCAN_STD_ID_SHIFT) & 0x7FF;
            m.rtr = rxMsg.rtr;
            m.len = rxMsg.dlc;
            memcpy(m.data, &rxMsg.data[0], m.len);

            canDispatch(d, &m);

        #if 1 
            char data[2*8 + 1];
            memset(data, 0, sizeof(data));
            for (int i = 0; i < m.len; i++)
            {
                sprintf(&data[i*2], "%02x", m.data[i]);
            }
            data[m.len*2] = '\0';
            DebugP_log("[MCAN] canRecv id: 0x%x dlc %d data: 0x%s\r\n",
                    m.cob_id,
                    m.len,
                    data);
        #endif
        }
#endif /* MCAN_MODE_INTERRUPT */
    }
}

static void App_mcanConfig()
{
    MCAN_StdMsgIDFilterElement stdFiltElem[sizeof(app_filter)/sizeof(app_filter_t)] = {0U};
    MCAN_InitParams            initParams = {0U};
    MCAN_ConfigParams          configParams = {0U};
    MCAN_MsgRAMConfigParams    msgRAMConfigParams = {0U};
    MCAN_BitTimingParams       bitTimes = {0U};

    /* Initialize MCAN module initParams */
    MCAN_initOperModeParams(&initParams);
    /* CAN FD Mode and Bit Rate Switch Enabled */
    initParams.fdMode          = FALSE; //0 = FD operation disabled
    initParams.brsEnable       = FALSE; //0 = Bit rate switching for transmissions disabled
    initParams.txpEnable       = FALSE; //0 = Transmit pause disabled
    initParams.pxhddisable     = FALSE; //0 = Protocol exception handling enabled
    initParams.darEnable       = FALSE; //0 = Automatic retransmission of messages not transmitted successfully enabled
    initParams.tdcEnable       = FALSE; //0 = Transmitter Delay Compensation is disabled
    initParams.emulationEnable = FALSE; //0 = Emulation/Debug Suspend does not wait for idle/immediate effect
    
    /* Initialize MCAN module Global Filter Params */
    MCAN_initGlobalFilterConfigParams(&configParams);
    configParams.filterConfig.anfe = 0x2;
    configParams.filterConfig.anfs = 0x2;
    configParams.filterConfig.rrfe = 0x0;
    configParams.filterConfig.rrfs = 0x0;
    
    /* Initialize MCAN module Bit Time Params */
    /* Configuring default 1Mbps and 5Mbps as nominal and data bit-rate resp */
    MCAN_initSetBitTimeParams(&bitTimes);
    
#if (APP_MCAN_DEFAULT_BAUD == MCAN_BAUD_1000K)
    /* Default Baudrate 1Mbps */
    bitTimes.nomRatePrescalar   = 0x7U; // Nominal Baud Rate Pre-scaler
    bitTimes.nomTimeSeg1        = 0x5U; // Nominal Time segment before SP
    bitTimes.nomTimeSeg2        = 0x2U; // Nominal Time segment after SP
    bitTimes.nomSynchJumpWidth  = 0x0U; // Nominal SJW
    bitTimes.dataRatePrescalar  = 0x7U; // Data Baud Rate Pre-scaler
    bitTimes.dataTimeSeg1       = 0x5U; // Data Time segment before SP
    bitTimes.dataTimeSeg2       = 0x2U; // Data Time segment after SP
    bitTimes.dataSynchJumpWidth = 0x0U; // Data SJW
#endif
    /* Initialize MCAN module Message Ram Params */
    App_mcanInitMsgRamConfigParams(&msgRAMConfigParams);

    /* wait for memory initialization to happen */
    while (FALSE == MCAN_isMemInitDone(gMcanBaseAddr))
    {}

    /* Put MCAN in SW initialization mode */
    MCAN_setOpMode(gMcanBaseAddr, MCAN_OPERATION_MODE_SW_INIT);
    while (MCAN_OPERATION_MODE_SW_INIT != MCAN_getOpMode(gMcanBaseAddr))
    {}

    /* Initialize MCAN module */
    MCAN_init(gMcanBaseAddr, &initParams);
    /* Configure MCAN module Gloabal Filter */
    MCAN_config(gMcanBaseAddr, &configParams);
    /* Configure Bit timings */
    MCAN_setBitTime(gMcanBaseAddr, &bitTimes);
    /* Configure Message RAM Sections */
    MCAN_msgRAMConfig(gMcanBaseAddr, &msgRAMConfigParams);
    /* Set Extended ID Mask */
    MCAN_setExtIDAndMask(gMcanBaseAddr, APP_MCAN_EXT_ID_MASK);

    /* Configure Standard ID filter element */
    for(int i = 0; i < sizeof(app_filter)/sizeof(app_filter_t); i++){
        if(app_filter[i].filter_type != (uint8_t)i){
			continue;
		}
        
        stdFiltElem[i].sfid1 = app_filter[i].std_start_addr;
        stdFiltElem[i].sfid2 = stdFiltElem[i].sfid1 + 128;
        stdFiltElem[i].sft   = MCAN_STD_FILT_TYPE_RANGE;

        if(app_filter[i].filter_type >= MCANOPEN_STDFILTER_TPDO1){
            stdFiltElem[i].sfec  = MCAN_STD_FILT_ELEM_FIFO1;
        }else{
            stdFiltElem[i].sfec  = MCAN_STD_FILT_ELEM_FIFO0;
        }
        
        MCAN_addStdMsgIDFilter(gMcanBaseAddr, i, &stdFiltElem[i]);
    }
    
    /* Take MCAN out of the SW initialization mode */
    MCAN_setOpMode(gMcanBaseAddr, MCAN_OPERATION_MODE_NORMAL);
    while (MCAN_OPERATION_MODE_NORMAL != MCAN_getOpMode(gMcanBaseAddr))
    {}

#if APP_MCAN_MODE == MCAN_MODE_INTERRUPT
    canIntcfg();

    App_mcanEnableIntr();
#endif

    return;
}

static void App_mcanInitMsgRamConfigParams(MCAN_MsgRAMConfigParams
                                           *msgRAMConfigParams)
{
    int32_t status;

    MCAN_initMsgRamConfigParams(msgRAMConfigParams);

    /* Configure the user required msg ram params */
    msgRAMConfigParams->lss = APP_MCAN_STD_ID_FILTER_CNT;
    msgRAMConfigParams->lse = APP_MCAN_EXT_ID_FILTER_CNT;
    msgRAMConfigParams->txBufCnt = APP_MCAN_TX_BUFF_CNT;
    msgRAMConfigParams->txFIFOCnt = APP_MCAN_TX_FIFO_CNT;
    /* Buffer/FIFO mode is selected */
    msgRAMConfigParams->txBufMode = MCAN_TX_MEM_TYPE_BUF;
    msgRAMConfigParams->txEventFIFOCnt = APP_MCAN_TX_EVENT_FIFO_CNT;
    msgRAMConfigParams->rxFIFO0Cnt = APP_MCAN_FIFO_0_CNT;
    msgRAMConfigParams->rxFIFO1Cnt = APP_MCAN_FIFO_1_CNT;
    /* FIFO blocking mode is selected */
    msgRAMConfigParams->rxFIFO0OpMode = MCAN_RX_FIFO_OPERATION_MODE_OVERWRITE;
    msgRAMConfigParams->rxFIFO1OpMode = MCAN_RX_FIFO_OPERATION_MODE_OVERWRITE;

    status = MCAN_calcMsgRamParamsStartAddr(msgRAMConfigParams);
    DebugP_assert(status == CSL_PASS);

    return;
}

#if APP_MCAN_MODE == MCAN_MODE_INTERRUPT
static void canIntcfg(void){
    int32_t                 status = SystemP_SUCCESS;
    HwiP_Params             hwiPrms;
    /* Register interrupt */
    HwiP_Params_init(&hwiPrms);
    hwiPrms.intNum      = APP_MCAN_INTR_NUM;
    hwiPrms.callback    = &App_mcanIntrISR;
    status              = HwiP_construct(&gMcanHwiObject, &hwiPrms);
    DebugP_assert(status == SystemP_SUCCESS);
}

static void App_mcanEnableIntr(void)
{
    MCAN_enableIntr(gMcanBaseAddr, MCAN_INTR_MASK_ALL, (uint32_t)TRUE);
    MCAN_enableIntr(gMcanBaseAddr,
                    MCAN_INTR_SRC_RES_ADDR_ACCESS, (uint32_t)FALSE);
    /* Select Interrupt Line 0 */
    MCAN_selectIntrLine(gMcanBaseAddr, MCAN_INTR_MASK_ALL, MCAN_INTR_LINE_NUM_0);
    /* Enable Interrupt Line */
    MCAN_enableIntrLine(gMcanBaseAddr, MCAN_INTR_LINE_NUM_0, (uint32_t)TRUE);

    return;
}

static void App_mcanIntrISR(void *arg)
{
    uint32_t intrStatus;

    intrStatus = MCAN_getIntrStatus(gMcanBaseAddr);
    MCAN_clearIntrStatus(gMcanBaseAddr, intrStatus);

    if (MCAN_INTR_SRC_TRANS_COMPLETE ==
        (intrStatus & MCAN_INTR_SRC_TRANS_COMPLETE))
    {
        SemaphoreP_post(&gMcanTxDoneSem);
    }

    /* If FIFO0/FIFO1 is used, then MCAN_INTR_SRC_DEDICATED_RX_BUFF_MSG macro
     * needs to be replaced by MCAN_INTR_SRC_RX_FIFO0_NEW_MSG/
     * MCAN_INTR_SRC_RX_FIFO1_NEW_MSG respectively */
    if (MCAN_INTR_SRC_RX_FIFO0_NEW_MSG ==
        (intrStatus & MCAN_INTR_SRC_DEDICATED_RX_BUFF_MSG))
    {
        SemaphoreP_post(&gMcanRxDoneSem);
    }

    return;
}
#endif