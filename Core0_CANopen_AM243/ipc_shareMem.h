#ifndef IPC_SHARE_MEM_H
#define IPC_SHARE_MEM_H

/******************************************************************/
// Protocol
#define IOCOUPLER_ETHERNETIP    (1)
#define IOCOUPLER_ECAT          (2)
#define IOCOUPLER_MODBUSTCP     (3)
#define IOCOUPLER_COMM_NONE     (4)

/******************************************************************/
// IOCoupler CANOpen Master
#define MASTER_NODEID           (0x7F)
#define MAX_IO_DEVICES          (20)
/******************************************************************/
// Device Information
#define IO_DIGITAL_CHANNEL_NUM          (16)
#define IO_ANALOG_CHANNEL_NUM           (8)

#define IO_ANALOG_BYTES_PER_CHANNEL      2

#define IO_DIGITAL_MODULE_BYTESIZE  (2)
#define IO_ANALOG_MODULE_BYTESIZE   (IO_ANALOG_CHANNEL_NUM*IO_ANALOG_BYTES_PER_CHANNEL)

#define MAX_INPUT_MODULES       (MAX_IO_DEVICES)
#define MAX_OUTPUT_MODULES      (MAX_IO_DEVICES)

#define MAX_INPUT_SIZE_BYTES    (IO_DIGITAL_MODULE_BYTESIZE*MAX_INPUT_MODULES)
#define MAX_OUTPUT_SIZE_BYTES   MAX_INPUT_SIZE_BYTES

#define GESPANT_VENDOR_ID   (1524)
#define GESPANT_HW_VER_SIZE (5)   //bytes char + null terminator
#define GESPANT_FW_VER_SIZE (7)   //bytes

#define NUM_SUB_INDEX_DATA  8U
/******************************************************************/
#define DI_GET(v, ch)   (((v).d_all >> ((ch)-1)) & 0x01)
#define DI_SET(v, ch)   ((v).d_all |=  (1U << ((ch)-1)))
#define DI_CLR(v, ch)   ((v).d_all &= ~(1U << ((ch)-1)))

enum io_device_type {
    IO_DEVICE_TYPE_DO16 = 0x01,
    IO_DEVICE_TYPE_DI16 = 0x02,
    IO_DEVICE_TYPE_AIC8 = 0x03,
    IO_DEVICE_TYPE_AIV8 = 0x04,
    IO_DEVICE_TYPE_AOC8 = 0x05,
    IO_DEVICE_TYPE_AOV8 = 0x06,
    IO_DEVICE_TYPE_RTDY = 0x07,
    IO_DEVICE_TYPE_RTDB = 0x08,
};

typedef enum {
    SCAN_IDLE = 0,
    SCAN_READ_VENDOR,
    SCAN_READ_PRODUCT,
    SCAN_READ_REVISION,
    SCAN_READ_SERIAL,
    SCAN_READ_HW_VER,
    SCAN_READ_FW_VER,
    SCAN_WAIT,
    SCAN_DONE
} ScanState_t;

typedef struct {
    uint16_t a_ch1;
    uint16_t a_ch2;
    uint16_t a_ch3;
    uint16_t a_ch4;
    uint16_t a_ch5;
    uint16_t a_ch6;
    uint16_t a_ch7;
    uint16_t a_ch8;
} IO_AnalogValues;

typedef struct {
    uint16_t d_all;
} IO_DigitalValues;

/* * NOTE: Keep this structure local to Core 0 code execution.
 * Do not transmit raw void pointers over shared IPC memory zones.
 */
typedef struct
{
    void *d_ptr; //pointer to digital value
    void *a_ptr[IO_ANALOG_CHANNEL_NUM]; //pointer to analog value

} IO_RuntimeInfo;

typedef struct {
    union {
        struct {
            uint8_t nodeId;
            uint8_t productCode;
            uint8_t input_index;    // index start from 1
            uint8_t output_index;   // index start from 1

            uint32_t nodeState;
            uint32_t vendorId;

            uint32_t revisionNumber;
            uint32_t serialNumber;

            uint32_t last_error_type;
            uint32_t last_error_code;
            
            uint32_t offset_index;
            
            char hw_version[GESPANT_HW_VER_SIZE]; // 5 bytes
            char fw_version[GESPANT_FW_VER_SIZE]; // 7 bytes
        };

        uint8_t automated_slave_block[128];
    };
} __attribute__((aligned(128))) IO_SlaveInfo;

typedef struct {
    union {
        struct {
            uint8_t numberOfSlaves;
            uint8_t numberOfInputSlaves;
            uint8_t numberOfOutputSlaves;
            uint8_t numberofDI;
            uint8_t numberofDO;
            uint8_t numberofAIC;
            uint8_t numberofAIV;
            uint8_t numberofAOC;
            uint8_t numberofAOV;
            uint8_t numberofRTDY;
            uint8_t numberofRTDB;
            
            // Explicit padding to keep the upcoming 128-byte aligned array clean
            uint8_t explicit_alignment_pad[5]; 

            /* * Array of 128-byte elements.
             * 20 devices * 128 bytes = 2560 bytes.
             */
            IO_SlaveInfo slaveInfo[MAX_IO_DEVICES]; 
        };

        /* * Automated Padding Block.
         * Base variables (11) + Pad (5) + Array (2560) = 2576 bytes.
         * Nearest 128-byte boundary is 2688 bytes (128 * 21).
         */
        uint8_t automated_cache_block[2688];
    };
} __attribute__((aligned(128))) IOCoupler_Device;

/** IPC Data Structure */
 typedef enum {
    ERR_TASK_NONE,
    ERR_TASK_CAN_TX,
    ERR_TASK_CAN_RX,
    ERR_TASK_INDS_COMM,
} task_err_type_t;

typedef enum {
    WS_NONE,
    WS_SCAN_IO,
    WS_CFG_NET_IP,
    WS_CFG_DEVICE_NAME
} ws_cmd_t;

typedef struct {
    uint32_t heartbeat;
    uint32_t cpu_load;           // CPU Load percentage 
    uint32_t cycle_count_max;    
    uint32_t cycle_count_curr;   
    uint32_t task_switches;      
    task_err_type_t last_error_type;
    uint32_t last_error_code;    
} core_stats_t; // Size: 28 bytes

typedef struct {
    uint8_t active_protocol;
    uint8_t master_state;
    uint8_t ws_scan_status;
    uint8_t explicit_pad0;       // Aligns enum execution scope
    
    ws_cmd_t ws_cmd;             // enum is 4 bytes
    
    core_stats_t core0_stats;
    core_stats_t core1_stats;
    
    uint8_t reserved[64];        
} __attribute__((aligned(128))) ipc_system_t;

typedef struct {
    union {
        struct {
            ipc_system_t     ipc_sys;           //  128 Bytes
            uint8_t          buff_in[1024];     // 1024 Bytes
            uint8_t          buff_out[1024];    // 1024 Bytes
            IOCoupler_Device IOCoupler_Devices; // 2688 Bytes
        };

        // This reserves a fixed 8KB block in MSRAM
        uint8_t automated_shared_ram_block[8192]; 
    };
} __attribute__((aligned(128))) ipc_data_t;

void init_ipc_sharemem(void);
void app_ipc_sharemem_lock(void);
void app_ipc_sharemem_unlock(void);

void app_cache_read_sharemem(void);
void app_cache_write_sharemem(void);

void Master_print_io_info(void);

#endif /* IPC_SHARE_MEM_H */