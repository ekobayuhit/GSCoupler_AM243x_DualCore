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
#include <kernel/dpl/DebugP.h>
#include <kernel/dpl/ClockP.h>
#include <kernel/dpl/TaskP.h>
#include "FreeRTOS.h"
#include "task.h"

#include "candrv.h"
#include "timerdrv.h"

#include "Master.h"
#include "ODMaster.h"
#include "IOCoupler.h"

#if (ACTIVE_PROTOCOL == IOCOUPLER_MODBUSTCP)
#include "mbs_app.h"
#endif

extern IOCoupler_Device IOCoupler_Devices;
IO_SlaveInfo *Slaveconfig;

// Step counts number of times ConfigureSlaveNode is called
// There is one per each slave
static UNS32 init_step[NMT_MAX_NODE_ID];

TaskHandle_t EIPThread = NULL;
TaskHandle_t MBTCPThread = NULL;
CAN_HANDLE canHandle;

void master_loop(void *args);
void SendPDOLoop(void *args);
void RecvPDOLoop(void *args);

#if (ACTIVE_PROTOCOL == IOCOUPLER_MODBUSTCP)
extern void appMain(void *args);
extern MbsStorage mbs_app_storage;
#elif (ACTIVE_PROTOCOL == IOCOUPLER_ETHERNETIP)
uint8_t gInputProcessImage[MAX_INPUT_SIZE_BYTES];   // T -> O (Device → PLC)
uint8_t gOutputProcessImage[MAX_OUTPUT_SIZE_BYTES]; // O -> T (PLC → Device)
#endif

#define MAIN_TASK_SIZE   	2048U
#define TIMER_TASK_SIZE   	2048U
#define CAN_RX_TASK_SIZE    4096U
#define RPDO_TASK_SIZE    	4096U
#define TPDO_TASK_SIZE    	4096U
#define INDSCOM_TASK_SIZE	16384U

StackType_t gMainTaskStack[MAIN_TASK_SIZE] __attribute__((aligned(32)));
StaticTask_t gMainTaskObj;

StackType_t gTimerTaskStack[TIMER_TASK_SIZE] __attribute__((aligned(32)));
StaticTask_t gTimerTaskObj;

StackType_t gRXTaskStack[CAN_RX_TASK_SIZE] __attribute__((aligned(32)));
StaticTask_t gRXTaskObj;

StackType_t gRPDOTaskStack[RPDO_TASK_SIZE] __attribute__((aligned(32)));
StaticTask_t gRPDOTaskObj;

StackType_t gTPDOTaskStack[TPDO_TASK_SIZE] __attribute__((aligned(32)));
StaticTask_t gTPDOTaskObj;

#if (ACTIVE_PROTOCOL != IOCOUPLER_COMM_NONE)
StackType_t gCOMTaskStack[INDSCOM_TASK_SIZE] __attribute__((aligned(32)));
StaticTask_t gCOMTaskObj;
#endif

task_t task[] =
{
    {TASK_MAIN, 1, MAIN_TASK_SIZE,  gMainTaskStack,  &gMainTaskObj,  NULL, master_loop},
    {TASK_CAN_TIMER, configMAX_PRIORITIES-2, TIMER_TASK_SIZE, gTimerTaskStack, &gTimerTaskObj, NULL, TimerTaskLoop},
	{TASK_CAN_RX, configMAX_PRIORITIES-1, CAN_RX_TASK_SIZE,  gRXTaskStack,  &gRXTaskObj,  NULL, canRecv_Task},
    {TASK_CAN_RPDO, configMAX_PRIORITIES-1, RPDO_TASK_SIZE,  gRPDOTaskStack,  &gRPDOTaskObj,  NULL, RecvPDOLoop},
#if (ACTIVE_PROTOCOL != IOCOUPLER_ECAT)
    {TASK_CAN_TPDO, configMAX_PRIORITIES-2, TPDO_TASK_SIZE,  gTPDOTaskStack,  &gTPDOTaskObj,  NULL, SendPDOLoop}
#endif
#if (ACTIVE_PROTOCOL == IOCOUPLER_MODBUSTCP)
    ,{TASK_INDS_COM, 5, INDSCOM_TASK_SIZE, gCOMTaskStack, &gCOMTaskObj, NULL, appMain}
#endif
};

static uint8_t sys_cmd = SYS_NO_EVENT;
static bool rescan_io = false;

static void App_printCpuLoad();
static void ConfigureSlaveNode(CO_Data* d, IO_SlaveInfo* slaveInfo);
static void CheckSDOAndContinue(CO_Data* d, UNS8 nodeId);
static void InitNodes();
static void Reconfigure_slave(CO_Data* d, UNS32 nodeId);
static void Exit(CO_Data* d, UNS32 id);
static void master_start_tasks(void);
static void master_stop_tasks(void);
static void master_rescan(void);
static void stop_master(void);
static void print_master_info(void);
static void master_reset_conf_step(CO_Data* d);
static void master_reset_conf_step_slave(UNS32 nodeId);
static void master_set_IOerror(IO_SlaveInfo *slaveInfo, uint32_t error_type, uint32_t error_code);

