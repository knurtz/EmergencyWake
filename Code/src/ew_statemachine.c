//===========================================================================
// Functions for handling the main statemachine.
// Involves reacting to different events and switching between states
//===========================================================================

#include "ch.h"
#include "hal.h"
#include "chprintf.h"
//#include "usbcfg.h"
#include "ew_statemachine.h"
#include "device_status.h"
#include "ew_time.h"
#include <string.h>
#include "ew_events.h"

typedef enum ew_togglevalue {
    EW_TOGGLE_TIME,
    EW_TOGGLE_ALARM_A,
    EW_TOGGLE_ALARM_B,
} ew_togglevalue_t;

static systime_t lever_down_time;                   // used to store systime when lever gets pulled down

//===========================================================================
// helper functions to get or set specific values
//===========================================================================

ew_togglevalue_t getToggleValue() {
    uint8_t ret = palReadLine(LINE_TOGGLE_DN) << 1 | palReadLine(LINE_TOGGLE_UP);   // yields 1 for toggle up position and 2 for toggle down position
    return (ret > 2) ? EW_TOGGLE_TIME : ret;
};

/*
int8_t getEncoderDifference() {
    uint8_t encoder_new;      // = read encoder timer counter register
    int8_t ret = encoder_old - encoder_new;
    encoder_old = encoder_new;
    return ret;
};
*/


//===========================================================================
// functions for entering a new state
//===========================================================================

ew_state_t enterIdle(uint8_t display_timeout) {
    // tell display thread to show current time and activated alarms, enable second blink
    // start standby timer depending on value of display_timeout
};

ew_state_t enterShowAlarm(ew_togglevalue_t toggle) {
    // tell display thread to show alarm a or b, no second blink, highlight selected alarm number
};

ew_state_t enterSetHours(ew_togglevalue_t toggle) {
    // tell display thread to only display hours of current time or selected alarm (maybe display minutes in lower brightness)
};

ew_state_t enterSetMinutes(ew_togglevalue_t toggle){
    // tell display thread to only display minutes of current time or selected alarm
};

ew_state_t enterAlarmRinging() {
    // tell display thread to display the corresponding alarm, blinking alarm number
    // tell audio thread to start playing alarm sound
    device_status.active_alarm = device_status.next_alarm;
    chEvtBroadcastFlags(&display_event, 0);
    chEvtBroadcastFlags(&audio_event, 0);
};

ew_state_t enterStandby() {

    // tell display thread to shut down DCDC converter and display driver IC
    chEvtBroadcastFlags(&display_event, 1);
    // wait for display thread to terminate
    
    // tell audio thread to stop playing any sound and disable the audio codec IC and MCLK
    chEvtBroadcastFlags(&audio_event, 0);
    // wait for audio thread to terminate

    PWR->CSR |= (PWR_CSR_EWUP);                     // enable wakeup on pin PA0
    RTC->ISR &= ~(RTC_ISR_ALRBF | RTC_ISR_ALRAF);   // clear RTC alarm flags
    // disable RTC alarm a (minute interrupt)
    PWR->CR  |= (PWR_CR_CSBF | PWR_CR_CWUF);        // clear wakeup flag -> any wakeup event after this instruction will keep device from entering standby

    // tell proximity thread to clear interrupt from proximity sensor and disable ambient light engine

    // go into standby
    chSysLock();
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    PWR->CR  |= (PWR_CR_PDDS);			            // enable standby instead of stop mode when entering deepsleep
    __WFI();

    return EW_STANDBY;
};


//===========================================================================
// main state machine handler
//===========================================================================

ew_state_t handleEvent(uint16_t new_event, uint16_t flags) {

    ew_state_t old_state = device_status.state;
    ew_state_t new_state = EW_IDLE;

    ew_togglevalue_t toggle_value = getToggleValue();   // determines whether one of the two alarms or the current time is selected

    chprintf((BaseSequentialStream*)&SD1, "Handling event: 0x%x\r\n", new_event);

    if (new_event & EVENT_MASK(EW_USER_ALARM))
        // user alarm event: flag carries alarm identifier

        return(enterAlarmRinging());

    if (new_event & EVENT_MASK(EW_LEVER_DOWN)) {
        switch (old_state) {
            case EW_ALARM_RINGING:
                stopRingtone();
                lever_down_time = chVTGetSystemTimeX();
                return(old_state);      // stay in this state until lever goes up again
            case EW_SHOW_ALARM:
                toggleAlarmEnable(toggle_value);
                return(old_state);
            case EW_SET_HOURS:
            case EW_SET_MINUTES:
                if (toggle_value == 0) toggleDST();
                return(old_state);
        }
        return(old_state);
    }

    if (new_event & EVENT_MASK(EW_LEVER_UP)) {
        switch (old_state) {
            case EW_IDLE:
                return(enterStandby());
            case EW_ALARM_RINGING:
                // if lever is released within 2 seconds after pulling down, enable snooze, if not restore saved alarm
                if (chVTIsSystemTimeWithinX(lever_down_time, lever_down_time + TIME_MS2I(2000))) {
                    // increase alarm time by snooze time
                    // play alarm snoozed sound
                }
                else {
                    // restore ringing alarm from eeprom
                    // play alarm stopped sound
                }                
                device_status.active_alarm = EW_ALARM_NONE;      // for now, no alarm is ringing anymore          
                return(enterIdle(EW_TIMEOUT_SHORT));
        }
        return(old_state);
    }

    if (new_event & EVENT_MASK(EW_TOGGLE_CHANGE_)) {
        // when an alarm is ringing, toggle input is ignored
        if (old_state == EW_ALARM_RINGING) return(old_state);
        // when returning to homescreen, save any changes done to alarms so far
        if (toggle_value == 0) {
            if (old_state == EW_SET_HOURS || old_state == EW_SET_MINUTES) saveTime(toggle_value);
            return(enterIdle(EW_TIMEOUT_LONG));
        }
        // when not returning to homescreen, new toggle value is either set to alarm a or b, so show corresponding alarm
        return(enterShowAlarm(toggle_value));
    }

    if (new_event & EVENT_MASK(EW_ENCODER_BUTTON)) {
        switch (old_state) {
            case EW_IDLE:
                return(enterSetHours(toggle_value));
            case EW_SHOW_ALARM:
                // only enter alarm setup if alarm is currently enabled
                if (isAlarmEnabled(toggle_value)) return(enterSetHours(toggle_value));
            case EW_SET_HOURS:
                return(enterSetMinutes(toggle_value));
            case EW_SET_MINUTES:
                saveTime(toggle_value);
                if (toggle_value == 0) return(enterIdle(EW_TIMEOUT_LONG));
                return(enterShowAlarm(toggle_value));
        }
        return(old_state);
    }

    if (new_event & EVENT_MASK(EW_ENCODER_CHANGED)) {
        switch(old_state) {
            case EW_SET_HOURS:
                // set new hour value for alarm or current time
                break;
            case EW_SET_MINUTES:
                // set new minute value
                break;
        }
        return(old_state);
    }

    if (new_event & EVENT_MASK(EW_SNOOZE_TIMER)) {
        // disable alarm automatically after some time
        stopRingtone();
        toggleAlarmEnable(device_status.active_alarm);       // alarms that were ignored once will be turned off to prevent this in the future
        saveAlarm();
        device_status.active_alarm = EW_ALARM_NONE;
        return(enterIdle(EW_TIMEOUT_SHORT));
    }

    if (new_event & EVENT_MASK(EW_STANDBY_TIMER))
        return(enterStandby());

}
