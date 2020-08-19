/*
Functions for everything audio and SD card related. Involves controlling the SDIO driver, communicating with the audio codec IC, 
reading files from the SD card, and providing functions for other threads to start and stop playback of certain files.

*/

#include "ch.h"
#include "hal.h"
#include "ff.h"
#include <string.h>
#include "chprintf.h"
#include "wm8960.h"
#include "wave_header.h"
#include "device_status.h"


#define WAV_BUFFER_SIZE 512

#define FORMAT_PCM		1
#define RIFF			0x46464952  // FFIR
#define WAVE			0x45564157  // EVAW 
#define DATA			0x61746164  // atad
#define FMT				0x20746D66  //  tmf

#define EVT_DAC_TC			     0	// DAC transmission complete
#define EVT_DAC_HT			     1	// DAC transmission half complete


static int16_t dac_buffer[WAV_BUFFER_SIZE];

static FATFS SDC_FS;
static FIL file;
static bool fs_ready = false;

static uint32_t bytes_to_play;

static event_source_t audio_dma_tc_event;
static event_source_t audio_dma_ht_event;

event_source_t audio_event;

extern ew_device_status_t device_status;


static void mclkDisable(void) {
    palSetLineMode(LINE_I2S_MCLK, PAL_MODE_RESET);
}

static void mclkEnable(void) {
    palSetLineMode(LINE_I2S_MCLK, PAL_MODE_ALTERNATE(MCO_AF));
}


static void wm8960SetRegister(uint8_t reg, uint16_t val) {
    uint8_t tx_buf[2] = {
        (reg << 1) | ((val >> 8) & 1),      // first byte contains register address and MSB bit of 9 bit data
        val & 0xff,                         // second byte is 8 LSBs of data
    };
    i2cMasterTransmit(&I2CD2, 0x1a, tx_buf, 2, NULL, 0);
}

static void wm8960Init(void) {    
    mclkDisable();

    // setup audio codec
    // I2C2 driver is already started in main()
    i2cAcquireBus(&I2CD2);

    // IC reset
    wm8960SetRegister(WM8960_RESET, 0);

    // enable / disable modules
    mclkEnable();
    wm8960SetRegister(WM8960_POWER1, 0b011 << 6);           // enable VREF and VMID voltages with 50k divider
    wm8960SetRegister(WM8960_POWER2, (1 << 8) | (1 << 4));  // enable left DAC and left output buffers
    wm8960SetRegister(WM8960_POWER3, 1 << 3);               // enable left output mixer
    wm8960SetRegister(WM8960_ADDCTL1, 1 | 1 << 4);          // enable slow clock for volume update and mono mix on enabled DACs
    wm8960SetRegister(WM8960_ADDCTL2, 1 << 1);              // tristate ADCDAT pin to save power
    wm8960SetRegister(WM8960_ADDCTL4, 0b100 << 4);          // set GPIO1 (ADCLRC) to output SYSCLK
    wm8960SetRegister(WM8960_IFACE2, 1 << 6);               // ADCLRC as GPIO1
    
    // setup clocks
    wm8960SetRegister(WM8960_CLOCK1, WM8960_SYSCLK_MCLK | WM8960_DAC_DIV_1 | WM8960_SYSCLK_DIV_1);
    //wm8960SetRegister(WM8960_CLOCK2, WM8960_DCLK_DIV_16);   // this is the default value after reset

    // setup DAC
    wm8960SetRegister(WM8960_IFACE1, 0b0010);               // set audio format to 16 bit, I2C standard
    wm8960SetRegister(WM8960_DACCTL2, (1 << 3) | (1 << 2)); // enable soft mute with a slow ramp up (171 ms)

    // setup mixer
    wm8960SetRegister(WM8960_LOUTMIX, 1 << 8);      // route left DAC to left output mixer

    // setup speaker output
    wm8960SetRegister(WM8960_CLASSD1, (1 << 6) | 0b110111);     // enable left class D speaker output
    
    wm8960SetRegister(WM8960_DACCTL1, 0);                       // disable mute
    wm8960SetRegister(WM8960_LOUT2, (1 << 8) | (1 << 7) | device_status.alarm_volume);  // update left volume
    i2cReleaseBus(&I2CD2);
}

static void sdCardInit(void) {
    FRESULT err;

    // enable SD card power supply
    palSetLine(LINE_SD_EN);

    // Connect to the card
    if (sdcConnect(&SDCD1)) return;

    // mount file system
    err = f_mount(&SDC_FS, "/", 1);
    if (err != FR_OK) {
        sdcDisconnect(&SDCD1);
        return;
    }
    fs_ready = true;

}