/***************************  PDO Task  *****************************************/
int find_slave_index_by_nodeId(uint8_t nodeId)
{
    for (int i = 0; i < IOCoupler_Devices.numberOfSlaves; i++)
    {
        if (IOCoupler_Devices.slaveInfo[i].nodeId == nodeId)
            return i;
    }
    return -1;
}
/* ========================================================= */
/* Data synchronization                                      */
/* ========================================================= */
bool isMaster_running(void){
	return ( (getState(&Master_Data) == Operational) ? true : false );
}

uint8_t getNumIOSlave(void){
	return IOCoupler_Devices.numberOfSlaves;
}

#if (ACTIVE_PROTOCOL == IOCOUPLER_MODBUSTCP)
void sync_mb_coil(void)
{
    for (int i = 0; i < IOCoupler_Devices.numberOfSlaves; i++)
    {
        // Only DO modules
        if (IOCoupler_Devices.slaveInfo[i].productCode != IO_DEVICE_TYPE_DO16)
            continue;

        // Safety checks
        if (IOCoupler_Devices.slaveInfo[i].d_ptr == NULL)
            continue;

        if (IOCoupler_Devices.slaveInfo[i].output_index == 0)
            continue;
		
		int pdonum = IOCoupler_Devices.slaveInfo[i].output_index - 1;

		if (pdonum >= MAX_IO_DEVICES)
					continue;
				
        int bit_offset = (IOCoupler_Devices.slaveInfo[i].output_index - 1) * 16;

        uint16_t do_val = 0;

        for (int bit = 0; bit < 16; bit++)
        {
            int coil_index = bit_offset + bit;

            uint8_t coil_byte = mbs_app_storage.Coils[coil_index >> 3];
            uint8_t mask      = (1U << (coil_index & 0x07));

            if (coil_byte & mask)
            {
                do_val |= (1U << bit);
            }
        }

        memcpy(IOCoupler_Devices.slaveInfo[i].d_ptr, &do_val, sizeof(uint16_t));
		uint8_t res = sendOnePDOevent(&Master_Data, pdonum);
		// DebugP_log(
		// 			"PDO_LOOP DO res=%d nodeId=0x%02X pdonum=%d val=0x%04X\r\n",
		// 			res,
		// 			IOCoupler_Devices.slaveInfo[i].nodeId,
		// 			pdonum,
		// 			*(UNS16 *)IOCoupler_Devices.slaveInfo[i].d_ptr
		// 		);
    }
}

void sync_mb_di(void)
{
    for (int i = 0; i < IOCoupler_Devices.numberOfSlaves; i++)
    {
        // Only DI modules
        if (IOCoupler_Devices.slaveInfo[i].productCode != IO_DEVICE_TYPE_DI16)
            continue;

        // Safety checks
        if (IOCoupler_Devices.slaveInfo[i].d_ptr == NULL)
            continue;

        if (IOCoupler_Devices.slaveInfo[i].input_index == 0)
            continue;

        int bit_offset = (IOCoupler_Devices.slaveInfo[i].input_index - 1) * 16;

        uint16_t di_val = 0;

        // Read DI value
        memcpy(&di_val, IOCoupler_Devices.slaveInfo[i].d_ptr, sizeof(uint16_t));

        for (int bit = 0; bit < 16; bit++)
        {
            int coil_index = bit_offset + bit;

            uint8_t *coil_byte = &mbs_app_storage.DiscreteCoils[coil_index >> 3];
            uint8_t  mask      = (1U << (coil_index & 0x07));

            if (di_val & (1U << bit))
            {
                *coil_byte |= mask;
            }
            else
            {
                *coil_byte &= ~mask;
            }
        }
    }
}
#endif

void BuildProcessImage()
{
    uint16_t inOffset = 0;
    uint16_t outOffset = 0;

    for (int i = 0; i < IOCoupler_Devices.numberOfSlaves; i++)
    {
        IO_SlaveInfo *s = &IOCoupler_Devices.slaveInfo[i];

        switch (s->productCode)
        {
            case IO_DEVICE_TYPE_DI16:
                s->offset_index = inOffset;
                inOffset += 2;
                break;

            case IO_DEVICE_TYPE_DO16:
                s->offset_index = outOffset;
                outOffset += 2;
                break;

            case IO_DEVICE_TYPE_AIC8:
            case IO_DEVICE_TYPE_AIV8:
                s->offset_index = inOffset;
                inOffset += 16;
                break;

            case IO_DEVICE_TYPE_AOC8:
            case IO_DEVICE_TYPE_AOV8:
                s->offset_index = outOffset;
                outOffset += 16;
                break;
        }
    }
}

#if (ACTIVE_PROTOCOL == IOCOUPLER_ETHERNETIP)

void UpdateInputProcessImage(void)
{
    for (int i = 0; i < IOCoupler_Devices.numberOfSlaves; i++)
    {
        IO_SlaveInfo *s = &IOCoupler_Devices.slaveInfo[i];

        /* =========================
         * DIGITAL INPUT
         * ========================= */
        if (s->productCode == IO_DEVICE_TYPE_DI16)
        {
            if (s->d_ptr != NULL)
            {
                memcpy(&gInputProcessImage[s->offset_index],
                       s->d_ptr,
                       IO_DIGITAL_MODULE_BYTESIZE);
            }
        }

        /* =========================
         * ANALOG INPUT
         * ========================= */
        else if ((s->productCode == IO_DEVICE_TYPE_AIC8) ||
                 (s->productCode == IO_DEVICE_TYPE_AIV8))
        {
            for (int ch = 0; ch < IO_ANALOG_CHANNEL_NUM; ch++)
            {
                if (s->a_ptr[ch] != NULL)
                {
                    memcpy(&gInputProcessImage[s->offset_index + (ch * 2)],
                           s->a_ptr[ch],
                           IO_ANALOG_BYTES_PER_CHANNEL);
                }
            }
        }
    }
}

