#include "ch.h"
#include "hal.h"

#include <string.h>
#include "chprintf.h"
#include "shell.h"
#include "ew_shell.h"

#include "ew_audio.h"

#include "ew_time.h"
#include "ew_statemachine.h"


//===========================================================================
// Global variables
//===========================================================================

ew_state_t current_state = EW_STARTUP;

ew_time_t current_time = {0, 0, false, false};
ew_time_t alarm_a = {0, 0, false, false};
ew_time_t alarm_b = {0, 0, false, false};


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
    {"hello", cmd_write},
    {"i2c", cmd_i2c},
    {NULL, NULL}
};

static const ShellConfig shell_cfg = {
    (BaseSequentialStream *)&SD1,
    commands
};

//===========================================================================
// Driver configurations
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
// Application entry point
//===========================================================================

int main(void) {

    current_state = EW_INIT;

    // System initialization
    // initializes GPIOs by calling __early_init() from board file
    halInit();
    chSysInit();

    // initialize RTC unit, and assign callback functions (alarms and periodic wakeup once a minute), if reset after power loss: load alarms from eeprom

    // start all nedded peripheral drivers
    i2cStart(&I2CD2, &i2ccfg);
    //sdcStart(&SDCD1, &sdccfg);

    // start threads
    chThdCreateStatic(blinker_wa, sizeof(blinker_wa), NORMALPRIO, blinkerThd, NULL);
    chThdCreateStatic(audio_wa, sizeof(audio_wa), NORMALPRIO + 1, audioThd, NULL);

    // initialize timer for rotary encoder and assign callback function

    // activate external interrupts for lever, toggle, buttons and proximity interrupt and assign callback functions

    // activate event listeners for main thread

/*
    palSetLine(LINE_SD_EN);
*/
    
    sdStart(&SD1, NULL);

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
