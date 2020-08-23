#include <string.h>

#include "ch.h"
#include "hal.h"

#include "chprintf.h"
#include "shell.h"

#include "ew_devicestatus.h"
#include "ew_audio.h"
#include "ew_display.h"
#include "ew_events.h"
#include "ew_shell.h"
#include "ew_statemachine.h"
#include "ew_time.h"

//===========================================================================
// Variables and local functions
//===========================================================================

// Event listener for main thread statemachine.
event_source_t statemachine_event;
event_listener_t el_statemachine;

// Short startup beep
static void startupBuzzer(void) {
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
// Command line configuration
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
// Global driver configuration (drivers that are used in multiple threads
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

static void toggle_cb(void *arg) {
    (void)arg;
    chSysLockFromISR();
    chEvtBroadcastFlagsI(&statemachine_event, EVENT_MASK(EW_TOGGLE_CHANGE));
    chSysUnlockFromISR();
}

static void lever_cb(void *arg) {
    (void)arg;
    chSysLockFromISR();
    // Lever event has two seperate events for falling / rising edge. This could technically be handled by two seperate callback functions.
    if (palReadLine(LINE_LEVER) == LEVER_DOWN)
        chEvtBroadcastFlagsI(&statemachine_event, EVENT_MASK(EW_LEVER_DOWN));
    else
        chEvtBroadcastFlagsI(&statemachine_event, EVENT_MASK(EW_LEVER_UP));
    chSysUnlockFromISR();
}

static void encoder_button_cb(void *arg) {
    (void)arg;
    chSysLockFromISR();
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
    // if RTC alarm A (minute interrupt)
    chEvtBroadcastFlagsI(&display_event, EVENT_MASK(EW_DISPLAY_REFRESH));       // signal minute interrupt to display thread
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
        chEvtBroadcastFlagsI(&statemachine_event, EVENT_MASK(EVENT_FLAGS_OFFSET + EW_ENCODER_CHANGED) | EVENT_MASK(EW_ENCODER_CHANGED));
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
    startupBuzzer();

    bool restart_after_standby = PWR->CSR & PWR_CSR_SBF;     // did the device just wake up from standby?
    PWR->CR |= PWR_CR_CSBF;                                     // clear standby flag

    // init event source for statemachine
    chEvtObjectInit(&statemachine_event);

    // register this event source as event 0 at our listener
    chEvtRegister(&statemachine_event, &el_statemachine, 0);

    // retrieve device status from eeprom and RTC backup registers
    retrieveDeviceStatus();

    // initialize RTC unit and assign callback functions (next alarm and minute wakeup)

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
        uint16_t alarm_flags = (RTC->ISR & (RTC_ISR_ALRAF | RTC_ISR_ALRBF)) >> RTC_ISR_ALRAF_Pos;
        if (alarm_flags > 0) current_state = 
        device_status.state = enterAlarmRinging();
        */
    }
    // if not restarting from standby, power was gone so RTC unit needs to be re-initialized
    else {
        ew_time_t temp = {0, 0};
        initRTC(temp);
        device_status.state = enterIdle(EW_TIMEOUT_LONG);
    }

    // wait for an event to occur
    while (true) {
        eventmask_t new_event = chEvtWaitAny(0);
        // retrieve event flags - check ew_event.h for more information on how events and event flags are used in this project.
        eventflags_t flags = chEvtGetAndClearFlags(&el_statemachine);
        handleEvent((uint16_t)(flags & 0xffff), (uint16_t)((flags >> EVENT_FLAGS_OFFSET) & 0xffff));
    }    

}  // end of main()
