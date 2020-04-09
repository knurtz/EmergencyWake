#include "ch.h"
#include "hal.h"

#include "statemachine.h"




static systime_t start_time;            // used to store systime when lever gets pulled down

ew_state_t handle_event(eventid_t new_event, ew_state_t current_state) {

    uint8_t toggle_value; // = read toggle value

    switch (new_event) {
        case EW_RTC_ALARM_EVENT:
            return(enterAlarmRinging(/*alarmA or alarmB*/));

        case EW_LEVER_DOWN_EVENT:
            switch (current_state) {
                case EW_ALARM_RINGING:
                    stopRingtone();
                    start_time = chVTGetSystemTimeX();
                    return(current_state);
                case EW_SHOW_ALARM:
                    toggleAlarmEnable(toggle_value);
                    return(current_state);
                case EW_SET_HOURS:
                case EW_SET_MINUTES:
                    if (toggle_value == 0) toggleDST();
                    return(current_state);
            }
            return(current_state);

        case EW_LEVER_UP_EVENT:
            switch (current_state) {
                case EW_IDLE:
                    return(enterStandby());
                case EW_ALARM_RINGING:
                    if(chVTIsSystemTimeWithinX(start_time, start_time + TIME_MS2I(500)))
                        setAlarm(/*alarm_id, current_time + snooze_time*/);
                    else
                        setAlarm(/*alarm_id, stored_alarm_time*/);
                    return(enterIdle(EW_TIMEOUT_SHORT));
            }
            return(current_state);

        case EW_TOGGLE_CHANGE_EVENT:
            if (current_state != EW_ALARM_RINGING) {
                if (toggle_value == 0) return(enterIdle(EW_TIMEOUT_LONG));
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
                    saveTime(toggle_value);
                    if (toggle_value == 0) return(enterIdle(EW_TIMEOUT_LONG));
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
            stopRingtone();
            toggleAlarmEnable(/*currently_ringing*/);
            saveAlarm(/*currently_ringing*/);
            return(enterIdle(EW_TIMEOUT_SHORT));

        case EW_STANDBY_TIMER_EVENT:
            return(enterStandby());

        default:
            return(current_state);
    }
}
