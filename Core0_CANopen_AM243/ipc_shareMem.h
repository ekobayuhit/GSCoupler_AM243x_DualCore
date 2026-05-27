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

typedef struct
{
    void *d_ptr; //pointer to digital value
    void *a_ptr[IO_ANALOG_CHANNEL_NUM]; //pointer to analog value

} IO_RuntimeInfo;

typedef struct {
    uint8_t nodeId;
    uint8_t productCode;
    uint8_t input_index;    //index start from 1
    uint8_t output_index;   //index start from 1

    uint32_t nodeState;
    uint32_t vendorId;

    uint32_t revisionNumber;
    uint32_t serialNumber;

    uint32_t last_error_type;
    uint32_t last_error_code;
    
    uint32_t offset_index;
    
    char hw_version[GESPANT_HW_VER_SIZE];
    char fw_version[GESPANT_FW_VER_SIZE];
} IO_SlaveInfo;

typedef struct {
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
    IO_SlaveInfo slaveInfo[MAX_IO_DEVICES];
    uint8_t reserved[128];
} IOCoupler_Device;

typedef struct{
    uint8_t active_protocol;
    uint8_t master_state;
    uint8_t ws_cmd;
    uint8_t reserved[256];
}ipc_system_t;

typedef struct {
    ipc_system_t ipc_sys;
    uint8_t reserved_a[512];
    uint8_t buff_in[1024];
    uint8_t buff_out[1024];
    IOCoupler_Device IOCoupler_Devices; 
    uint8_t reserved_b[512];
}ipc_data_t;

void init_ipc_sharemem(void);
void app_ipc_sharemem_lock(void);
void app_ipc_sharemem_unlock(void);

void app_cache_read_sharemem(void);
void app_cache_write_sharemem(void);

void Master_print_io_info(void);

#endif /* IPC_SHARE_MEM_H */