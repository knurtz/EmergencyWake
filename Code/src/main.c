#include "ch.h"
#include "hal.h"

#include <string.h>
#include "chprintf.h"

#include "shell.h"
#include "shell_commands.h"

#include "usbcfg.h"

//===========================================================================
// Thread functions                                                         
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
// I2C related                                                         
//===========================================================================

static const I2CConfig i2ccfg = {
    OPMODE_I2C,
    100000,
    STD_DUTY_CYCLE,
};

//===========================================================================
// SD Card related.                                                    
//===========================================================================

// Working area for driver.
static uint8_t sd_scratchpad[512];

// SDIO configuration.
static const SDCConfig sdccfg = {
  sd_scratchpad,
  SDC_MODE_1BIT
};


//===========================================================================
// Command line related                                                
//===========================================================================

#define SHELL_WA_SIZE           THD_WORKING_AREA_SIZE(2048)

static const ShellCommand commands[] = {
  {"tree", cmd_tree},
  {"create", cmd_create},
  {"sdc", cmd_sdc},
  {"hello", cmd_write},
  {"i2c", cmd_i2c},
  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU1,
  commands
};

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

  // Initialize I2C2 driver used for proximity sensor and audio codec
  i2cStart(&I2CD2, &i2ccfg);

  // Initialize shell
  shellInit();

  // Initialize SD card driver
  sdcStart(&SDCD1, &sdccfg);

  // Create the blinker thread
  chThdCreateStatic(waBlinker, sizeof(waBlinker), NORMALPRIO, Blinker, NULL);

  while (true) {

    if (SDU1.config->usbp->state == USB_ACTIVE) {
      thread_t *shelltp = chThdCreateFromHeap(NULL, SHELL_WA_SIZE,
                                              "shell", NORMALPRIO + 1,
                                              shellThread, (void *)&shell_cfg1);
      chThdWait(shelltp);       // wait for termination of shell thread
    }

    chThdSleepMilliseconds(1000);

  }

} // end of main()
