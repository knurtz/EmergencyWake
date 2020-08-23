#ifndef EW_TIME_H
#define EW_TIME_H

#include "ch.h"

typedef enum {
    EW_ALARM_DISABLED,
    EW_ALARM_ENABLED,
    EW_ALARM_SNOOZED
} ew_alarmstate_t;

typedef enum {
    EW_ALARM_ONE,
    EW_ALARM_TWO,
    EW_ALARM_NONE
} ew_alarmnumber_t;

typedef struct {
    int8_t hours;
    int8_t minutes;
} ew_time_t;

typedef struct {
        ew_time_t saved_time;           // alarm as it is set up by the user
        ew_time_t modified_time;        // used while the user sets up a new alarm time (before saving)
        uint16_t snooze_timer;           // how many minutes to add to original alarm time due to a snoozing user
        ew_alarmstate_t state;          // alarm state
} ew_alarm_t;

/*
void toggleAlarmEnable(ew_togglevalue_t toggle);    
void setAlarmEnable(ew_togglevalue_t toggle, bool enabled);
void toggleDST();           // toggle daylight savings time

void modifyHoursValue(ew_togglevalue_t toggle, uint8_t delta);
void modifyMinutesValue(ew_togglevalue_t toggle, uint8_t delta);
void setHoursValue(ew_togglevalue_t toggle, uint8_t hours);
void setMinutesValue(ew_togglevalue_t toggle, uint8_t minutes);    

void saveTime(ew_togglevalue_t toggle);             // save time to RTC module, alarms to eeprom
*/

// initialize RTC module and reset to given time
void initRTC(ew_time_t time);

// toggle selected alarm between enabled and disabled state
void toggleAlarmEnable(ew_alarmnumber_t alarm);

// enable or disable selected alarm
void setAlarmEnable(ew_alarmnumber_t alarm, bool enabled);

// add or subtract one to / from hours field of currently modfied time
void changeHours(ew_alarmnumber_t toggle_value, bool inc);

// add or subtract one to / from minutes field of currently modfied time
void changeMinutes(ew_alarmnumber_t toggle_value, bool inc);

// toggle daylight savings time for current time
void toggleDST(void);

// save new hours and minutes values from modified_time field
void saveChanges(ew_alarmnumber_t selection);

// add snooze time to selected alarm
void increaseSnoozeTime(ew_alarmnumber_t alarm);

#endif /* EW_TIME_H */
