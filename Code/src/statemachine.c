#include "ch.h"
#include "hal.h"

#include "statemachine.h"






ew_state_t handle_event(eventid_t new_event, ew_state_t current_state) {

    uint8_t toggle_value; // = read toggle value

    switch (new_event) {
        case EW_RTC_ALARM_EVENT:
            return(enterAlarmRinging(/*alarmA or alarmB*/));

        case EW_LEVER_DOWN_EVENT:
            switch (current_state) {
                case EW_ALARM_RINGING:
                    //stopRingtone();
                    //save timestamp for later
                    return(current_state);
                case EW_SHOW_ALARM:
                    //toggleAlarmEnable(toggle_value);
                    return(current_state);
                case EW_SET_HOURS:
                case EW_SET_MINUTES:
                    //if (toggle_status == 0) toggleDST();
                    return(current_state);
            }
            return(current_state);

        case EW_LEVER_UP_EVENT:
            switch (current_state) {
                case EW_IDLE:
                    return(enterStandby());
                case EW_ALARM_RINGING:
                    //get time, compare to saved value from line 16
                    //if elapsed time < 2s:
                    //setAlarm(alarm_id, current_time + snooze_time);
                    //else
                    //setAlarm(alarm_id, stored_alarm_time)
                    return(enterIdle(EW_TIMEOUT_SHORT));
            }
            return(current_state);

        case EW_TOGGLE_CHANGE_EVENT:
            if (current_state != EW_ALARM_RINGING) {
                if (toggle_value == 0) return(enterIdle());
                return(enterShowAlarm(toggle_value));
            }
            return(current_state);

        case EW_ENCODER_BUTTON_EVENT:
            switch (current_state) {
                case EW_IDLE:
                case EW_SHOW_ALARM:
                    return(enterSetHours(toggle_value));
                case EW_SET_HOURS:
                    return(enterSetMinutes(toggle_value));
                case EW_SET_MINUTES:
                    //saveTime(toggle_value);
                    //if (toggle_value == 0) return(enterIdle());
                    return(enterShowAlarm(toggle_value));
            }
            return(current_state);

        case EW_ENCODER_CHANGED_EVENT:
            uint8_t encoder_value; // = get encoder value
            switch(current_state) {
                case EW_SET_HOURS:
                    // set new hour value
                    break;
                case EW_SET_MINUTES:
                    // set new minute value
                    break;
            }
            return(current_state);

        case EW_SNOOZE_TIMER_EVENT:
            //stopRingtone();
            //toggleAlarmEnable(currently_ringing);
            //saveAlarm(currently_ringing);
            return(enterIdle());

        case EW_STANDBY_TIMER_EVENT:
            return(enterStandby());

        default:
            return(current_state);
    }
}