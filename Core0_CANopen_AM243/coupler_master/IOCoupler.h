#ifndef IOCOUPLER_H
#define IOCOUPLER_H 

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "canfestival.h"
#include "data.h"
#include "ipc_shareMem.h"

void IOCoupler_scan_task(CO_Data* d);
ScanState_t get_scan_state(void);
void ODMaster_Init_PDO(CO_Data *d);
void ODMaster_configure_PDO(CO_Data *d);
void IOCoupler_reset(CO_Data *d);

#endif /* IOCOUPLER_H */