#include <string.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

#include "device_status.h"
#include "ew_audio.h"
#include "ew_display.h"
#include "ew_events.h"
#include "ew_shell.h"
#include "ew_statemachine.h"
#include "ew_time.h"

//===========================================================================
// Variables and local functions
//===========================================================================

// Global variable for device status. Accessed by audio and display thread.
ew_device_status_t device_status;

// Event listener for main thread statemachine.
event_listener_t el_statemachine;

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

// Retrieves device status from EEPROM and backup registers of the RTC unit after waking up from deep sleep.
// Hard coded values for now.
// RTC register map:
// BKP0R - snooze timer and state for alarm one
// BKP1R - snooze timer and state for alarm two
// bits 0-15 for snooze timer, bits 16 and 17 for alarm state
static void retrieveDeviceStatus(void) {
    device_status.state = EW_STARTUP;
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

    device_status.dst_enabled = false;  // get this from RTC module
}

// Short startup beep
static void confirmationBuzzer(void) {
    palSetLine(LINE_BUZZER);
    chThdSleepMilliseconds(100);
    palClearLine(LINE_BUZZER);
    chThdSleepMilliseconds(50);
    palSetLine(LINE_BUZZER);
    chThdSleepMilliseconds(100);
    palClearLine(LINE_BUZZER);
}

//===========================================================================
// Blinker thread
//===========================================================================

static THD_WORKING_AREA(blinker_wa, 128);
static THD_FUNCTION(blinkerThd, arg) {
    (void)arg;
    chRegSetThreadName("blinker");
    while (true) {
        palSetLine(LINE_LED1);
        chThdSleepMilliseconds(100);
        palSetLine(LINE_LED2);
        chThdSleepMilliseconds(500);
        palClearLine(LINE_LED1);
        chThdSleepMilliseconds(100);
        palClearLine(LINE_LED2);
        chThdSleepMilliseconds(500);
    }
}

//===========================================================================
// Command line related
//===========================================================================

#define SHELL_WA_SIZE THD_WORKING_AREA_SIZE(2048)
static const ShellCommand commands[] = {
    {"tree", cmd_tree},
    {"create", cmd_create},
    {"sdc", cmd_sdc},
    {"i2c", cmd_i2c},
    {NULL, NULL}
};

//===========================================================================
// Driver configs
//===========================================================================

static const I2CConfig i2ccfg = {
    OPMODE_I2C,
    100000,
    STD_DUTY_CYCLE,
};

//static uint8_t sd_scratchpad[512];    // Working area for SDC driver (still necessary?)
static const SDCConfig sdccfg = {
    SDC_MODE_1BIT
};

static const ShellConfig shell_cfg = {
    (BaseSequentialStream *)&SD1,
    commands
};

//===========================================================================
// Events and callbacks
// check ew_events.h for further explanation of how events are used in this project.
// Debouncing all these external interrupts should be done by hardware (low pass filter on every input).
//===========================================================================

event_source_t statemachine_event;

static void toggle_cb(void *arg) {
    (void)arg;
    chSysLockFromISR();
    // Toggle switch has no flags. New position of toggle switch is read inside statemachine handler.
    chEvtBroadcastFlagsI(&statemachine_event, EVENT_MASK(EW_TOGGLE_CHANGE));
    chSysUnlockFromISR();
}

static void lever_cb(void *arg) {
    (void)arg;
    chSysLockFromISR();
    // Lever has no flags, but two seperate events for falling / rising edge. This could technically be handled by two seperate callback functions.
    if (palReadLine(LINE_LEVER) == LEVER_DOWN)
        chEvtBroadcastFlagsI(&statemachine_event, EVENT_MASK(EW_LEVER_DOWN));
    else
        chEvtBroadcastFlagsI(&statemachine_event, EVENT_MASK(EW_LEVER_UP));
    chSysUnlockFromISR();
}

static void encoder_button_cb(void *arg) {
    (void)arg;
    chSysLockFromISR();
    // Encoder center button has no flags.
    chEvtBroadcastFlagsI(&statemachine_event, EVENT_MASK(EW_ENCODER_BUTTON));
    chSysUnlockFromISR();
}

// currently the proximity sensor interrupt is only used to wakeup device from standby and not during operation
/*
static void proximity_interrupt_cb(void *arg) {
    (void)arg;
    chSysLockFromISR();
    chEvtBroadcastFlagsI(&statemachine_event, EVENT_MASK(EW_LEVER_UP));
    chSysUnlockFromISR();
}
*/

static void rtc_alarm_cb(void *arg) {
    (void)arg;
    chSysLockFromISR();
    // RTC alarm has no flags, but two seperate events depending on alarm A or alarm B
    // if RTC alarm a (minute interrupt)
    chEvtBroadcastFlagsI(&display_event, 0);  // set flags to signal minute interrupt to display thread
    // else (user alarm)
    chEvtBroadcastFlagsI(&statemachine_event, EVENT_MASK(EW_USER_ALARM));
    chSysUnlockFromISR();
}


