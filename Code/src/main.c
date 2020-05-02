#include "ch.h"
#include "hal.h"

#include <string.h>
#include "chprintf.h"
#include "shell.h"
#include "ew_shell.h"

#include "device_status.h"
#include "ew_audio.h"
#include "ew_display.h"
#include "ew_events.h"
#include "ew_statemachine.h"
#include "ew_time.h"


//===========================================================================
// Variables and local functions
//===========================================================================

ew_device_status_t device_status;

// calculate depending on current RTC timee, saved alarm times and their respective states (only look at enabled alarms and if they are set to snooze or not)
uint8_t findNextAlarm(void) {
    return 1;
}

ew_time_t findNextAlarmTime(void) {
    uint8_t next_alarm = findNextAlarm();
    ew_time_t ret = {
        device_status.alarms[next_alarm - 1].saved_time.hours, 
        device_status.alarms[next_alarm - 1].saved_time.minutes
    };
    return ret;
}

// hard coded values for now, read from eeprom later
static void retrieveDeviceStatus(void) {
    device_status.state = EW_INIT;

    device_status.alarms[0].saved_time.hours = 6;       // get alarm info from eeprom
    device_status.alarms[0].saved_time.minutes = 23;
    device_status.alarms[0].snooze_timer = 0;
    device_status.alarms[0].state = EW_ALARM_ENABLED;
    
    device_status.alarms[1].saved_time.hours = 8;
    device_status.alarms[1].saved_time.minutes = 41;
    device_status.alarms[1].snooze_timer = 0;
    device_status.alarms[1].state = EW_ALARM_ENABLED;

    device_status.next_alarm = findNextAlarm();
    device_status.next_alarm_time = findNextAlarmTime();    

    device_status.alarm_volume = 120;           // get these two from eeprom
    device_status.snooze_time = 10;

    device_status.dst_enabled = false;          // get this from RTC module
}