static void wm8960PlayFile(char* filename) {
	FRESULT err;
	UINT btr;

    if (!fs_ready) {
        chprintf((BaseSequentialStream*) &SD1, "Filesystem not mounted!\n");
        return;
    }

    err = f_open(&file, filename, FA_READ);
    if (err != FR_OK) {
        chprintf((BaseSequentialStream*) &SD1, "Error opening file.\n");
        return;
    }

    err = f_read(&file, &dac_buffer, sizeof(FILEHeader), &btr);
	if (err != FR_OK) {
        chprintf((BaseSequentialStream*) &SD1, "Error reading file header.\n");
        return;
    }

    FILEHeader* header = (FILEHeader*) dac_buffer;

    if (!(header->riff.descriptor.id == RIFF
        && header->riff.type == WAVE
        && header->wave.descriptor.id == FMT
        && header->wave.audioFormat == FORMAT_PCM)) {
            chprintf((BaseSequentialStream*) &SD1, "Wrong file format.\n");
            return;
    }

    chprintf((BaseSequentialStream*) &SD1,
		"Channels: %d\r\nSample Rate: %ld\r\nBits per Sample: %d\r\n",
		header->wave.numChannels, header->wave.sampleRate, header->wave.bitsPerSample);

    if (header->wave.numChannels != 2) {
        chprintf((BaseSequentialStream*) &SD1, "Wrong number of channels.\n");
        return;
    }

    if (header->wave.bitsPerSample != 16) {
        chprintf((BaseSequentialStream*) &SD1, "Wrong number of bits per sample.\n");
        return;
    }
    /*
    if (header->wave.sampleRate != 48000) {
        chprintf((BaseSequentialStream*) &SD1, "Wrong sample rate.\n");
        return;
    }
    */

    err = f_read(&file, &dac_buffer, sizeof(DATAHeader), &btr);
	if (err != FR_OK) {
        chprintf((BaseSequentialStream*) &SD1, "Error reading file header.\n");
        return;
    }    
    DATAHeader* dataheader = (DATAHeader*) &dac_buffer;

    if ((uint32_t) dataheader->descriptor.id != DATA) {
        chprintf((BaseSequentialStream*) &SD1, "Could not find data header.\n");
        return;
    }  
    
    chprintf((BaseSequentialStream*) &SD1, "Now playing: %s\n", filename);

    bytes_to_play = dataheader->descriptor.size;

    // fill buffer completely
    err = f_read(&file, &dac_buffer, WAV_BUFFER_SIZE * sizeof(int16_t), &btr);
    bytes_to_play -= btr;

    // start continuous transfer of audio data
    i2sStartExchange(&I2SD3);
}

//===========================================================================
// Driver configs and callbacks
//===========================================================================

static void i2scallback(I2SDriver *i2sp) {
    chSysLockFromISR();
    if (i2sIsBufferComplete(i2sp)) {
        chEvtBroadcastI(&audio_dma_tc_event);
    } 
    else {
        chEvtBroadcastI(&audio_dma_ht_event);
    }
    chSysUnlockFromISR();
}

static const I2SConfig i2scfg = {
    dac_buffer,
    NULL,
    WAV_BUFFER_SIZE,
    i2scallback,
    0,  // CFGR register: I2SSTD = 00 (Philips I2S), CKPOL = 0 (clk default low), DATLEN = 00 (16-bit data), CHLEN = 0 (16-bit)
        // I2SMOD, I2SE and I2SCFG bits will be set by driver automatically
    20  // I2SPR register: I2SDIV = 20 (to get from 61.44 MHz I2SCLK to 48 kHz sample rate)
};

//===========================================================================
// Audio thread main function
//===========================================================================

THD_FUNCTION(audioThd, arg) {
    (void)arg;
	UINT btr = 0;
    event_listener_t el0, el1;
	void *pbuffer;

    pbuffer = dac_buffer;

    chRegSetThreadName("audio");
    
    chEvtObjectInit(&audio_event);
    
    i2sStart(&I2SD3, &i2scfg);              // setup I2S module
    wm8960Init();                           // setup audio IC
    sdCardInit();                           // setup filesystem

    wm8960PlayFile("/TWODOT~1.WAV");

    // register events
    chEvtObjectInit(&audio_dma_tc_event);
    chEvtObjectInit(&audio_dma_ht_event);
    chEvtRegister(&audio_dma_tc_event, &el0, EVT_DAC_TC);
    chEvtRegister(&audio_dma_ht_event, &el1, EVT_DAC_HT);

    // infinite loop
    while (1) {
        eventmask_t evt = chEvtWaitOne(ALL_EVENTS);

        if (evt & EVENT_MASK(EVT_DAC_TC)) {
            // TC -> fill second half of buffer with new data
            pbuffer += WAV_BUFFER_SIZE * sizeof(int16_t) / 2;
        }

        if (evt & EVENT_MASK(EVT_DAC_HT)) {
            // HT -> fill first half of buffer with new data
            pbuffer = dac_buffer;
        }

        f_read(&file, pbuffer, WAV_BUFFER_SIZE * sizeof(int16_t) / 2, &btr);

        bytes_to_play -= btr;

        // stop playing when there are not enough bytes left to play
        if (bytes_to_play < WAV_BUFFER_SIZE * sizeof(int16_t) / 2) {
            i2sStopExchange(&I2SD3);
        }
    }
}