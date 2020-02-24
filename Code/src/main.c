#include "ch.h"
#include "hal.h"

#include <string.h>
#include "shell.h"
#include "chprintf.h"

#include "usbcfg.h"

/*===========================================================================*/
/* Thread functions.                                                         */
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
/* SD Card related.                                                     */
/*===========================================================================*/

/*
 * Working area for driver.
 */
static uint8_t sd_scratchpad[512];

/*
 * SDIO configuration.
 */
static const SDCConfig sdccfg = {
  sd_scratchpad,
  SDC_MODE_1BIT
};

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

#define SHELL_WA_SIZE           THD_WORKING_AREA_SIZE(2048)
#define SDC_BURST_SIZE          16

// SD test command
/* Buffer for block read/write operations, note that extra bytes are
   allocated in order to support unaligned operations.*/
static uint8_t buf[MMCSD_BLOCK_SIZE * SDC_BURST_SIZE + 4];

/* Additional buffer for sdcErase() test */
static uint8_t buf2[MMCSD_BLOCK_SIZE * SDC_BURST_SIZE ];

void cmd_sdc(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *mode[] = {"SDV11", "SDV20", "MMC", NULL};
  systime_t start, end;
  uint32_t n, startblk;

  if (argc != 1) {
    chprintf(chp, "Usage: sdiotest read|write|erase|all\r\n");
    return;
  }

  /* Card presence check.*/
  if (!blkIsInserted(&SDCD1)) {
    chprintf(chp, "Card not inserted, aborting.\r\n");
    return;
  }

  /* Connection to the card.*/
  chprintf(chp, "Connecting... ");
  if (sdcConnect(&SDCD1)) {
    chprintf(chp, "failed\r\n");
    return;
  }

  chprintf(chp, "OK\r\n\r\nCard Info\r\n");
  chprintf(chp, "CSD      : %08X %8X %08X %08X \r\n",
           SDCD1.csd[3], SDCD1.csd[2], SDCD1.csd[1], SDCD1.csd[0]);
  chprintf(chp, "CID      : %08X %8X %08X %08X \r\n",
           SDCD1.cid[3], SDCD1.cid[2], SDCD1.cid[1], SDCD1.cid[0]);
  chprintf(chp, "Mode     : %s\r\n", mode[SDCD1.cardmode & 3U]);
  chprintf(chp, "Capacity : %DMB\r\n", SDCD1.capacity / 2048);

  /* The test is performed in the middle of the flash area.*/
  startblk = (SDCD1.capacity / MMCSD_BLOCK_SIZE) / 2;

  if ((strcmp(argv[0], "read") == 0) ||
      (strcmp(argv[0], "all") == 0)) {

    /* Single block read performance, aligned.*/
    chprintf(chp, "Single block aligned read performance:           ");
    start = chVTGetSystemTime();
    end = chTimeAddX(start, TIME_MS2I(1000));
    n = 0;
    do {
      if (blkRead(&SDCD1, startblk, buf, 1)) {
        chprintf(chp, "failed\r\n");
        goto exittest;
      }
      n++;
    } while (chVTIsSystemTimeWithin(start, end));
    chprintf(chp, "%D blocks/S, %D bytes/S\r\n", n, n * MMCSD_BLOCK_SIZE);

    /* Multiple sequential blocks read performance, aligned.*/
    chprintf(chp, "16 sequential blocks aligned read performance:   ");
    start = chVTGetSystemTime();
    end = chTimeAddX(start, TIME_MS2I(1000));
    n = 0;
    do {
      if (blkRead(&SDCD1, startblk, buf, SDC_BURST_SIZE)) {
        chprintf(chp, "failed\r\n");
        goto exittest;
      }
      n += SDC_BURST_SIZE;
    } while (chVTIsSystemTimeWithin(start, end));
    chprintf(chp, "%D blocks/S, %D bytes/S\r\n", n, n * MMCSD_BLOCK_SIZE);

#if STM32_SDC_SDIO_UNALIGNED_SUPPORT
    /* Single block read performance, unaligned.*/
    chprintf(chp, "Single block unaligned read performance:         ");
    start = chVTGetSystemTime();
    end = chTimeAddX(start, TIME_MS2I(1000));
    n = 0;
    do {
      if (blkRead(&SDCD1, startblk, buf + 1, 1)) {
        chprintf(chp, "failed\r\n");
        goto exittest;
      }
      n++;
    } while (chVTIsSystemTimeWithin(start, end));
    chprintf(chp, "%D blocks/S, %D bytes/S\r\n", n, n * MMCSD_BLOCK_SIZE);

    /* Multiple sequential blocks read performance, unaligned.*/
    chprintf(chp, "16 sequential blocks unaligned read performance: ");
    start = chVTGetSystemTime();
    end = chTimeAddX(start, TIME_MS2I(1000));
    n = 0;
    do {
      if (blkRead(&SDCD1, startblk, buf + 1, SDC_BURST_SIZE)) {
        chprintf(chp, "failed\r\n");
        goto exittest;
      }
      n += SDC_BURST_SIZE;
    } while (chVTIsSystemTimeWithin(start, end));
    chprintf(chp, "%D blocks/S, %D bytes/S\r\n", n, n * MMCSD_BLOCK_SIZE);
#endif /* STM32_SDC_SDIO_UNALIGNED_SUPPORT */
  }

  if ((strcmp(argv[0], "write") == 0) ||
      (strcmp(argv[0], "all") == 0)) {
    unsigned i;

    memset(buf, 0xAA, MMCSD_BLOCK_SIZE * 2);
    chprintf(chp, "Writing...");
    if(sdcWrite(&SDCD1, startblk, buf, 2)) {
      chprintf(chp, "failed\r\n");
      goto exittest;
    }
    chprintf(chp, "OK\r\n");

    memset(buf, 0x55, MMCSD_BLOCK_SIZE * 2);
    chprintf(chp, "Reading...");
    if (blkRead(&SDCD1, startblk, buf, 1)) {
      chprintf(chp, "failed\r\n");
      goto exittest;
    }
    chprintf(chp, "OK\r\n");

    for (i = 0; i < MMCSD_BLOCK_SIZE; i++)
      buf[i] = i + 8;
    chprintf(chp, "Writing...");
    if(sdcWrite(&SDCD1, startblk, buf, 2)) {
      chprintf(chp, "failed\r\n");
      goto exittest;
    }
    chprintf(chp, "OK\r\n");

    memset(buf, 0, MMCSD_BLOCK_SIZE * 2);
    chprintf(chp, "Reading...");
    if (blkRead(&SDCD1, startblk, buf, 1)) {
      chprintf(chp, "failed\r\n");
      goto exittest;
    }
    chprintf(chp, "OK\r\n");
  }

  if ((strcmp(argv[0], "erase") == 0) ||
      (strcmp(argv[0], "all") == 0)) {
    /**
     * Test sdcErase()
     * Strategy:
     *   1. Fill two blocks with non-constant data
     *   2. Write two blocks starting at startblk
     *   3. Erase the second of the two blocks
     *      3.1. First block should be equal to the data written
     *      3.2. Second block should NOT be equal too the data written (i.e. erased).
     *   4. Erase both first and second block
     *      4.1 Both blocks should not be equal to the data initially written
     * Precondition: SDC_BURST_SIZE >= 2
     */
    memset(buf, 0, MMCSD_BLOCK_SIZE * 2);
    memset(buf2, 0, MMCSD_BLOCK_SIZE * 2);
    /* 1. */
    unsigned int i = 0;
    for (; i < MMCSD_BLOCK_SIZE * 2; ++i) {
      buf[i] = (i + 7) % 'T'; //Ensure block 1/2 are not equal
    }
    /* 2. */
    if(sdcWrite(&SDCD1, startblk, buf, 2)) {
      chprintf(chp, "sdcErase() test write failed\r\n");
      goto exittest;
    }
    /* 3. (erase) */
    if(sdcErase(&SDCD1, startblk + 1, startblk + 2)) {
      chprintf(chp, "sdcErase() failed\r\n");
      goto exittest;
    }
    sdcflags_t errflags = sdcGetAndClearErrors(&SDCD1);
    if(errflags) {
      chprintf(chp, "sdcErase() yielded error flags: %d\r\n", errflags);
      goto exittest;
    }
    if(sdcRead(&SDCD1, startblk, buf2, 2)) {
      chprintf(chp, "single-block sdcErase() failed\r\n");
      goto exittest;
    }
    /* 3.1. */
    if(memcmp(buf, buf2, MMCSD_BLOCK_SIZE) != 0) {
      chprintf(chp, "sdcErase() non-erased block compare failed\r\n");
      goto exittest;
    }
    /* 3.2. */
    if(memcmp(buf + MMCSD_BLOCK_SIZE,
              buf2 + MMCSD_BLOCK_SIZE, MMCSD_BLOCK_SIZE) == 0) {
      chprintf(chp, "sdcErase() erased block compare failed\r\n");
      goto exittest;
    }
    /* 4. */
    if(sdcErase(&SDCD1, startblk, startblk + 2)) {
      chprintf(chp, "multi-block sdcErase() failed\r\n");
      goto exittest;
    }
    if(sdcRead(&SDCD1, startblk, buf2, 2)) {
      chprintf(chp, "single-block sdcErase() failed\r\n");
      goto exittest;
    }
    /* 4.1 */
    if(memcmp(buf, buf2, MMCSD_BLOCK_SIZE) == 0) {
      chprintf(chp, "multi-block sdcErase() erased block compare failed\r\n");
      goto exittest;
    }
    if(memcmp(buf + MMCSD_BLOCK_SIZE,
              buf2 + MMCSD_BLOCK_SIZE, MMCSD_BLOCK_SIZE) == 0) {
      chprintf(chp, "multi-block sdcErase() erased block compare failed\r\n");
      goto exittest;
    }
    /* END of sdcErase() test */
  }
  
  /* Card disconnect and command end.*/
exittest:
  sdcDisconnect(&SDCD1);
}