static void confirmationBuzzer(void) {
    palSetLine(LINE_BUZZER);
    chThdSleepMilliseconds(20);
    palClearLine(LINE_BUZZER);
    chThdSleepMilliseconds(50);
    palSetLine(LINE_BUZZER);
    chThdSleepMilliseconds(20);
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

static const ShellConfig shell_cfg = {
    (BaseSequentialStream *)&SD1,
    commands
};

//===========================================================================
// Driver configs
//===========================================================================

static const I2CConfig i2ccfg = {
    OPMODE_I2C,
    100000,
    STD_DUTY_CYCLE,
};

static uint8_t sd_scratchpad[512];      // Working area for SDC driver
static const SDCConfig sdccfg = {
  sd_scratchpad,
  SDC_MODE_1BIT
};

//===========================================================================
// Events and their callbacks
//===========================================================================

// External interrupts
event_source_t toggle_changed_event;
event_source_t lever_down_event;
event_source_t lever_up_event;
event_source_t encoder_changed_event;
event_source_t encoder_button_event;
event_source_t proximity_event;
event_source_t alarm_event;
event_source_t display_update_event;
event_source_t play_audio_event;

static void toggle_cb(void *arg) {
    (void)arg;
    chSysLockFromISR();
    chEvtBroadcastI(&toggle_changed_event);
    chSysUnlockFromISR();
}

static void lever_cb(void *arg) {
    (void)arg;
    chSysLockFromISR();
    if (palReadLine(LINE_LEVER) == LEVER_DOWN)
        chEvtBroadcastI(&lever_down_event);
    else
        chEvtBroadcastI(&lever_up_event);
    chSysUnlockFromISR();
}

static void encoder_button_cb(void *arg) {
    (void)arg;
    chSysLockFromISR();
    chEvtBroadcastI(&encoder_button_event);
    chSysUnlockFromISR();
}

static void proximity_interrupt_cb(void *arg) {
    (void)arg;
    chSysLockFromISR();
    chEvtBroadcastI(&proximity_event);
    chSysUnlockFromISR();
}

static void rtc_alarm_cb(void *arg) {
    (void)arg;
    chSysLockFromISR();
    // if alarm a (aka minute interrupt)
    chEvtBroadcastI(&display_update_event);     // set flags to signal minute interrupt to display thread
    // else (aka alarm b aka user alarm)
    chEvtBroadcastI(&user_alarm_event);
    chSysUnlockFromISR();
}

CH_IRQ_HANDLER(STM32_TIM4_HANDLER) {
    CH_IRQ_PROLOGUE();
    
    STM32_TIM4->SR = 0;         // clear all pending TIM4 interrupts
     
    chSysLockFromISR();
    if (STM32_TIM4->CR1 & TIM_CR1_DIR) palToggleLine(LINE_LED3);
    else palToggleLine(LINE_LED4);
    chEvtBroadcastI(&encoder_changed_event);
    chSysUnlockFromISR();
 
    CH_IRQ_EPILOGUE();  
}

//===========================================================================
// Application entry point
//===========================================================================

int main(void) {
    // System initialization
    // initializes GPIOs by calling __early_init() from board file
    halInit();
    chSysInit();
    confirmationBuzzer();

    bool restarting_after_standby = PWR->CSR & PWR_CSR_SBF;         // did we just wakeup from standby?
    PWR->CR |= PWR_CR_CSBF;                                         // clear standby flag

    chEvtObjectInit(&toggle_changed_event);
    chEvtObjectInit(&lever_down_event);
    chEvtObjectInit(&lever_up_event);
    chEvtObjectInit(&encoder_changed_event);
    chEvtObjectInit(&encoder_button_event);
    chEvtObjectInit(&proximity_event);
    chEvtObjectInit(&user_alarm_event);
    chEvtObjectInit(&display_update_event);
    chEvtObjectInit(&play_audio_event);

    // retrieve device status from eeprom
    retrieveDeviceStatus();

    // initialize RTC unit, and assign callback functions (next alarm and minute wakeup). Only reset alarm when restarting after complete power down.

    // start all needed peripheral drivers
    i2cStart(&I2CD2, &i2ccfg);
    sdcStart(&SDCD1, &sdccfg);    
    sdStart(&SD1, NULL);

    // start threads
    chThdCreateStatic(blinker_wa, sizeof(blinker_wa), NORMALPRIO, blinkerThd, NULL);
    //chThdCreateStatic(audio_wa, sizeof(audio_wa), NORMALPRIO + 1, audioThd, NULL);
    chThdCreateStatic(display_wa, sizeof(display_wa), NORMALPRIO, displayThd, NULL);

    // initialize timer for rotary encoder and assign callback function
    rccEnableTIM4(true);
    STM32_TIM4->SMCR = 3;          // encoder mode 3
    STM32_TIM4->CCER = 0;          // rising edge polarity
    STM32_TIM4->ARR = 0x1;         // count between 0 and 1 -> update interrupt every two impulses, matches encoder
    STM32_TIM4->CCMR1 = 0xc1c1;    // f_DTS/16, N=8 IC1->TI1, IC2->TI2
    STM32_TIM4->CNT = 0;           // initialize counter
    STM32_TIM4->EGR = 1;           // generate an update event, clearing the counter
    STM32_TIM4->CR1 = 1;           // enable the counter
    STM32_TIM4->DIER = 1;          // enable update interrupt
    STM32_TIM4->SR = 0;            // clear all pending interrupts
    nvicEnableVector(STM32_TIM4_NUMBER, 7);         // enable TIM4 interrupt vector with priotity 7 = default priority from mcuconf.h (i)f GPT was used)

    // activate external interrupts for lever, toggle, buttons and proximity interrupt and assign callback functions
    palEnableLineEvent(LINE_TOGGLE_UP, PAL_EVENT_MODE_BOTH_EDGES);
    palEnableLineEvent(LINE_TOGGLE_DN, PAL_EVENT_MODE_BOTH_EDGES);
    palSetLineCallback(LINE_TOGGLE_UP, toggle_cb, NULL);
    palSetLineCallback(LINE_TOGGLE_DN, toggle_cb, NULL);

    palEnableLineEvent(LINE_LEVER, PAL_EVENT_MODE_BOTH_EDGES);
    palSetLineCallback(LINE_LEVER, lever_cb, NULL);

    palEnableLineEvent(LINE_I2C_INT, PAL_EVENT_MODE_FALLING_EDGE);
    palSetLineCallback(LINE_I2C_INT, proximity_interrupt_cb, NULL);

    // activate event listeners for main thread


    while (1) {
        thread_t *shelltp = chThdCreateFromHeap(NULL, SHELL_WA_SIZE, "shell", NORMALPRIO + 1, shellThread, (void *)&shell_cfg);
        chThdWait(shelltp);
        chThdSleepMilliseconds(1000);
    }

/*
    uint16_t alarm_flags = (RTC->ISR & (RTC_ISR_ALRAF | RTC_ISR_ALRBF)) >> RTC_ISR_ALRAF_Pos;   // yields 1 for alarm A flag, 2 for alarm B flag, 3 for both
    if (alarm_flags > 0) current_state = enterAlarmRinging(alarm_flags);
    else current_state = enterIdle();

    while (true) {
        eventmask_t new_event = chEvtWaitOne(ALL_EVENTS);
        current_state = handleEvent(new_event, current_state);
    }
    */

}  // end of main()
