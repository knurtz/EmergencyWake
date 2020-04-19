#include "ch.h"
#include "hal.h"

#include <string.h>
#include "chprintf.h"
#include "usbcfg.h"
#include "shell.h"
#include "ew_shell.h"

#include "ew_time.h"
#include "ew_statemachine.h"

#define USB_DEBUG

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

static THD_WORKING_AREA(wa_blinker, 128);
static THD_FUNCTION(blinker, arg) {
    (void)arg;
    chRegSetThreadName("blinker");

    while (true) {
        palSetLine(LINE_DISCO_LED1);
        chThdSleepMilliseconds(100);
        palSetLine(LINE_DISCO_LED2);
        chThdSleepMilliseconds(500);
        palClearLine(LINE_DISCO_LED1);
        chThdSleepMilliseconds(100);
        palClearLine(LINE_DISCO_LED2);
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
    {NULL, NULL}};

static const ShellConfig shell_cfg1 = {
    (BaseSequentialStream *)&SDU1,
    commands};

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

    // start blinker thread
    chThdCreateStatic(wa_blinker, sizeof(wa_blinker), NORMALPRIO, blinker, NULL);

    // start display, audio and proximity threads

    // initialize timer for rotary encoder and assign callback function

    // activate external interrupts for lever, toggle, buttons and proximity interrupt and assign callback functions

    // activate event listeners for main thread

#ifdef USB_DEBUG

    // Initialize a serial-over-USB CDC driver
    sduObjectInit(&SDU1);
    sduStart(&SDU1, &serusbcfg);

    // Activate the USB driver and pull-up on D+ by connecting the bus
    usbDisconnectBus(serusbcfg.usbp);
    chThdSleepMilliseconds(1000);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);

    // Initialize shell
    shellInit();

    // Create shell thread
    if (SDU1.config->usbp->state == USB_ACTIVE) chThdCreateFromHeap(NULL, SHELL_WA_SIZE, "shell", NORMALPRIO + 1, shellThread, (void *)&shell_cfg1);

#endif

    uint16_t alarm_flags = (RTC->ISR & (RTC_ISR_ALRAF | RTC_ISR_ALRBF)) >> RTC_ISR_ALRAF_Pos;   // yields 1 for alarm A flag, 2 for alarm B flag, 3 for both
    if (alarm_flags > 0) current_state = enterAlarmRinging(alarm_flags);
    else current_state = enterIdle();

    while (true) {
        eventmask_t new_event = chEvtWaitOne(ALL_EVENTS);
        current_state = handleEvent(new_event, current_state);
    }

}  // end of main()
