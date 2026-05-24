#ifndef CANDRV_H
#define CANDRV_H

#include "FreeRTOS.h"
#include "queue.h"

#include "data.h"
#include "can.h"

/*Mcan Mode*/
#define MCAN_MODE_POLLING       (1)
#define MCAN_MODE_INTERRUPT     (2)
#define MCAN_MODE_DMA           (3)

/*Mcan Baudrate*/
#define MCAN_BAUD_500K          (1)
#define MCAN_BAUD_1000K         (2)
#define MCAN_BAUD_5000K         (3)

#define APP_MCAN_MODE           (MCAN_MODE_POLLING)
#define APP_MCAN_BASE_ADDR      (CONFIG_MCAN0_BASE_ADDR)
#define APP_MCAN_INTR_NUM       (CONFIG_MCAN0_INTR)

#define APP_MCAN_DEFAULT_BAUD   (MCAN_BAUD_1000K)
#define APP_DEBUG_MCAN_TX          (0)
#define APP_DEBUG_MCAN_RX          (0)

#define REC_THRESHOLD           (5U)
#define CEL_THRESHOLD           (5U)

/* Allocate Message RAM memory section to filter elements, buffers, FIFO */
/* Maximum STD Filter Element can be configured is 128 */
#define APP_MCAN_STD_ID_FILTER_CNT               (5U)
/* Maximum EXT Filter Element can be configured is 64 */
#define APP_MCAN_EXT_ID_FILTER_CNT               (0U)
/* Maximum TX Buffer + TX FIFO, combined can be configured is 32 */
#define APP_MCAN_TX_BUFF_CNT                     (32U)
#define APP_MCAN_TX_FIFO_CNT                     (0U)
/* Maximum TX Event FIFO can be configured is 32 */
#define APP_MCAN_TX_EVENT_FIFO_CNT               (0U)
/* Maximum RX FIFO 0 can be configured is 64 */
#define APP_MCAN_FIFO_0_CNT                      (64U)
/* Maximum RX FIFO 1 can be configured is 64 and
 * rest of the memory is allocated to RX buffer which is again of max size 64 */
#define APP_MCAN_FIFO_1_CNT                      (64U)

/* Standard Id configured in this app */
#define APP_MCAN_STD_ID                          (0x01U)
#define APP_MCAN_STD_ID_MASK                     (0x7FFU)
#define APP_MCAN_STD_ID_SHIFT                    (18U)

#define APP_MCAN_EXT_ID_MASK                     (0x1FFFFFFFU)

typedef enum {
    MCANOPEN_STDFILTER_HEARTBEAT = 0,
    MCANOPEN_STDFILTER_SDO,
    MCANOPEN_STDFILTER_EMCY,
    MCANOPEN_STDFILTER_TPDO1,
    MCANOPEN_STDFILTER_TPDO2,
} mcanopen_stdfilter_t;

typedef struct {
    mcanopen_stdfilter_t filter_type;
    uint32_t std_start_addr;
} app_filter_t;

extern QueueHandle_t xMcanRxQueue;

void canInit(void);
void canDeinit(void);
int canSend(CAN_PORT port, Message *m);
void canRecv_Task(void *arg);

#endif /*CANDRV_H*/