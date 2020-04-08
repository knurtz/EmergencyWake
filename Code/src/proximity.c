#include "ch.h"
#include "hal.h"
#include "usbcfg.h"

#include <string.h>
#include "chprintf.h"

//===========================================================================
// I2C related
//===========================================================================

static const I2CConfig i2ccfg = {
    OPMODE_I2C,
    100000,
    STD_DUTY_CYCLE,
};

void proximity_init(void)
{

    // Initialize I2C2 driver used for proximity sensor and audio codec
    uint8_t tx[] = {0x80, 0b100101};
    uint8_t rx[] = {0};
    i2cStart(&I2CD2, &i2ccfg);
    i2cAcquireBus(&I2CD2);
    // set register 0x80 (Enable register) to 0b100101 -> PIEN, PEN, PON bits set
    i2cMasterTransmitTimeout(&I2CD2,
                             0x39,
                             tx,
                             2,
                             NULL,
                             0,
                             TIME_MS2I(10));
    i2cReleaseBus(&I2CD2);

    // read sensor value
    tx[0] = 0x9c;
    i2cAcquireBus(&I2CD2);
    // read register 0x9c (proximity data)
    i2cMasterTransmitTimeout(&I2CD2,
                               0x39,
                               tx,
                               1,
                               rx,
                               1,
                               TIME_MS2I(10));
    i2cReleaseBus(&I2CD2);
    chprintf((BaseSequentialStream*) &SDU1, "prox value: %d\r\n", rx[0]);
}