void UpdateOutputModules(void)
{	
    for (int i = 0; i < IOCoupler_Devices.numberOfSlaves; i++)
    {
        IO_SlaveInfo *s = &IOCoupler_Devices.slaveInfo[i];

        /* =========================
         * DIGITAL OUTPUT
         * ========================= */
        if (s->productCode == IO_DEVICE_TYPE_DO16)
        {
            if (s->d_ptr != NULL)
            {
                memcpy(s->d_ptr,
                       &gOutputProcessImage[s->offset_index],
                       IO_DIGITAL_MODULE_BYTESIZE);
            }
        }

        /* =========================
         * ANALOG OUTPUT
         * ========================= */
        else if ((s->productCode == IO_DEVICE_TYPE_AOC8) ||
                 (s->productCode == IO_DEVICE_TYPE_AOV8))
        {
            for (int ch = 0; ch < IO_ANALOG_CHANNEL_NUM; ch++)
            {
                if (s->a_ptr[ch] != NULL)
                {
                    memcpy(s->a_ptr[ch],
                           &gOutputProcessImage[s->offset_index + (ch * 2)],
                           IO_ANALOG_BYTES_PER_CHANNEL);
                }
            }
        }
    }
}
#endif

#if (ACTIVE_PROTOCOL == IOCOUPLER_ECAT)
// static TaskHandle_t task_inds_comm_ecat = NULL;
// void set_task_handle(TaskHandle_t *taskh){
// 	task_inds_comm_ecat = taskh;
// }

// void start_inds_comm_ecat(){
// 	vTaskResume(task_inds_comm_ecat);
// }
// void stop_inds_comm_ecat(){
// 	vTaskSuspend(task_inds_comm_ecat);
// }
#endif
/* ========================================================= */
void RecvPDOLoop(void *args)
{
    (void)args;

    Message m;
    CO_Data* d = &Master_Data;

    while (1)
    {
        if (xQueueReceive(xMcanRxQueue, &m, portMAX_DELAY) == pdPASS)
        {
            canDispatch(d, &m);
        }
    }
}

void SendPDOLoop(void *args)
{
	(void)args;

	vTaskSuspend(NULL);

#if (ACTIVE_PROTOCOL == IOCOUPLER_MODBUSTCP)
	while(1){
		sync_mb_coil();
		sync_mb_di();
		vTaskDelay(1);
	}
#endif
    CO_Data* d = &Master_Data;

    UNS16 last_do[MAX_IO_DEVICES];
    UNS16 curr_do[MAX_IO_DEVICES];
	UNS8 pdonum = 0;
	
	for(int i = 0; i < MAX_IO_DEVICES; i++){
		last_do[i] = 0xFFFF; /* invalid initial value */
	}

	while (1)
	{
		for(int i = 0; i < IOCoupler_Devices.numberOfSlaves; i++){
			/* only DO16 devices */
			if (IOCoupler_Devices.slaveInfo[i].productCode != IO_DEVICE_TYPE_DO16)
				continue;

			/* validate d_ptr */
			if (!IOCoupler_Devices.slaveInfo[i].d_ptr)
				continue;

			/* validate output index (must be 1-based) */
			if (IOCoupler_Devices.slaveInfo[i].output_index == 0)
				continue;

			pdonum = IOCoupler_Devices.slaveInfo[i].output_index - 1;

			/* bounds check */
			if (pdonum >= MAX_IO_DEVICES)
				continue;		

#if 0 //TEST
			/* TEST: force DO toggle */
			// if (IOCoupler_Devices.slaveInfo[i].d_ptr)
			// {
			// 	curr_do[pdonum] =
			// 		(*(UNS16 *)IOCoupler_Devices.slaveInfo[i].d_ptr) ^ 0xFFFF;

			// 	*(UNS16 *)IOCoupler_Devices.slaveInfo[i].d_ptr = curr_do[pdonum];
			// }
			if (IOCoupler_Devices.slaveInfo[i].d_ptr)
			{
				UNS16 *curr_ptr = (UNS16 *)IOCoupler_Devices.slaveInfo[i].d_ptr;
				uint8_t nodeId = IOCoupler_Devices.slaveInfo[i].nodeId;

				// DO nodes follow pattern: (nodeId % 4 == 1 or 2)
				if ((nodeId % 4 == 1) || (nodeId % 4 == 2))
				{
					//--------------------------------------------------
					// DO-1 → toggle
					//--------------------------------------------------
					if (nodeId == 1)
					{
						curr_do[pdonum] = (*curr_ptr) ^ 0xFFFF;
					}

					//--------------------------------------------------
					// DO-x (x % 4 == 2) → from DI (x+1)
					//--------------------------------------------------
					else if (nodeId % 4 == 2)
					{
						uint8_t di_node = nodeId + 1;

						int di_idx = find_slave_index_by_nodeId(di_node);
						if (di_idx < 0 || !IOCoupler_Devices.slaveInfo[di_idx].d_ptr)
							continue;

						UNS16 *di_ptr = (UNS16 *)IOCoupler_Devices.slaveInfo[di_idx].d_ptr;
						curr_do[pdonum] = *di_ptr;
					}

					//--------------------------------------------------
					// DO-x (x % 4 == 1, >1) → from previous DO (x-1)
					//--------------------------------------------------
					else if ((nodeId % 4 == 1) && (nodeId > 1))
					{
						uint8_t src_node = nodeId - 1;

						int src_idx = find_slave_index_by_nodeId(src_node);
						if (src_idx < 0 || !IOCoupler_Devices.slaveInfo[src_idx].d_ptr)
							continue;

						UNS16 *src_ptr = (UNS16 *)IOCoupler_Devices.slaveInfo[src_idx].d_ptr;
						curr_do[pdonum] = *src_ptr;
					}

					//--------------------------------------------------
					// Apply result
					//--------------------------------------------------
					*curr_ptr = curr_do[pdonum];
				}
			}
			else
#endif
			{
				curr_do[pdonum] =
					*(UNS16 *)IOCoupler_Devices.slaveInfo[i].d_ptr;
			}

			/* send only on change */
			if (curr_do[pdonum] != last_do[pdonum])
			{
				sendOnePDOevent(d, pdonum);
				last_do[pdonum] = curr_do[pdonum];

				// DebugP_log(
				// 	"PDO_LOOP DO nodeId=0x%02X pdonum=%d curr=0x%04X raw=0x%04X\r\n",
				// 	IOCoupler_Devices.slaveInfo[i].nodeId,
				// 	pdonum,
				// 	curr_do[pdonum],
				// 	*(UNS16 *)IOCoupler_Devices.slaveInfo[i].d_ptr
				// );
			}
		}
		vTaskDelay(1);
	}
}

