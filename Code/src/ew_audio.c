/*
Functions for everything audio and SD card related. Involves controlling the SDIO driver, communicating with the audio codec IC, 
reading files from the SD card, and providing functions for other threads to start and stop playback of certain files.

*/

#include "ch.h"
#include "hal.h"
#include "wm8960.h"

#define PLLN_VALUE 12
#define PLLK_VALUE 4831838  // int(2^24 * 0,288)

extern const uint16_t dac_buffer[480];

static void mclkDisable(void) {
    palSetLineMode(LINE_I2S_MCLK, PAL_MODE_RESET);
}

static void mclkEnable(void) {
    palSetLineMode(LINE_I2S_MCLK, PAL_MODE_ALTERNATE(MCO_AF));
}

static void wm8960SetRegister(uint8_t reg, uint16_t val) {
    uint8_t tx_buf[2] = {
        (reg << 1) | ((val >> 8) & 1),  // first byte contains register address and MSB bit of 9 bit data
        val & 0xff,                     // second byte is 8 LSBs of data
    };
    i2cMasterTransmit(&I2CD2, 0x1a, tx_buf, 2, NULL, 0);
}

static void i2scallback(I2SDriver *i2sp) {
    if (i2sIsBufferComplete(i2sp)) {
        palToggleLine(LINE_LED3);
    } else {
        palToggleLine(LINE_LED3);
    }
}

static const I2SConfig i2scfg = {
    dac_buffer,
    NULL,
    480,
    i2scallback,
    0,  // CFGR register: I2SSTD = 00 (Philips I2S), CKPOL = 0 (clk default low), DATLEN = 00 (16-bit data), CHLEN = 0 (16-bit)
        // I2SMOD, I2SE and I2SCFG bits will be set by driver automatically
    25  // I2SPR register: I2SDIV = 25 (to get from 76.8 MHz I2SCLK to 48 kHz sample rate)
};

THD_FUNCTION(audioThd, arg) {
    (void)arg;
    chRegSetThreadName("audio");

    mclkDisable();

    // setup I2S module
    i2sStart(&I2SD3, &i2scfg);

    // setup audio codec
    // I2S driver is already started in main()
    i2cAcquireBus(&I2CD2);

    // setup clocks
    wm8960SetRegister(WM8960_CLOCK1, WM8960_SYSCLK_PLL | WM8960_DAC_DIV_1 | WM8960_SYSCLK_DIV_2);
    wm8960SetRegister(WM8960_CLOCK2, WM8960_DCLK_DIV_16);     // is the default value
    wm8960SetRegister(WM8960_POWER2, 1);                      // enable PLL
    wm8960SetRegister(WM8960_PLL1, 1 << 5 | PLLN_VALUE);      // enable fractional mode of PLL, set PLLN value
    wm8960SetRegister(WM8960_PLL2, PLLK_VALUE >> 16 & 0xff);  // 8 MSBs of PLLK value
    wm8960SetRegister(WM8960_PLL3, PLLK_VALUE >> 8 & 0xff);   // next 8 bits of PLLK value
    wm8960SetRegister(WM8960_PLL4, PLLK_VALUE & 0xff);        // 8 LSBs of PLLK value

    mclkEnable();
    chThdSleepMilliseconds(100);  // wait for PLL to settle

    // setup DAC
    wm8960SetRegister(WM8960_IFACE1, 0b0010);                // set audio format to 16 bit, I2C standard
    //wm8960SetRegister(WM8960_ADDCTL2, 1 << 1);             // tristate ADC pin to save power (I guess)
    wm8960SetRegister(WM8960_POWER2, 1 | (1 << 8));          // enable PLL and left DAC
    wm8960SetRegister(WM8960_DACCTL2, (1 << 3) | (1 << 2));  // enable soft mute with a slow ramp up (171 ms)
    wm8960SetRegister(WM8960_DACCTL1, 0);                    // disable mute

    // setup mixer
    wm8960SetRegister(WM8960_LOUTMIX, 1 << 8);  // route left DAC to left output mixer
    wm8960SetRegister(WM8960_POWER3, 1 << 3);   // enable left output mixer

    // setup speaker output
    wm8960SetRegister(WM8960_LOUT2, (1 < 8) | (1 < 7) | 60);    // update left volume on next zero crossing, set to 100
    wm8960SetRegister(WM8960_POWER2, 1 | (1 << 8) | (1 << 4));  // enable left output buffers
    wm8960SetRegister(WM8960_CLASSD1, (1 << 6) | 0b110111);     // enable left class D speaker output

    i2cReleaseBus(&I2CD2);

    // start continuous transfer of audio data
    i2sStartExchange(&I2SD3);

    // infinite loop
    while (1) {

      chThdSleepMilliseconds(2000);
      i2cAcquireBus(&I2CD2);
      wm8960SetRegister(WM8960_DACCTL1, 1 << 3);                    // enable mute
      i2cReleaseBus(&I2CD2);
      palToggleLine(LINE_LED4);

      chThdSleepMilliseconds(2000);
      i2cAcquireBus(&I2CD2);
      wm8960SetRegister(WM8960_DACCTL1, 0);                    // disable mute
      i2cReleaseBus(&I2CD2);
      palToggleLine(LINE_LED4);

    }

}