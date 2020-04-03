#include "ch.h"
#include "hal.h"

#include <string.h>
#include "shell.h"
#include "chprintf.h"

#include "usbcfg.h"

#include "ff.h"

//===========================================================================
// Thread functions.                                                         
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
// FatFs related.                                                            
//===========================================================================

// FS object
static FATFS SDC_FS;

// FS mounted and ready
static bool fs_ready = FALSE;

// Generic large buffer
static uint8_t fbuff[1024];

static FRESULT scan_files(BaseSequentialStream *chp, char *path) {
  static FILINFO fno;
  FRESULT res;
  DIR dir;
  size_t i;
  char *fn;

  res = f_opendir(&dir, path);
  if (res == FR_OK) {
    i = strlen(path);
    while (((res = f_readdir(&dir, &fno)) == FR_OK) && fno.fname[0]) {
      if (FF_FS_RPATH && fno.fname[0] == '.')
        continue;
      fn = fno.fname;
      if (fno.fattrib & AM_DIR) {
        *(path + i) = '/';
        strcpy(path + i + 1, fn);
        res = scan_files(chp, path);
        *(path + i) = '\0';
        if (res != FR_OK)
          break;
      }
      else {
        chprintf(chp, "%s/%s\r\n", path, fn);
      }
    }
  }
  return res;
}

//===========================================================================
// Command line related.                                                   
//===========================================================================

#define SHELL_WA_SIZE           THD_WORKING_AREA_SIZE(2048)

// SD test command
void cmd_sdc(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *mode[] = {"SDV11", "SDV20", "MMC", NULL};
 
  if (argc > 0) {
    chprintf(chp, "Usage: sdc\r\n");
    return;
  }

  // Card presence check
  if (!blkIsInserted(&SDCD1)) {
    chprintf(chp, "Card not inserted, aborting.\r\n");
    return;
  }

  // Connection to the card
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
    
  // Card disconnect and command end
  sdcDisconnect(&SDCD1);
}

// tree command
static void cmd_tree(BaseSequentialStream *chp, int argc, char *argv[]) {
  FRESULT err;
  uint32_t fre_clust;
  FATFS *fsp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: tree\r\n");
    return;
  }

  // Connection to the card
  chprintf(chp, "Connecting... ");
  if (sdcConnect(&SDCD1)) {
    chprintf(chp, "failed\r\n");
    return;
  }

  // mount file system
  err = f_mount(&SDC_FS, "/", 1);
  if (err != FR_OK) {
    sdcDisconnect(&SDCD1);
    return;
  }
  fs_ready = TRUE;

  if (!fs_ready) {
    chprintf(chp, "File System not mounted\r\n");
    return;
  }

  err = f_getfree("/", &fre_clust, &fsp);
  if (err != FR_OK) {
    chprintf(chp, "FS: f_getfree() failed\r\n");
    return;
  }
  chprintf(chp,
           "FS: %lu free clusters with %lu sectors (%lu bytes) per cluster\r\n",
           fre_clust, (uint32_t)fsp->csize, (uint32_t)fsp->csize * 512);
  fbuff[0] = 0;
  scan_files(chp, (char *)fbuff);

  sdcDisconnect(&SDCD1);
  fs_ready = FALSE;
}

// create command
static void cmd_create(BaseSequentialStream *chp, int argc, char *argv[]) {
  FRESULT err;
  FIL f;
  static const char data[] = "the quick brown fox jumps over the lazy dog";
  UINT btw = sizeof data - 1;
  UINT bw;

  if (argc != 1) {
    chprintf(chp, "Usage: create <filename>\r\n");
    return;
  }

  // Connection to the card
  chprintf(chp, "Connecting... ");
  if (sdcConnect(&SDCD1)) {
    chprintf(chp, "failed\r\n");
    return;
  }

  // mount file system
  err = f_mount(&SDC_FS, "/", 1);
  if (err != FR_OK) {
    sdcDisconnect(&SDCD1);
    return;
  }
  fs_ready = TRUE;

  if (!fs_ready) {
    chprintf(chp, "File System not mounted\r\n");
    return;
  }

  err = f_open(&f, (const TCHAR *)argv[0], FA_CREATE_ALWAYS | FA_WRITE);
  if (err != FR_OK) {
    chprintf(chp, "FS: f_open() failed\r\n");
    return;
  }

  err = f_write(&f, (const void *)data, btw, &bw);
  if (err != FR_OK) {
    chprintf(chp, "FS: f_write() failed\r\n");
  }

  err = f_close(&f);
  if (err != FR_OK) {
    chprintf(chp, "FS: f_close() failed\r\n");
    return;
  }

  sdcDisconnect(&SDCD1);
  fs_ready = FALSE;
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
  {"tree", cmd_tree},
  {"create", cmd_create},
  {"sdc", cmd_sdc},
  {"hello", cmd_write},
  {NULL, NULL}
};

static const ShellConfig shell_cfg1 = {
  (BaseSequentialStream *)&SDU1,
  commands
};

//===========================================================================
// Application entry point.                                                  
//===========================================================================

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

} // end of main()
