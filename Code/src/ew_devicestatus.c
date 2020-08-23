#include "ch.h"
#include "hal.h"
#include "ew_devicestatus.h"

// Global variable for device status. Accessed by audio and display thread.
ew_device_status_t device_status;

// Calculates upcoming alarm depending on current time and enabled alarms.
// static response for now
ew_alarmnumber_t findNextAlarm(void) {
    return EW_ALARM_ONE;
}

// Get the time of the upcoming alarm from device status.
ew_time_t findNextAlarmTime(void) {
    ew_alarmnumber_t next_alarm = findNextAlarm();
    ew_time_t ret = {
        device_status.alarms[next_alarm - 1].saved_time.hours,
        device_status.alarms[next_alarm - 1].saved_time.minutes};
    return ret;
}

// find out if alarm is enabled / snoozed or not
bool isAlarmEnabled(ew_alarmnumber_t alarm) {
    if (alarm == EW_ALARM_NONE) return false;
    return !(device_status.alarms[alarm].state == EW_ALARM_DISABLED);
}

ew_alarmnumber_t updateToggleState(void) {
    if (palReadLine(LINE_TOGGLE_UP)) device_status.toggle_state =  EW_ALARM_ONE;
    else if (palReadLine(LINE_TOGGLE_DN)) device_status.toggle_state = EW_ALARM_TWO;
    else device_status.toggle_state = EW_ALARM_NONE;
}

// Retrieves device status from EEPROM and backup registers of the RTC unit after waking up from deep sleep.
// RTC register map:
// BKP0R - snooze timer and state for alarm one
// BKP1R - snooze timer and state for alarm two
// bits 0 - 15 for snooze timer, bits 16 + 17 for alarm state
static void retrieveDeviceStatus(void) {
    device_status.state = EW_STARTUP;
    updateToggleValue();
    device_status.active_alarm = EW_ALARM_NONE;

    device_status.alarms[EW_ALARM_ONE].saved_time.hours = 6;
    device_status.alarms[EW_ALARM_ONE].saved_time.minutes = 23;
    device_status.alarms[EW_ALARM_ONE].snooze_timer = RTC->BKP0R & 0xffff;
    device_status.alarms[EW_ALARM_ONE].state = RTC->BKP0R >> 16 & 0b11;

    device_status.alarms[EW_ALARM_TWO].saved_time.hours = 8;
    device_status.alarms[EW_ALARM_TWO].saved_time.minutes = 41;
    device_status.alarms[EW_ALARM_TWO].snooze_timer = RTC->BKP1R & 0xffff;
    device_status.alarms[EW_ALARM_TWO].state = RTC->BKP1R >> 16 & 0b11;

    device_status.next_alarm = findNextAlarm();
    device_status.next_alarm_time = findNextAlarmTime();

    device_status.alarm_volume = 120;  // get these two from eeprom
    device_status.snooze_time = 10;
    device_status.unsaved_changes = false;

    device_status.dst_enabled = false;  // get this from backup bit of RTC module
}