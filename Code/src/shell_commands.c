#include "ch.h"
#include "hal.h"
#include <string.h>
#include "shell.h"
#include "chprintf.h"
#include "ff.h"
#include "stdlib.h"

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


// SD test command
void cmd_sdc(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *mode[] = {"SDV11", "SDV20", "MMC", NULL};

  (void)argv;
 
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


// FatFS tree command
void cmd_tree(BaseSequentialStream *chp, int argc, char *argv[]) {
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


// FatFS create command
void cmd_create(BaseSequentialStream *chp, int argc, char *argv[]) {
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


// I2C test command
void cmd_i2c(BaseSequentialStream *chp, int argc, char *argv[]) {

    uint16_t start_address = 0x00;
    uint16_t end_address = 0xff;

    if (argc == 1) {
        start_address = (uint16_t)atoi(argv[0]);
        end_address = start_address;
    }

    if (argc == 2) {
        start_address = (uint16_t)atoi(argv[0]);
        end_address = (uint16_t)atoi(argv[1]);
    }

    else if (argc > 2) {
        chprintf(chp, "Usage: i2c [start address] [end address]\r\n");
        return;
    }

    chprintf(chp, "Checking I2C bus for addresses from %d to %d.\r\n", start_address, end_address);

    uint8_t test_buf[] = {0};

    i2cAcquireBus(&I2CD2);
    for (uint16_t a = start_address; a <= end_address; a++) {
        chprintf(chp, "Trying address %d ... ", a);
        msg_t res = i2cMasterTransmitTimeout(&I2CD2,
                               a,
                               test_buf,
                               1,
                               NULL,
                               0,
                               TIME_MS2I(10));
        if (res == MSG_OK) chprintf(chp, "success!\r\n");
        else chprintf(chp, "failed: %x\r\n", i2cGetErrors(&I2CD2));

    }
    i2cReleaseBus(&I2CD2);

}

// Hello world command
void cmd_write(BaseSequentialStream *chp, int argc, char *argv[]) {

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: hello\r\n");
    return;
  }

  chprintf(chp, "Hello world!\r\n");
}
