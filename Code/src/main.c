#include <string.h>

#include "ch.h"
#include "chprintf.h"
#include "hal.h"
#include "shell.h"
#include "shell_commands.h"
#include "statemachine.h"
#include "usbcfg.h"

//===========================================================================
// Blinker thread
//===========================================================================

static THD_WORKING_AREA(waBlinker, 128);
static THD_FUNCTION(Blinker, arg) {
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
    // System initialization, also initializes GPIOs by calling __early_init();
    halInit();
    chSysInit();

    // Initialize a serial-over-USB CDC driver
    sduObjectInit(&SDU1);
    sduStart(&SDU1, &serusbcfg);

    // Activate the USB driver and then the USB bus pull-up on D+
    usbDisconnectBus(serusbcfg.usbp);
    chThdSleepMilliseconds(1000);
    usbStart(serusbcfg.usbp, &usbcfg);
    usbConnectBus(serusbcfg.usbp);

    // Initialize shell
    shellInit();

    // Create the blinker thread
    chThdCreateStatic(waBlinker, sizeof(waBlinker), NORMALPRIO, Blinker, NULL);
    if (SDU1.config->usbp->state == USB_ACTIVE) chThdCreateFromHeap(NULL, SHELL_WA_SIZE,
                                                                    "shell", NORMALPRIO + 1,
                                                                    shellThread, (void *)&shell_cfg1);

    ew_state_t current_state = EW_STARTUP;

    while (true) {
        eventid_t new_event;  // = wait for any event

        current_state = handle_event(new_event, current_state);

    }

}  // end of main()
