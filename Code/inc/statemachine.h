#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#define EW_RTC_ALARM_EVENT          0
#define EW_LEVER_DOWN_EVENT         1
#define EW_LEVER_UP_EVENT           2
#define EW_TOGGLE_CHANGE_EVENT      3
#define EW_ENCODER_BUTTON_EVENT     4
#define EW_ENCODER_CHANGED_EVENT    5
#define EW_SNOOZE_TIMER_EVENT       6
#define EW_STANDBY_TIMER_EVENT      7


#define EW_TIMEOUT_SHORT            10      // short timeout, after starting snooze
#define EW_TIMEOUT_LONG             300     // long timeout, after showing current time


typedef enum {
    EW_STARTUP,
    EW_INIT,
    EW_IDLE,
    EW_SHOW_ALARM,
    EW_SET_HOURS,
    EW_SET_MINUTES,
    EW_ALARM_RINGING,
    EW_STANDBY,
    EW_ERROR,
} ew_state_t;


ew_state_t handle_event(eventid_t new_event, ew_state_t current_state);


#endif /* STATEMACHINE_H */
