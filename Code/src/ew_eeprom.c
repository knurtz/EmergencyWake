#include "ch.h"
#include "hal.h"
#include "ew_devicestatus.h"

// save device_status to eeprom
void saveToEeprom(void) {
    if (device_status.unsaved_changes) return;
}
