#ifndef EW_EVENTS_H
#define EW_EVENTS_H

#include "ch.h"

#define EW_RTC_USER_ALARM_EVENT         0
#define EW_LEVER_DOWN_EVENT             1
#define EW_LEVER_UP_EVENT               2
#define EW_TOGGLE_CHANGE_EVENT          3
#define EW_ENCODER_BUTTON_EVENT         4
#define EW_ENCODER_CHANGED_EVENT        5
#define EW_SNOOZE_TIMER_EVENT           6
#define EW_STANDBY_TIMER_EVENT          7

// Project wide event sources. These all get defined and initialized in main.c
// They might be subscribed and broadcasted to by many different threads within the project.

// UI related, processed by statemachine
extern event_source_t toggle_changed_event;
extern event_source_t lever_down_event;
extern event_source_t lever_up_event;
extern event_source_t encoder_changed_event;
extern event_source_t encoder_button_event;
extern event_source_t proximity_event;

// RTC related, also processed by statemachine
extern event_source_t user_alarm_event;     // whether alarm A or alarm B can be deducted by next_alarm value in device status

// Display related, processed by display thread
extern event_source_t display_event;        // which part of the information inside "device_status" to display is encoded in event flags

// Audio related, processed by audio thread
extern event_source_t audio_event;          // which sound or whether to stop audio is encoded in event flags

#endif /* EW_EVENTS_H */