/************************  Master  Callback Function ****************************/
void Master_heartbeatError(CO_Data* d, UNS8 heartbeatID)
{
	DebugP_log("Master_heartbeatError %d\r\n", heartbeatID);
	// if(heartbeatID == 2){
	// 	DebugP_log("consumer hb node 2 : %u ms\r\n", ClockP_getTimeUsec()/1000);
	// }
	for(int i = 0; i < IOCoupler_Devices.numberOfSlaves; i++){
		if(IOCoupler_Devices.slaveInfo[i].nodeId == heartbeatID){
			IOCoupler_Devices.slaveInfo[i].nodeState = getNodeState(d, heartbeatID);
			master_set_IOerror(&IOCoupler_Devices.slaveInfo[i], CANOPEN_IO_HEARTBEAT, HEARTBEAT_ERR_TIMEOUT);
			break;
		}
	}
}

void Master_initialisation(CO_Data* d)
{
	// DebugP_log("Master_initialisation\r\n");
	IOCoupler_reset(d);
}

void Master_preOperational(CO_Data* d)
{
	DebugP_log("[MASTER] Master_preOperational\r\n");
	
	// Reset all nodes
	vTaskDelay(2000);
	masterSendNMTstateChange (d, 0x00, NMT_Reset_Node);
	vTaskDelay(15000); // wait for reset to be processed
	// masterSendNMTstateChange(d, 0x00, NMT_Enter_PreOperational);
	// vTaskDelay(1000);

	// Scan the network for slaves
	while(get_scan_state() != SCAN_DONE){
		IOCoupler_scan_task(d);
		vTaskDelay(1);
	}
	
	//Configure PDO Master_
	ODMaster_configure_PDO(d);

	// Configure each slave
	for(int i=0; i<IOCoupler_Devices.numberOfSlaves; i++){
		ConfigureSlaveNode(d, &IOCoupler_Devices.slaveInfo[i]);
		vTaskDelay(20);
	}

	// Put all slaves in operational mode
	// for(int i=0; i<IOCoupler_Devices.numberOfSlaves; i++){
	// 	masterSendNMTstateChange (d, IOCoupler_Devices.slaveInfo[i].nodeId, NMT_Start_Node);
	// 	vTaskDelay(5);
	// }
	masterSendNMTstateChange (d, 0x00, NMT_Start_Node);
	
#if ((ACTIVE_PROTOCOL == IOCOUPLER_ETHERNETIP) || (ACTIVE_PROTOCOL == IOCOUPLER_ECAT))
    BuildProcessImage();
#endif

	vTaskDelay(100);
	
	master_start_tasks();
	//Put the master in operational mode
	setState(d, Operational);
}

void Master_operational(CO_Data* d)
{
	DebugP_log("[Master] Master_operational\r\n");
}

void Master_stopped(CO_Data* d)
{
	DebugP_log("[Master] Master_stopped\r\n");
}

