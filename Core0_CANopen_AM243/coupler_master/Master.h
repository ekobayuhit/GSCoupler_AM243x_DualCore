/*
This file is part of CanFestival, a library implementing CanOpen Stack. 

Copyright (C): Edouard TISSERANT and Francis DUPIN

See COPYING file for copyrights details.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "canfestival.h"

#define MASTER_TYPE	        "GS-Coupler-EtherNet/IP"
#define MASTER_FW_VERSION	"Alpha-1.0.0-050526"

typedef void (*TaskCallback_t)(void *);

enum task_id
{
    TASK_MAIN = 0,
    TASK_CAN_TIMER,
    TASK_CAN_RX,
    TASK_CAN_RPDO,
    TASK_CAN_TPDO,
    TASK_END
};

typedef struct
{
    enum task_id TaskId;
    uint8_t TaskPrio;
    uint32_t TaskSize;
    StackType_t *TaskStack;
    StaticTask_t *TaskObj;
    TaskHandle_t TaskHandle;
    TaskCallback_t callback;
} task_t;

typedef enum{
    SYS_NO_EVENT = 0x00,
    SYS_CMD_MASTER_RESCAN
}sys_event_type_t;

typedef enum{
    CANOPEN_IO_NO_ERR   = 0x00,
    CANOPEN_IO_EMCY,
    CANOPEN_IO_HEARTBEAT,
    CANOPEN_IO_WRONG_MODE_OP,
    CANOPEN_IO_REBOOT
}CANOpen_err_type_t;

typedef enum {
    HEARTBEAT_ERR_TIMEOUT = 0x01,
    HEARTBEAT_ERR_RECOVERED,
} heartbeat_error_t;

typedef struct
{
    uint8_t m_state;
} ipc_var_t;

void trigger_master_rescan(void);
void get_master_firmware_ver(char *buff1, char *buff2, size_t dest_size);
bool get_master_rescan_status(void);
