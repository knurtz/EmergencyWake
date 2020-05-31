//===========================================================================
// Functions for handling the main statemachine.
// Involves reacting to different events and switching between states
//===========================================================================

#include "ch.h"
#include "chprintf.h"
#include "hal.h"
//#include "usbcfg.h"
#include <string.h>

#include "device_status.h"
#include "ew_events.h"
#include "ew_statemachine.h"
#include "ew_time.h"

/*
typedef enum ew_togglevalue {
    EW_TOGGLE_TIME,
    EW_TOGGLE_ALARM_ONE,
    EW_TOGGLE_ALARM_TWO,
} ew_togglevalue_t;
*/

static systime_t lever_down_time;  // used to store systime when lever gets pulled down

//===========================================================================
// helper functions to get or set specific values
//===========================================================================

ew_alarmnumber_t getToggleValue() {
    if (palReadLine(LINE_TOGGLE_UP)) return(EW_ALARM_ONE);
    if (palReadLine(LINE_TOGGLE_DN)) return(EW_ALARM_TWO);
    else return(EW_ALARM_NONE);
}

bool isAlarmEnabled(ew_alarmnumber_t toggle) {
    // illegal condition: checking an alarm even though toggle value is in middle position
    if (toggle == EW_ALARM_NONE) return false;
    if (device_status.alarms[toggle].state == EW_ALARM_DISABLED) return false;
    return true;
}

// defined in main.c
ew_alarmnumber_t findNextAlarm(void);
ew_time_t findNextAlarmTime(void);

//===========================================================================
// functions for entering a new state
//===========================================================================

ew_state_t enterIdle(uint8_t display_timeout){
    // tell display thread to show current time and activated alarms, enable second blink
    // start standby timer depending on value of display_timeout
};

ew_state_t enterShowAlarm(ew_alarmnumber_t toggle){
    // tell display thread to show alarm a or b, no second blink, highlight selected alarm number
};

ew_state_t enterSetHours(ew_alarmnumber_t toggle){
    // tell display thread to only display hours of current time or selected alarm (maybe display minutes in lower brightness)
};

ew_state_t enterSetMinutes(ew_alarmnumber_t toggle){
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

    PWR->CSR |= (PWR_CSR_EWUP);                    // enable wakeup on pin PA0
    RTC->ISR &= ~(RTC_ISR_ALRBF | RTC_ISR_ALRAF);  // clear RTC alarm flags
    // disable RTC alarm A (minute interrupt)
    PWR->CR |= (PWR_CR_CSBF | PWR_CR_CWUF);  // clear wakeup flag -> any wakeup event after this instruction will keep device from entering standby

    // tell proximity thread to clear interrupt from proximity sensor and disable ambient light engine

    // go into standby
    chSysLock();
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    PWR->CR |= (PWR_CR_PDDS);  // enable standby instead of stop mode when entering deepsleep
    __WFI();

    return EW_STANDBY;
};

//===========================================================================
// main state machine handler
//===========================================================================