void Master_post_sync(CO_Data* d)
{
	DebugP_log("-----------------------------------\r\n");
	DebugP_log("[Master] Master_post_sync\r\n");

	for (int i = 0; i < IOCoupler_Devices.numberOfSlaves; i++)
    {
		if (IOCoupler_Devices.slaveInfo[i].productCode == IO_DEVICE_TYPE_DI16)
		{
			if (IOCoupler_Devices.slaveInfo[i].d_ptr)
			{
				DebugP_log(
					"DI nodeId 0x%02X Value = 0x%04X\n",
					IOCoupler_Devices.slaveInfo[i].nodeId,
					*(UNS16 *)IOCoupler_Devices.slaveInfo[i].d_ptr
				);
			}
		}
	}

	DebugP_log("-----------------------------------\r\n");
}

void Master_post_TPDO(CO_Data* d)
{
	if(OUTPUT_TRANSMISSION_TYPE == 0x01){
		DebugP_log("-----------------------------------\r\n");
		DebugP_log("[Master] Master_post_TPDO\r\n");

		for (int i = 0; i < IOCoupler_Devices.numberOfSlaves; i++)
		{
			if (IOCoupler_Devices.slaveInfo[i].productCode == IO_DEVICE_TYPE_DO16)
			{
				/* Force DO value for slave index 0x13 */
				if (IOCoupler_Devices.slaveInfo[i].nodeId == 0x13)
				{
					UNS16 *do_val = (UNS16 *)IOCoupler_Devices.slaveInfo[i].d_ptr;
					*do_val ^= 0xFFFF;
				}

				if (IOCoupler_Devices.slaveInfo[i].d_ptr)
				{
					DebugP_log(
						"post_TPDO DO nodeId 0x%02X Value = 0x%04X\r\n",
						IOCoupler_Devices.slaveInfo[i].nodeId,
						*(UNS16 *)IOCoupler_Devices.slaveInfo[i].d_ptr
					);
				}
			}
		}
		
		DebugP_log("-----------------------------------\r\n");
	}
}

void Master_post_emcy(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg)
{
    for(int i = 0; i < IOCoupler_Devices.numberOfSlaves; i++) {
        if(IOCoupler_Devices.slaveInfo[i].nodeId == nodeID) {
			uint32_t last_error_type = IOCoupler_Devices.slaveInfo[i].last_error_type;
			uint32_t last_error_code = IOCoupler_Devices.slaveInfo[i].last_error_code;
            
            if((last_error_type == CANOPEN_IO_EMCY) && (errCode == 0x0000)) {
				DebugP_log("[Master] EMCY received. Node: 0x%02X  ErrorCode: 0x%04X  ErrorRegister: 0x%02X\r\n", 
            		nodeID, errCode, errReg);
                master_set_IOerror(&IOCoupler_Devices.slaveInfo[i], CANOPEN_IO_NO_ERR, 0);
            } else if (errCode != 0x0000) {
				DebugP_log("[Master] EMCY received. Node: 0x%02X  ErrorCode: 0x%04X  ErrorRegister: 0x%02X\r\n", 
            		nodeID, errCode, errReg);
                uint32_t io_errcode = ((uint32_t)errReg << 16) | (errCode & 0xFFFF);
                
                DebugP_log("io_errcode = 0x%08X\r\n", io_errcode);
                master_set_IOerror(&IOCoupler_Devices.slaveInfo[i], CANOPEN_IO_EMCY, io_errcode);
            }
            break;
        }
    }
}

void Master_post_SlaveBootup(CO_Data* d, UNS8 nodeid)
{
	// DebugP_log("[Master] Master_post_SlaveBootup 0x%x\r\n", nodeid);
	for(int i = 0; i < IOCoupler_Devices.numberOfSlaves; i++){
		if(IOCoupler_Devices.slaveInfo[i].nodeId == nodeid){
			e_nodeState nodeState = (e_nodeState)IOCoupler_Devices.slaveInfo[i].nodeState;
			
			if(nodeState == Disconnected){
				uint32_t last_error_type = IOCoupler_Devices.slaveInfo[i].last_error_type;
				uint32_t last_error_code = IOCoupler_Devices.slaveInfo[i].last_error_code;
				if( (last_error_type == CANOPEN_IO_HEARTBEAT) 
					&& (last_error_code == HEARTBEAT_ERR_TIMEOUT) ){
					master_set_IOerror(&IOCoupler_Devices.slaveInfo[i], CANOPEN_IO_REBOOT, 1);
					Reconfigure_slave(d, nodeid);
				}
			}else{
				IOCoupler_Devices.slaveInfo[i].nodeState = Initialisation;
			}
			
			break;
		}
	}
}

