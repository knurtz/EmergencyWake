//===========================================================================
// Functions for handling the main statemachine.
// Involves reacting to different events and switching between states
//===========================================================================

#include "ch.h"
#include "hal.h"

#include <string.h>
#include "chprintf.h"

#include "ew_statemachine.h"
#include "ew_events.h"
#include "ew_time.h"
#include "ew_devicestatus.h"

//===========================================================================
// Variables and local functions
//===========================================================================

// used to store systime when lever gets pulled down
static systime_t lever_down_time;

//===========================================================================
// functions for entering a new state
//===========================================================================

ew_state_t enterIdle(int display_timeout) {
    // tell display thread to show current time and activated alarms, enable second blink
    // start standby timer depending on value of display_timeout
    return EW_IDLE;
}

static ew_state_t enterShowAlarm(void) {
    // tell display thread to show alarm a or b, no second blink, highlight selected alarm number
    return EW_SHOW_ALARM;
}

static ew_state_t enterSetHours(void) {
    // tell display thread to only display hours of current time or selected alarm (maybe display minutes in lower brightness)
    return EW_SET_HOURS;
}

static ew_state_t enterSetMinutes(void) {
    // tell display thread to only display minutes of current time or selected alarm
    return EW_SET_MINUTES;
}

ew_state_t enterAlarmRinging(void) {
    // tell audio thread to start playing alarm sound
    device_status.active_alarm = device_status.next_alarm;
    chEvtBroadcastFlags(&audio_event, EVENT_MASK(EW_AUDIO_PLAY_ALARM));
    // TODO: start timeout timer to stop alarm after x minutes if no one pays attention
    return EW_ALARM_RINGING;
}

static ew_state_t enterStandby(void) {
    // tell display thread to shut down DCDC converter and display driver IC
    chEvtBroadcastFlags(&display_event, EVENT_MASK(EW_DISPLAY_TURNOFF));
    // wait for display thread to terminate

    // tell audio thread to stop playing any sound and disable the audio codec IC and MCLK
    chEvtBroadcastFlags(&audio_event, EVENT_MASK(EW_AUDIO_TURNOFF));
    // wait for audio thread to terminate

    // save current configuration to eeprom
    saveToEeprom();

    PWR->CSR |= (PWR_CSR_EWUP);                    // enable wakeup on pin PA0
    RTC->ISR &= ~(RTC_ISR_ALRBF | RTC_ISR_ALRAF);  // clear RTC alarm flags
    // disable RTC alarm A (minute interrupt)
    PWR->CR |= (PWR_CR_CSBF | PWR_CR_CWUF);  // clear wakeup flag -> any wakeup event after this instruction will keep device from entering standby

    // tell proximity thread to clear interrupt from proximity sensor and disable ambient light engine

    // go into standby
    chSysLock();
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
    PWR->CR |= (PWR_CR_PDDS);               // enable standby instead of stop mode when entering deepsleep
    //__WFI();

    return EW_STANDBY;
}

//===========================================================================
// main state machine handler
//===========================================================================