ew_state_t handleEvent(uint16_t new_event, uint16_t flags) {
    ew_state_t old_state = device_status.state;
    ew_state_t new_state = old_state;

    // get current state of toggle.
    // It determines which screen is displayed and what value all user actions like changing hours or minutes apply to (current time or user alarm one or two)
    ew_alarmnumber_t toggle_value = getToggleValue();  // determines whether one of the two alarms or the current time is selected

    chprintf((BaseSequentialStream*)&SD1, "Handling event: 0x%x\r\n", new_event);

    // Check each event. It is possible to handle multiple events in the same cycle.
    // The order of checks should make sure they still yield the correct result.
    // Most checks keep the device in the same state. If not, this is indicated by assigning a new value to the variable new_state.

    if (new_event & EVENT_MASK(EW_USER_ALARM))
        // user alarm event
        new_state = enterAlarmRinging();

    if (new_event & EVENT_MASK(EW_LEVER_DOWN)) {
        switch (old_state) {
            case EW_ALARM_RINGING:
                chEvtBroadcastFlags(&audio_event, 1);       // stop alarm sound
                lever_down_time = chVTGetSystemTimeX();
            case EW_SHOW_ALARM:
                toggleAlarmEnable(toggle_value);
            case EW_SET_HOURS:
            case EW_SET_MINUTES:
                // toggling DST not available when modifying a user alarm
                if (toggle_value == EW_ALARM_NONE) toggleDST();
        }
    }

    if (new_event & EVENT_MASK(EW_LEVER_UP)) {
        switch (old_state) {
            case EW_IDLE:
                new_state = enterStandby();
            case EW_ALARM_RINGING:
                // if lever is released within 2 seconds after pulling down enable snooze, if not restore saved alarm
                if (chVTIsSystemTimeWithinX(lever_down_time, lever_down_time + TIME_MS2I(2000))) {
                    increaseSnoozeTime(device_status.active_alarm);
                    chEvtBroadcastFlags(&audio_event, 2);       // play alarm snoozed sound
                } 
                else {
                    chEvtBroadcastFlags(&audio_event, 3);       // play alarm stopped sound
                }
                device_status.active_alarm = EW_ALARM_NONE;     // for now, no alarm is ringing anymore
                // recalculate next alarm
                device_status.next_alarm = findNextAlarm();
                device_status.next_alarm_time = findNextAlarmTime();
                if (toggle_value == EW_ALARM_NONE) new_state = enterIdle(EW_TIMEOUT_SHORT);
                else new_state = enterShowAlarm(toggle_value);
        }
        return (old_state);
    }

    if (new_event & EVENT_MASK(EW_TOGGLE_CHANGE)) {
        // while an alarm is ringing, toggle input is ignored
        if (old_state != EW_ALARM_RINGING) {
            // when changing toggle from within a setup routine, save any changes done so far
            if (old_state == EW_SET_HOURS || old_state == EW_SET_MINUTES) saveTime(toggle_value);
            if (toggle_value == EW_ALARM_NONE) new_state = enterIdle(EW_TIMEOUT_LONG);
            else new_state = enterShowAlarm(toggle_value);
        }
    }

    if (new_event & EVENT_MASK(EW_ENCODER_BUTTON)) {
        switch (old_state) {
            case EW_IDLE:
                new_state = enterSetHours(toggle_value);
            case EW_SHOW_ALARM:
                // only enter alarm setup if alarm is currently enabled
                if (isAlarmEnabled(toggle_value)) new_state = enterSetHours(toggle_value);
            case EW_SET_HOURS:
                new_state = enterSetMinutes(toggle_value);
            case EW_SET_MINUTES:
                saveTime(toggle_value);
                if (toggle_value == 0) new_state = enterIdle(EW_TIMEOUT_LONG);
                new_state = enterShowAlarm(toggle_value);
        }
    }

    if (new_event & EVENT_MASK(EW_ENCODER_CHANGED)) {
        bool inc = flags & EVENT_MASK(EW_ENCODER_CHANGED);
        switch (old_state) {
            case EW_SET_HOURS:
                changeHours(toggle_value, inc);
                break;
            case EW_SET_MINUTES:
                changeMinutes(toggle_value, inc);
                break;
        }
    }

    if (new_event & EVENT_MASK(EW_SNOOZE_TIMER)) {
        // disable alarm automatically after some time
        stopRingtone();
        toggleAlarmEnable(device_status.active_alarm);  // alarms that were ignored once will be turned off to prevent this in the future
        saveAlarm();
        device_status.active_alarm = EW_ALARM_NONE;
        return (enterIdle(EW_TIMEOUT_SHORT));
    }

    if (new_event & EVENT_MASK(EW_STANDBY_TIMER))
        return (enterStandby());
}