void Master_post_SlaveStateChange(CO_Data* d, UNS8 nodeid, e_nodeState newNodeState)
{
	for(int i = 0; i < IOCoupler_Devices.numberOfSlaves; i++){
		if(IOCoupler_Devices.slaveInfo[i].nodeId == nodeid){
			e_nodeState nodeState = (e_nodeState)IOCoupler_Devices.slaveInfo[i].nodeState;
			uint32_t last_error_type = IOCoupler_Devices.slaveInfo[i].last_error_type;
			uint32_t last_error_code = IOCoupler_Devices.slaveInfo[i].last_error_code;
			// DebugP_log("[Master] Master_post_SlaveStateChange id 0x%x | currstate 0x%x |newstate 0x%x\r\n", nodeid, nodeState, newNodeState);
			
			if(nodeState == Disconnected){
				if(newNodeState == Operational){
					master_set_IOerror(&IOCoupler_Devices.slaveInfo[i], CANOPEN_IO_NO_ERR, 0);
				}
				else {
					if( (last_error_type == CANOPEN_IO_HEARTBEAT) 
						&& (last_error_code == HEARTBEAT_ERR_TIMEOUT) ){
						master_set_IOerror(&IOCoupler_Devices.slaveInfo[i], HEARTBEAT_ERR_RECOVERED, HEARTBEAT_ERR_RECOVERED);
						Reconfigure_slave(d, nodeid);
					}
				}
			}else if ( (nodeState == Operational) && (newNodeState != nodeState) ){
				DebugP_log("[Master] Slave 0x%x changed from OPERATIONAL to WRONG MODE\r\n", nodeid);
				master_set_IOerror(&IOCoupler_Devices.slaveInfo[i], CANOPEN_IO_WRONG_MODE_OP, newNodeState);
			}else {
				master_set_IOerror(&IOCoupler_Devices.slaveInfo[i], CANOPEN_IO_NO_ERR, 0);
			}

			IOCoupler_Devices.slaveInfo[i].nodeState = newNodeState;
			break;
		}
	}
}

/****************************************************************************/
static void App_printCpuLoad()
{
	const uint32_t cpuLoad = TaskP_loadGetTotalCpuLoad();
    TaskP_loadResetAll();
    DebugP_log("[MASTER] CPU load = %d %% \r\n", cpuLoad/100);
    return;
}

static void InitNodes()
{
	DebugP_log("[Master] InitNodes\r\n");
	/* Defining the node Id */
	setNodeId(&Master_Data, MASTER_NODEID);

	/* init */
	Master_Data.nodeState = Unknown_state;
	setState(&Master_Data, Initialisation);
}

static void Reconfigure_slave(CO_Data* d, UNS32 nodeId)
{
	DebugP_log("[MASTER] Reconfigure After received Slave's hearbeat recoverd \r\n");

	masterSendNMTstateChange(d, nodeId, NMT_Enter_PreOperational);
	vTaskDelay(250);
	
	for(int i=0; i<IOCoupler_Devices.numberOfSlaves; i++){
		if(IOCoupler_Devices.slaveInfo[i].nodeId == nodeId){
			master_reset_conf_step_slave(nodeId);
			ConfigureSlaveNode(d, &IOCoupler_Devices.slaveInfo[i]);
			vTaskDelay(20);

			masterSendNMTstateChange (d, nodeId, NMT_Start_Node);
			vTaskDelay(5);
		}
	}
}

static void Exit(CO_Data* d, UNS32 id)
{
	masterSendNMTstateChange(&Master_Data, 0x00, NMT_Reset_Node);

    //Stop master
	setState(&Master_Data, Stopped);
}

static void CheckSDOAndContinue(CO_Data* d, UNS8 nodeId)
{
	UNS32 abortCode;

	if(getWriteResultNetworkDict (d, nodeId, &abortCode) != SDO_FINISHED)
		DebugP_log("Master_ : Failed in initializing slave %2.2x, step %d, AbortCode :%4.4x\r\n", nodeId, init_step, abortCode);

	/* Finalise last SDO transfer with this node */
	closeSDOtransfer(&Master_Data, nodeId, SDO_CLIENT);

	for(int i=0; i<IOCoupler_Devices.numberOfSlaves; i++){
		if(IOCoupler_Devices.slaveInfo[i].nodeId == nodeId){
			/* Continue the configuration */
			ConfigureSlaveNode(d, &IOCoupler_Devices.slaveInfo[i]);
		}
	}
	
}

