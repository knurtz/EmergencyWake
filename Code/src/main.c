#include "ch.h"
#include "hal.h"

#include "usbcfg.h"

/*
 * This is a periodic thread that does absolutely nothing except flashing
 * a LED.
 */
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

/*
 * Application entry point.
 */
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

  chThdSleepMilliseconds(1000);

  // Initializes a serial-over-USB CDC driver.
  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);

  // Activates the USB driver and then the USB bus pull-up on D+.
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1000);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

  while (true) {

    if (palReadLine(LINE_DISCO_BUTTON)) {
      palToggleLine(LINE_DISCO_LED3);
      static uint8_t buf[] = "Hello world!";
      chnWrite(&SDU1, buf, sizeof buf - 1);
    }

    chThdSleepMilliseconds(500);

  }

} /* end of main() */