void handleEvent(uint16_t new_event, uint16_t flags) {
    ew_state_t old_state = device_status.state;
    ew_state_t new_state = old_state;

    // as long as the lever is held down (and hasn't just been pulled down), don't process any new events
    if (palReadLine(LINE_LEVER) == LEVER_DOWN && !(new_event & EVENT_MASK(EW_LEVER_DOWN))) return new_state;

    // get current state of toggle.
    // It determines which screen is displayed and what value all user actions like changing hours or minutes apply to (system time or user alarm one or two)
    updateToggleState();

    chprintf((BaseSequentialStream*)&SD1, "Handling event: 0x%x - ", new_event);

    // Check each event. It is possible to handle multiple events in the same cycle.
    // The order of checks should make sure they still yield the correct result.
    // Most checks keep the device in the same state. If not, this is indicated by assigning a new value to the variable new_state.

    if (new_event & EVENT_MASK(EW_USER_ALARM)) {
        // user alarm
        chprintf((BaseSequentialStream*)&SD1, "User alarm\n");
        new_state = enterAlarmRinging();
    }

    if (new_event & EVENT_MASK(EW_LEVER_DOWN)) {
        // lever pulled down
        chprintf((BaseSequentialStream*)&SD1, "Lever pulled down\n");

        switch (old_state) {
            case EW_ALARM_RINGING:
                chEvtBroadcastFlags(&audio_event, EVENT_MASK(EW_AUDIO_STOP_ALARM));       // stop alarm sound and remember at what time the lever was pulled down
                lever_down_time = chVTGetSystemTimeX();
                break;
            case EW_SHOW_ALARM:
                toggleAlarmEnable(device_status.toggle_state);
                break;
            case EW_SET_HOURS:
            case EW_SET_MINUTES:
                if (device_status.toggle_state == EW_ALARM_NONE) toggleDST();     // toggling DST only available while setting up system time
                break;
            default:
                break;
        }
    }

    if (new_event & EVENT_MASK(EW_LEVER_UP)) {
        // lever released
        chprintf((BaseSequentialStream*)&SD1, "Lever released\n");

        switch (old_state) {
            case EW_IDLE:
                new_state = enterStandby();
                break;
            case EW_ALARM_RINGING:
                // if lever is released within 2 seconds after pulling down -> enable snooze, if not -> restore saved alarm
                if (chVTIsSystemTimeWithinX(lever_down_time, lever_down_time + TIME_MS2I(2000))) {
                    increaseSnoozeTime(device_status.active_alarm);
                    chEvtBroadcastFlags(&audio_event, EVENT_MASK(EW_AUDIO_SNOOZE_ALARM));       // play alarm snoozed sound
                } 
                else {
                    chEvtBroadcastFlags(&audio_event, EVENT_MASK(EW_AUDIO_STOP_ALARM));         // play alarm stopped sound
                }
                device_status.active_alarm = EW_ALARM_NONE;                         // no alarm is ringing anymore
                // recalculate next alarm
                device_status.next_alarm = findNextAlarm();
                device_status.next_alarm_time = findNextAlarmTime();
                if (device_status.toggle_state == EW_ALARM_NONE) new_state = enterIdle(EW_TIMEOUT_SHORT);
                else new_state = enterShowAlarm();
                break;
            default:
                break;
        }
    }

    if (new_event & EVENT_MASK(EW_TOGGLE_CHANGE)) {
        // toggle changed
        chprintf((BaseSequentialStream*)&SD1, "Toggle changed\n");

        // while an alarm is ringing, toggle input is ignored
        if (old_state != EW_ALARM_RINGING) {
            // when changing toggle while setting up hours or minutes, save any changes done so far
            // if (old_state == EW_SET_HOURS || old_state == EW_SET_MINUTES) saveChanges(toggle_value);     // skip this, that way the user can cancel setup without saving
            if (device_status.toggle_state == EW_ALARM_NONE) new_state = enterIdle(EW_TIMEOUT_LONG);
            else new_state = enterShowAlarm();
        }
    }

    if (new_event & EVENT_MASK(EW_ENCODER_BUTTON)) {
        // encoder button pressed
        chprintf((BaseSequentialStream*)&SD1, "Encoder button pressed\n");

        switch (old_state) {
            case EW_IDLE:
                new_state = enterSetHours();
                break;
            case EW_SHOW_ALARM:
                // only enter alarm setup if alarm is currently enabled
                if (isAlarmEnabled(device_status.toggle_state)) new_state = enterSetHours();
                break;
            case EW_SET_HOURS:
                new_state = enterSetMinutes();
                break;
            case EW_SET_MINUTES:
                saveChanges(device_status.toggle_state);
                if (device_status.toggle_state == EW_ALARM_NONE) new_state = enterIdle(EW_TIMEOUT_LONG);
                new_state = enterShowAlarm();
                break;
            default:
                break;
        }
    }

    if (new_event & EVENT_MASK(EW_ENCODER_CHANGED)) {
        // encoder changed
        bool inc = flags & EVENT_MASK(EW_ENCODER_CHANGED);
        chprintf((BaseSequentialStream*)&SD1, inc ? "Encoder value increased\n" : "Encoder value decreased\n");

        switch (old_state) {
            case EW_SET_HOURS:
                changeHours(device_status.toggle_state, inc);
                break;
            case EW_SET_MINUTES:
                changeMinutes(device_status.toggle_state, inc);
                break;
            case EW_IDLE:
            case EW_SHOW_ALARM:
                // updateVolume(toggle_value, inc);               // implement on a later day
                break;
            default:
                break;
        }
    }

    if (new_event & EVENT_MASK(EW_STANDBY_TIMER)) {
        // idle screen timeout
        chprintf((BaseSequentialStream*)&SD1, "Idle screen timeout\n");
        new_state = enterStandby();
    }

    device_status.state = new_state;
    chEvtBroadcastFlags(&display_event, EVENT_MASK(EW_DISPLAY_REFRESH));
}