static void ConfigureSlaveNode(CO_Data* d, IO_SlaveInfo* slaveInfo)
{
	UNS8 res = 0;
	UNS32 PDO_COBId = 0;
	UNS32 PDO_index = 0;
	UNS32 PDO_subindex = 0;

	/*setup Slave's TPDO 1 to be transmitted on SYNC*/
	UNS8 Transmission_Type = 0x01;

	UNS8 nodeId = slaveInfo->nodeId;
	uint32_t productCode = slaveInfo->productCode;

	// DebugP_log("Master_ : ConfigureSlaveNode %2.2x\r\n", nodeId);
	// DebugP_log("init_step=%d\r\n",init_step[nodeId-1]);
	switch(++init_step[nodeId-1]){
		case 1: 
		{	
			/* Disable Slave's PDO */
			/*COB-ID, 32-bit
			  	Bit 31 : PDO valid (0 = enabled; 1 = disabled)
				Bit 30 : RTR allowed (0 = RTR allowed; 1 = RTR NOT allowed)
				Bit 29 : Frame type (0 = Standard (11-bit); 1 = Extended (29-bit)
				Bits 0–28 : CAN Identifier
			*/
			if(productCode == IO_DEVICE_TYPE_DO16 || productCode == IO_DEVICE_TYPE_AOC8 || productCode == IO_DEVICE_TYPE_AOV8){
				PDO_COBId = 0x80000200 + nodeId;
				PDO_index = 0x1400; // RPDO1
				PDO_subindex = 0x01;
			}else if (productCode == IO_DEVICE_TYPE_DI16 || productCode == IO_DEVICE_TYPE_AIC8 || productCode == IO_DEVICE_TYPE_AIV8)
			{
				PDO_COBId = 0x80000180 + nodeId;
				PDO_index = 0x1800; //TPDO1
				PDO_subindex = 0x01;
			}else{
				init_step[nodeId-1] = 99; // skip rest of steps
				break;
			}

			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
				nodeId, /*UNS8 nodeId*/
				PDO_index, /*UNS16 index*/
				PDO_subindex, /*UNS8 subindex*/
				4, /*UNS8 count*/
				0, /*UNS8 dataType*/
				&PDO_COBId,/*void *data*/
				CheckSDOAndContinue, /*SDOCallback_t Callback*/
				0); /* use block mode */
        }			
		break;

		case 2: 
		{	
			/* Change transmission type */
			PDO_subindex = 0x02; // Transmission Type

			if(productCode == IO_DEVICE_TYPE_DO16 || productCode == IO_DEVICE_TYPE_AOC8 || productCode == IO_DEVICE_TYPE_AOV8){
				PDO_index = 0x1400; // RPDOx
				Transmission_Type = INPUT_TRANSMISSION_TYPE;

				res = writeNetworkDictCallBack (d, /*CO_Data* d*/
				nodeId, /*UNS8 nodeId*/
				PDO_index, /*UNS16 index*/
				PDO_subindex, /*UNS8 subindex*/
				1, /*UNS8 count*/
				0, /*UNS8 dataType*/
				&Transmission_Type,/*void *data*/
				CheckSDOAndContinue, /*SDOCallback_t Callback*/
				0); /* use block mode */
			}else if (productCode == IO_DEVICE_TYPE_DI16 || productCode == IO_DEVICE_TYPE_AIC8 || productCode == IO_DEVICE_TYPE_AIV8)
			{
				PDO_index = 0x1800; //TPDOx
				Transmission_Type = OUTPUT_TRANSMISSION_TYPE;

				res = writeNetworkDictCallBack (d, /*CO_Data* d*/
				nodeId, /*UNS8 nodeId*/
				PDO_index, /*UNS16 index*/
				PDO_subindex, /*UNS8 subindex*/
				1, /*UNS8 count*/
				0, /*UNS8 dataType*/
				&Transmission_Type,/*void *data*/
				CheckSDOAndContinue, /*SDOCallback_t Callback*/
				0); /* use block mode */
			}
		}			
		break;

		case 3: 
		{	
			/* Re-enable Slave's PDO */
			if(productCode == IO_DEVICE_TYPE_DO16 || productCode == IO_DEVICE_TYPE_AOC8 || productCode == IO_DEVICE_TYPE_AOV8){
				PDO_COBId = 0x00000200 + nodeId;
				PDO_index = 0x1400; // RPDO1
				PDO_subindex = 0x01;
			}else if (productCode == IO_DEVICE_TYPE_DI16 || productCode == IO_DEVICE_TYPE_AIC8 || productCode == IO_DEVICE_TYPE_AIV8)
			{
				PDO_COBId = 0x00000180 + nodeId;
				PDO_index = 0x1800; //TPDO1
				PDO_subindex = 0x01;
			}

			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
				nodeId, /*UNS8 nodeId*/
				PDO_index, /*UNS16 index*/
				PDO_subindex, /*UNS8 subindex*/
				4, /*UNS8 count*/
				0, /*UNS8 dataType*/
				&PDO_COBId,/*void *data*/
				CheckSDOAndContinue, /*SDOCallback_t Callback*/
				0); /* use block mode */
		}			
		break;

		case 4:	
		{
			UNS32 _Slave_Prod_Heartbeat_T=GET_SLAVE_PROD_HEARTBEAT;
			
			res = writeNetworkDictCallBack (d, /*CO_Data* d*/
				nodeId, /*UNS8 nodeId*/
				0x1017, /*UNS16 index*/ //Producer Heartbeat Time
				0x00, /*UNS8 subindex*/
				2, /*UNS8 count*/
				0, /*UNS8 dataType*/
				&_Slave_Prod_Heartbeat_T,/*void *data*/
				CheckSDOAndContinue, /*SDOCallback_t Callback*/
				0); /*UNS8 useBlockMode*/
		}			
		break; 

		default: break;
	}			
}

static void master_reset_conf_step(CO_Data* d){
	// DebugP_log("[Master] master_reset_conf_step\r\n");
	for(int i = 0; i < NMT_MAX_NODE_ID; i++){
		init_step[i] = 0;
		d->NMTable[i] = Unknown_state;
	}
}

static void master_reset_conf_step_slave(UNS32 nodeId){
	// DebugP_log("[Master] master_reset_conf_step\r\n");
	init_step[nodeId-1] = 0;
}

static void master_start_tasks(void){
#if (ACTIVE_PROTOCOL == IOCOUPLER_MODBUSTCP)
	if(task[TASK_INDS_COM].TaskHandle){
		vTaskResume(task[TASK_INDS_COM].TaskHandle);
	}
#endif

	if(task[TASK_CAN_TPDO].TaskHandle){
		vTaskResume(task[TASK_CAN_TPDO].TaskHandle);
	}
}

