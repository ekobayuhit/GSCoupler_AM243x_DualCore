#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "ipc_shareMem.h"

#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/AddrTranslateP.h>
#include <kernel/dpl/SemaphoreP.h>
#include <drivers/spinlock.h>
#include <drivers/ipc_notify.h>
#include <kernel/dpl/CacheP.h>
#include <kernel/dpl/ClockP.h>

#define SPINLOCK_BASE_ADDR      (CSL_SPINLOCK0_BASE)
#define SPINLOCK_LOCK_NUM       (0U)

volatile ipc_data_t gSharedMem __attribute__((aligned(128), section(".bss.user_shared_mem")));
static uint32_t            spinlockBaseAddr;
static SemaphoreP_Object   mutexObj;

uint32_t curr_time = 0, last_time = 0;

void app_cache_read_sharemem(void){
    CacheP_inv(
        (void *)&gSharedMem,
        sizeof(gSharedMem),
        CacheP_TYPE_ALL
    );
}

void app_cache_write_sharemem(void){
    CacheP_wb(
        (void *)&gSharedMem,
        sizeof(gSharedMem),
        CacheP_TYPE_ALL
    );
}

void init_ipc_sharemem(void){
    int32_t  status;

    // memset((void *)&gSharedMem, 0, sizeof(ipc_data_t));

    // DebugP_log("Waiting for all cores to start ...\r\n");
    
    // /* Wait for all cores to start */
    // IpcNotify_syncAll(SystemP_WAIT_FOREVER);

    /* Get address after translation translate */
    spinlockBaseAddr = (uint32_t) AddrTranslateP_getLocalAddr(SPINLOCK_BASE_ADDR);

    /* Mutex for local thread safe operation on spinlock */
    status = SemaphoreP_constructMutex(&mutexObj);
    DebugP_assert(status==SystemP_SUCCESS);
}

void app_ipc_sharemem_lock(void)
{
    int32_t  status;

    /* Take local mutex to protect against multi-thread from same core */
    SemaphoreP_pend(&mutexObj, SystemP_WAIT_FOREVER);

    /* Spin till lock is acquired */
    while(1U)
    {
        status = Spinlock_lock(spinlockBaseAddr, SPINLOCK_LOCK_NUM);
        if(status == SPINLOCK_LOCK_STATUS_FREE)
        {
            break;  /* Free and taken */
        }
        /* Note: Customers can implement timeout instead of wait forever */
    }

    __asm__ __volatile__  ("  dsb"   "\n\t": : : "memory");
    app_cache_read_sharemem();
    
    return;
}

void app_ipc_sharemem_unlock(void)
{
    app_cache_write_sharemem();
    __asm__ __volatile__  ("  dsb"   "\n\t": : : "memory");
    
    /* Free the aquired lock and release local mutex */
    Spinlock_unlock(spinlockBaseAddr, SPINLOCK_LOCK_NUM);
    SemaphoreP_post(&mutexObj);

    return;
}

void init_ipc_data(bool is_core0){
    app_ipc_sharemem_lock();
    if(is_core0){
        memset(&gSharedMem.IOCoupler_Devices, 0, sizeof(gSharedMem.IOCoupler_Devices));
        memset(gSharedMem.buff_in, 0 , sizeof(gSharedMem.buff_in));
        memset(gSharedMem.buff_out, 0, sizeof(gSharedMem.buff_out));
        memset(&gSharedMem.ipc_sys.core0_mcan, 0, sizeof(gSharedMem.ipc_sys.core0_mcan));
        gSharedMem.ipc_sys.master_state = 0;
        gSharedMem.ipc_sys.ws_scan_status = SCAN_STATUS_IDLE;
    }else{
        memset(&gSharedMem.ipc_sys.core1_indcomm, 0, sizeof(gSharedMem.ipc_sys.core1_indcomm));
        gSharedMem.ipc_sys.active_protocol = 0;
        gSharedMem.ipc_sys.ws_cmd = WS_NONE;
    }
    app_ipc_sharemem_unlock();
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

void Master_print_io_info(void)
{
    curr_time = ClockP_getTimeUsec() / 1000;
    
    if(curr_time - last_time > 5000){
        last_time = curr_time;

        DebugP_log("Master_print_io_info. Total Slaves Found: %u\r\n", gSharedMem.IOCoupler_Devices.numberOfSlaves);

        if (gSharedMem.IOCoupler_Devices.numberOfSlaves == 0) {
            DebugP_log("No IO slaves detected on the bus.\r\n");
        } else {
            for (uint8_t i = 0; i < gSharedMem.IOCoupler_Devices.numberOfSlaves; i++) {
                const IO_SlaveInfo *slave = (IO_SlaveInfo *)&gSharedMem.IOCoupler_Devices.slaveInfo[i];
                DebugP_log("[Slot-%02u] ID: 0x%02X | Type: %-6s | State: %u | Error : %u %u\r\n",
                    i+1,
                    slave->nodeId,
                    IO_typename(slave->productCode),
                    slave->nodeState,
                    slave->last_error_type,
                    slave->last_error_code);
            }
        }
        DebugP_log("-------------------------------\r\n");
    }
}