// Hello world command
static void cmd_write(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: hello\r\n");
    return;
  }

  chprintf(chp, "Hello world!\r\n");
}

static const ShellCommand commands[] = {
  {"sdc", cmd_sdc},
  {"hello", cmd_write},
  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU1,
  commands
};

/*===========================================================================*/
/* Application entry point.                                                  */
/*===========================================================================*/

int main(void) {

  // System initialization
  halInit();
  chSysInit();

  // Initialize some GPIOs
  palSetLineMode(LINE_DISCO_BUTTON, PAL_MODE_INPUT);

  palSetLineMode(LINE_DISCO_LED1, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_DISCO_LED2, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_DISCO_LED3, PAL_MODE_OUTPUT_PUSHPULL);
  palSetLineMode(LINE_DISCO_LED4, PAL_MODE_OUTPUT_PUSHPULL);

  palSetPadMode(GPIOA, GPIOA_USB_DM, PAL_MODE_ALTERNATE(10));     // USB FS DM
  palSetPadMode(GPIOA, GPIOA_USB_DP, PAL_MODE_ALTERNATE(10));     // USB FS DP

  palSetPadMode(GPIOD, 2, PAL_STM32_MODE_ALTERNATE | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_ALTERNATE(12));    // SDIO CMD
  palSetPadMode(GPIOC, 12, PAL_STM32_MODE_ALTERNATE | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_ALTERNATE(12));   // SDIO CK
  palSetPadMode(GPIOC, 8, PAL_STM32_MODE_ALTERNATE | PAL_STM32_OTYPE_PUSHPULL | PAL_STM32_OSPEED_HIGHEST | PAL_STM32_ALTERNATE(12));    // SDIO D0

  // Initialize a serial-over-USB CDC driver
  sduObjectInit(&SDU1);
  sduStart(&SDU1, &serusbcfg);

  // Activate the USB driver and then the USB bus pull-up on D+
  usbDisconnectBus(serusbcfg.usbp);
  chThdSleepMilliseconds(1000);
  usbStart(serusbcfg.usbp, &usbcfg);
  usbConnectBus(serusbcfg.usbp);

  shellInit();

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
    //chprintf((BaseSequentialStream *)&SDU1, "Hello world!");

  }

} /* end of main() */
