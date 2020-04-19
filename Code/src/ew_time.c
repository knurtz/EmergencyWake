//===========================================================================
// Functions for time management.
// Involves controlling the RTC module, saving alarms to EEPROM, modifying alarms and time and updating the time from DCF77 radio clock source.
//===========================================================================

#include "ch.h"
#include "hal.h"
#include "ew_time.h"
#include "ew_statemachine.h"

void toggleAlarmEnable(ew_togglevalue_t toggle);    
void setAlarmEnable(ew_togglevalue_t toggle, bool enabled);
void toggleDST();           // toggle daylight savings time

void modifyHoursValue(ew_togglevalue_t toggle, uint8_t delta);
void modifyMinutesValue(ew_togglevalue_t toggle, uint8_t delta);
void setHoursValue(ew_togglevalue_t toggle, uint8_t hours);
void setMinutesValue(ew_togglevalue_t toggle, uint8_t minutes);    

void saveTime(ew_togglevalue_t toggle);             // save time to RTC module, alarms to eeprom