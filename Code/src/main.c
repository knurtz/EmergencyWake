#include "ch.h"
#include "hal.h"

#include "shell.h"
#include "chprintf.h"

#include "usbcfg.h"

/*===========================================================================*/
/* Thread functions   .                                                      */
/*===========================================================================*/

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

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)

static void cmd_write(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: write\r\n");
    return;
  }

  chprintf(chp, "Hello world!");
}

static const ShellCommand commands[] = {
  {"hello", cmd_write},
  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU1,
  commands
};

/*===========================================================================*/
/* Application entry point                                                   */
/*===========================================================================*/

int main(void) {

  // System initializations
  halInit();
  chSysInit();

  // Initializes some GPIOs.
  palSetLineMode(LINE_DISCO_BUTTON, PAL_MODE_INPUT);

  palSetLineMode(LINE_DISCO_LED1, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_DISCO_LED2, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_DISCO_LED3, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_DISCO_LED4, PAL_MODE_OUTPUT_PUSHPULL);

  palSetPadMode(GPIOA, GPIOA_USB_DM, PAL_MODE_ALTERNATE(10));
  palSetPadMode(GPIOA, GPIOA_USB_DP, PAL_MODE_ALTERNATE(10));

  // Creates the blinker thread.
  chThdCreateStatic(waBlinker, sizeof(waBlinker), NORMALPRIO, Blinker, NULL);

  // Initializes a serial-over-USB CDC driver.
  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);

  // Activates the USB driver and then the USB bus pull-up on D+.
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1000);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

  shellInit();

  while (true) {

    if (SDU1.config->usbp->state == USB_ACTIVE) {
      thread_t *shelltp = chThdCreateFromHeap(NULL, SHELL_WA_SIZE,
                                              "shell", NORMALPRIO + 1,
                                              shellThread, (void *)&shell_cfg1);
      chThdWait(shelltp);       // wait for termination of shell thread
    }

    chThdSleepMilliseconds(1000);
    //chprintf((BaseSequentialStream *)&SDU1, "Hello world!");

  }

} /* end of main() */