// For TIM 4, used to read rotary encoder, no ChibiOS HAL is used 
CH_IRQ_HANDLER(STM32_TIM4_HANDLER) {
    CH_IRQ_PROLOGUE();

    chSysLockFromISR();
    STM32_TIM4->SR = 0;  // clear all pending TIM4 interrupts
    // encoder changed event has an additional flag, indicating the direction (flag set for positive direction)
    if (STM32_TIM4->CR1 & TIM_CR1_DIR)
        chEvtBroadcastFlagsI(&statemachine_event, 1 << (EVENT_FLAGS_OFFSET + EW_ENCODER_CHANGED) | EVENT_MASK(EW_ENCODER_CHANGED));
    else
        chEvtBroadcastFlagsI(&statemachine_event, EVENT_MASK(EW_ENCODER_CHANGED));
    chSysUnlockFromISR();

    CH_IRQ_EPILOGUE();
}

//===========================================================================
// Application entry point
//===========================================================================

int main(void) {    
    // System initialization
    halInit();
    chSysInit();
    confirmationBuzzer();

    bool restart_after_standby = PWR->CSR & PWR_CSR_SBF;     // did the device just wake up from standby?
    PWR->CR |= PWR_CR_CSBF;                                     // clear standby flag

    // init event source for statemachine
    chEvtObjectInit(&statemachine_event);

    // register this event source as event 0 at our listener
    chEvtRegister(&statemachine_event, &el_statemachine, 0);

    // retrieve device status from eeprom and RTC backup registers
    retrieveDeviceStatus();

    // initialize RTC unit and assign callback functions (next alarm and minute wakeup).

    // start all peripheral drivers that are used in multiple threads (project wide)
    i2cStart(&I2CD2, &i2ccfg);
    sdcStart(&SDCD1, &sdccfg);
    sdStart(&SD1, NULL);

    // start threads
    chThdCreateStatic(blinker_wa, sizeof(blinker_wa), NORMALPRIO - 1, blinkerThd, NULL);
    chThdCreateStatic(audio_wa, sizeof(audio_wa), NORMALPRIO + 1, audioThd, NULL);
    chThdCreateStatic(display_wa, sizeof(display_wa), NORMALPRIO, displayThd, NULL);
    thread_t *shelltp = chThdCreateFromHeap(NULL, SHELL_WA_SIZE, "shell", NORMALPRIO, shellThread, (void *)&shell_cfg);

    // initialize timer for rotary encoder and assign callback function
    rccEnableTIM4(true);
    STM32_TIM4->SMCR = 3;           // encoder mode 3
    STM32_TIM4->CCER = 0;           // rising edge polarity
    STM32_TIM4->ARR = 0x1;          // count between 0 and 1 -> update interrupt occurs every two pulses, matches encoder datasheet
    STM32_TIM4->CCMR1 = 0xc1c1;     // f_DTS/16, N = 8 IC1->TI1, IC2->TI2
    STM32_TIM4->CNT = 0;             // initialize counter
    //STM32_TIM4->EGR = 1;           // generate an update event, clearing the counter
    STM32_TIM4->CR1 = 1;                     // enable the counter
    STM32_TIM4->SR = 0;                      // clear all pending interrupts
    STM32_TIM4->DIER = 1;                    // enable update interrupt
    nvicEnableVector(STM32_TIM4_NUMBER, 7);  // enable TIM4 interrupt vector with priority 7 (default priority from mcuconf.h)

    // activate external interrupts for lever, toggle and buttons and assign callback functions
    palEnableLineEvent(LINE_TOGGLE_UP, PAL_EVENT_MODE_BOTH_EDGES);
    palEnableLineEvent(LINE_TOGGLE_DN, PAL_EVENT_MODE_BOTH_EDGES);
    palSetLineCallback(LINE_TOGGLE_UP, toggle_cb, NULL);
    palSetLineCallback(LINE_TOGGLE_DN, toggle_cb, NULL);

    palEnableLineEvent(LINE_LEVER, PAL_EVENT_MODE_BOTH_EDGES);
    palSetLineCallback(LINE_LEVER, lever_cb, NULL);

    palEnableLineEvent(LINE_ENC_BUT, PAL_EVENT_MODE_FALLING_EDGE);
    palSetLineCallback(LINE_ENC_BUT, encoder_button_cb, NULL);

    /*
    palEnableLineEvent(LINE_I2C_INT, PAL_EVENT_MODE_FALLING_EDGE);
    palSetLineCallback(LINE_I2C_INT, proximity_interrupt_cb, NULL);
    */

    // if woken up from standby, this might be because of a user alarm. if this is the case, immediately enter alarm ringing state
    if (restart_after_standby) {
        /*
        // yields 1 for alarm A, 2 for alarm B, 3 for both.
        // after standby only 0 or 2 are valid, since alarm A is used for minute interrupt, which is disabled before going to sleep
        uint16_t alarm_flags = (RTC->ISR & (RTC_ISR_ALRAF | RTC_ISR_ALRBF)) >> RTC_ISR_ALRAF_Pos;
        if (alarm_flags > 0) current_state = 
        enterAlarmRinging(alarm_flags);
        */
    }

    // wait for an event to occur
    while (true) {
        eventmask_t new_event = chEvtWaitAny(0);
        // retrieve event flags - check ew_event.h for more information on how events and event flags are used in this project.
        eventflags_t flags = chEvtGetAndClearFlags(&el_statemachine);
        handleEvent((uint16_t)(flags & 0xffff), (uint16_t)(flags >> EVENT_FLAGS_OFFSET));
    }    

}  // end of main()
