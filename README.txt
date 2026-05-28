Gespant IO Coupler AM243

+-----------------------------------------------------------+
|                    AM2432 Dual Core                       |
+-----------------------------------------------------------+

 CORE0 : CANOpen Master                  CORE1 : Ethernet Core
+--------------------------------+      +--------------------------------+
| CANOpen Master Stack           |      | EtherNet/IP Adapter            |
| PDO Processing                 |      | EtherCAT Slave                 |
| SDO Configuration              |      | Modbus TCP Server              |
| Slave Scanning                 |      | Embedded WebServer             |
| Sync Management                |      | REST/WebSocket API             |
| Error Handling                 |      | Configuration Service          |
+---------------+----------------+      +----------------+---------------+
                |                                        |
                | Shared Memory IPC                      |
                +-------------------+--------------------+
                                    |
                   +-------------------------------+
                   | Shared Memory Regions         |
                   |                               |
                   | Process Data Image            |
                   | Input Buffer                  |
                   | Output Buffer                 |
                   | System Status                 |
                   | CANOpen Master Status         |
                   | CANOpen Master Command        |
                   | Slave Information             |
                   | Error Information             |
                   +-------------------------------+
                   
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++                   
Memory Configuration
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MSRAM
    Core0
        MSRAM : ORIGIN = 0x70080000, LENGTH = 0x48000
        (Range 70080000 - 700C7FFF; Size 288 KB)
    ----------------------------------------------------------
    Reserved (Separated cores)
        0x700C8000 - 0x700C9FFF (8 KB)
    ----------------------------------------------------------
    Core1
        MSRAM : ORIGIN = 0x700CA000, LENGTH = 0x106000
        (Range 700CA000 - 701CFFFF; Size 1.024 MB + 24 KB)
--------------------------------------------------------------
USER_SHM_MEM
    USER_SHM_MEM : ORIGIN = 0x701D0000, LENGTH = 0x4000
    (Range 0x701D0000 - 0x701D3FFF; 16 KB)
--------------------------------------------------------------
LOG_SHM_MEM
    LOG_SHM_MEM : ORIGIN = 0x701D4000, LENGTH = 0x4000
    (Range 0x701D4000 - 0x701D7FFF; 16 KB)
--------------------------------------------------------------
RTOS_NORTOS_IPC_SHM_MEM
    RTOS_NORTOS_IPC_SHM_MEM : ORIGIN = 0x701D8000, LENGTH = 0x8000
    (Range 0x701D8000 - 0x701DBFFF; 32 KB)
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
FLASH (16MB)
    Core0
        FLASH : ORIGIN = 0x60100000, LENGTH = 0x400000
        (Range 0x60100000 - 0x604FFFFF; Size 4MB)
    ----------------------------------------------------------
    Reserved (Separated cores)
        0x60500000 - 0x60600000 (1 MB)
    ----------------------------------------------------------
    Core1
        FLASH : ORIGIN = 0x60600000, LENGTH = 0xA00000
        (Range 0x60600000 - 0x60FFFFFF; Size 10 MB)
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++