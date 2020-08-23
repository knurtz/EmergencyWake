//===========================================================================
// Functions for time management.
// Involves controlling the RTC module, saving alarms to EEPROM, modifying alarms and time and updating the time from DCF77 radio clock source.
//===========================================================================

#include "ch.h"
#include "hal.h"
#include "ew_time.h"
#include "ew_devicestatus.h"


// initialize RTC module and reset to given time
void initRTC(ew_time_t time) {

}

// toggle selected alarm between enabled and disabled state
void toggleAlarmEnable(ew_alarmnumber_t alarm) {
    if (alarm >= EW_ALARM_NONE) return;
    setAlarmEnable(alarm, (device_status.alarms[alarm].state == EW_ALARM_DISABLED));        // if alarm was disabled enable, otherwise disable
}

// enable or disable selected alarm
void setAlarmEnable(ew_alarmnumber_t alarm, bool enabled) {
    if (alarm >= EW_ALARM_NONE) return;
    device_status.alarms[alarm].state = enabled ? EW_ALARM_ENABLED : EW_ALARM_DISABLED;
    device_status.unsaved_changes = true;
}

// add or subtract one to / from hours field of currently modfied time
void changeHours(ew_alarmnumber_t selection, bool inc) {
    int8_t *hour_pointer = 
        (selection == EW_ALARM_NONE) ? 
        &device_status.modified_time.hours : 
        &device_status.alarms[selection].modified_time.hours;

    *hour_pointer += inc ? 1 : -1;
    if (*hour_pointer > 23) *hour_pointer = 0;
    if (*hour_pointer < 0) *hour_pointer = 23;
}

// add or subtract one to / from minutes field of currently modfied time
void changeMinutes(ew_alarmnumber_t selection, bool inc) {
    int8_t *minutes_pointer = 
        (selection == EW_ALARM_NONE) ? 
        &device_status.modified_time.minutes : 
        &device_status.alarms[selection].modified_time.minutes;

    *minutes_pointer += inc ? 1 : -1;
    if (*minutes_pointer > 59) *minutes_pointer = 0;
    if (*minutes_pointer < 0) *minutes_pointer = 59;
}

// toggle DST for system time
void toggleDST(void) {
    
}

// save new hours and minutes values stored in modified_time field of selected alarm or system time
void saveChanges(ew_alarmnumber_t selection) {
    // modifications to system time are saved by re-initializing the RTC module
    if (selection == EW_ALARM_NONE) initRTC(device_status.modified_time);

    // modifications to a user alarm are stored in device status
    else {
        device_status.alarms[selection].saved_time = device_status.alarms[selection].modified_time;
        device_status.unsaved_changes = true;
    }
}

// add snooze time to selected alarm
void increaseSnoozeTime(ew_alarmnumber_t alarm) {
    device_status.alarms[alarm].snooze_timer += device_status.snooze_time;
}