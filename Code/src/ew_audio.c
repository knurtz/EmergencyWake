/*
Functions for everything audio and SD card related. Involves controlling the SDIO driver, communicating with the audio codec IC, 
reading files from the SD card, and providing functions for other threads to start and stop playback of certain files.

*/

#include "ch.h"
#include "hal.h"

//===========================================================================
// SD Card related.                                                    
//===========================================================================

// Working area for driver.
static uint8_t sd_scratchpad[512];

void playSnooze();
void playDisabled();
void playRingtone();
void stopRingtone();

// SDIO configuration.
static const SDCConfig sdccfg = {
  sd_scratchpad,
  SDC_MODE_1BIT
};


void audio_init(void) {
  // Initialize SD card driver
  sdcStart(&SDCD1, &sdccfg);
  
}