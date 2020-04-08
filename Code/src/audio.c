#include "ch.h"
#include "hal.h"

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


void audio_init(void) {
  // Initialize SD card driver
  sdcStart(&SDCD1, &sdccfg);
  
}