static void master_stop_tasks(void){
#if (ACTIVE_PROTOCOL == IOCOUPLER_MODBUSTCP)
	if(task[TASK_INDS_COM].TaskHandle){
		vTaskSuspend(task[TASK_INDS_COM].TaskHandle);
	}
#endif

	if(task[TASK_CAN_TPDO].TaskHandle){
		vTaskSuspend(task[TASK_CAN_TPDO].TaskHandle);
	}
}

static void master_rescan(void){
	master_stop_tasks();
    IOCoupler_reset(&Master_Data);
	master_reset_conf_step(&Master_Data);
    setState(&Master_Data, Pre_operational);
    DebugP_log("[Master] Software Reset to Pre-Operational Complete\r\n");
}

static void stop_master(void) {
    setState(&Master_Data, Stopped);
	master_stop_tasks();
}

static void print_master_info(void){
	DebugP_log("-------------------------------------\r\n");
	DebugP_log("Master FW Version %s\r\n", MASTER_FW_VERSION);
	DebugP_log("Max IO Devices %d\r\n", MAX_IO_DEVICES);
	DebugP_log("Active Protocol %d\r\n", ACTIVE_PROTOCOL);
	DebugP_log("-------------------------------------\r\n");
}

static void master_set_IOerror(IO_SlaveInfo *slaveInfo, uint32_t error_type, uint32_t error_code) {
    
    if (error_type == CANOPEN_IO_NO_ERR) {
        slaveInfo->last_error_type = CANOPEN_IO_NO_ERR;
        slaveInfo->last_error_code = 0;
        return;
    }

    // ignore lesser errors (like heartbeats).
    if (slaveInfo->last_error_type == CANOPEN_IO_EMCY) {
        return; 
    }

    slaveInfo->last_error_type = error_type;
    slaveInfo->last_error_code = error_code;
}

/****************************************************************************/
/***************************  MAIN  *****************************************/
/****************************************************************************/
void master_loop(void *args)
{
	(void)args;
	// int count_data = 0;
	print_master_info();
	DebugP_log("[MASTER] master_loop started...\r\n");
	
	InitNodes();
	
	// Start timer thread
	StartTimerLoop(NULL);
	
#if 0 //TEST RPDO and TPDO
	SendPDOLoop(NULL);
#endif
	while(1){		
		App_printCpuLoad();
		
		if(sys_cmd == SYS_CMD_MASTER_RESCAN){
			DebugP_log("[MASTER] Software Restart Master...\r\n");
			master_rescan();
			sys_cmd = SYS_NO_EVENT;
		}

		for(int i = 0; i < 50; i++) {
			vTaskDelay(pdMS_TO_TICKS(100));
		}

#if 0 //TEST
		Message m;
		m.cob_id = 0x10;
		m.len = 4;
		for(int i =0; i< m.len; i++){
			m.data[i] = count_data%255;
		}
		canSend(NULL, &m);
		count_data++;
#endif
	}	
	// for(int i=1; i<IOCoupler_Devices.numberOfSlaves; i++){
	// 	// Reset the slave node for next use (will stop emitting heartbeat)
	// 	masterSendNMTstateChange (&Master_Data, IOCoupler_Devices.slaveInfo[i-1].nodeId, NMT_Reset_Node);	
	// }
}

int master_main(void)
{
	canInit();
	
	Master_Data.heartbeatError = Master_heartbeatError;
	Master_Data.initialisation = Master_initialisation;
	Master_Data.preOperational = Master_preOperational;
	Master_Data.operational = Master_operational;
	Master_Data.stopped = Master_stopped;
	Master_Data.post_sync = Master_post_sync;
	Master_Data.post_TPDO = Master_post_TPDO;
	Master_Data.post_emcy = Master_post_emcy;
	Master_Data.post_SlaveBootup=Master_post_SlaveBootup;
	Master_Data.post_SlaveStateChange=Master_post_SlaveStateChange;
		
	for (int i = TASK_MAIN; i < TASK_END; i++)
	{
		if(task[i].TaskId != (uint8_t)i){
			continue;
		}
		if (task[i].callback != NULL)
		{
			task[i].TaskHandle = xTaskCreateStatic(
									task[i].callback,
									"task",
									task[i].TaskSize,
									&Master_Data,
									task[i].TaskPrio,
									task[i].TaskStack,
									task[i].TaskObj
								);

			configASSERT(task[i].TaskStack != NULL);
			configASSERT(task[i].TaskObj != NULL);
			configASSERT(task[i].TaskHandle != NULL);

			if(task[i].TaskId == (uint8_t)TASK_CAN_TIMER){
				timerInit(task[TASK_CAN_TIMER].TaskHandle);
			}
		}
	}

    /* Start the scheduler */
    vTaskStartScheduler();
    DebugP_assertNoLog(0);

    return 0;
}

/******************************** API **************************************/
void trigger_master_rescan(void) {
	rescan_io = true;
    sys_cmd = SYS_CMD_MASTER_RESCAN;
}

void get_master_firmware_ver(char *buff1, char *buff2, size_t dest_size) {
    if (buff1 == NULL || buff2 == NULL) return;
	if (dest_size == 0) return;

    // Use the actual destination buffer size, not the size of the constant
	snprintf(buff1, dest_size, "%s", MASTER_TYPE);
    snprintf(buff2, dest_size, "%s", MASTER_FW_VERSION);
}

bool get_master_rescan_status(void){
	return rescan_io;
}