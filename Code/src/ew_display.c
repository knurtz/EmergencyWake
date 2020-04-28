/*
Functions for everything display related. Involves activating the DCDC converter for the negative display supply voltage, starting a timer in order
to generate a PWM signal for the display's filament and communicating with the PT6312 display driver IC via SPI.
*/

#include "ch.h"
#include "hal.h"
#include <string.h>
#include "chprintf.h"

#define PT6312_COMMAND_1            (0b00 << 6)
#define PT6312_COMMAND_2            (0b01 << 6)
#define PT6312_COMMAND_3            (0b11 << 6)
#define PT6312_COMMAND_4            (0b10 << 6)

// options for command 1
#define PT6312_MODE_4DIGITS         0

// options for command 2
#define PT6312_WRITE_DISPLAY_DATA   0
#define PT6312_AUTO_INCREMENT       0

// options for command 3
#define PT6312_ADDRESS_DIGIT0       0
#define PT6312_ADDRESS_DIGIT1       2
#define PT6312_ADDRESS_DIGIT2       4
#define PT6312_ADDRESS_DIGIT3       6

// options for command 4
#define PT6312_PULSEWIDTH_MIN       0
#define PT6312_PULSEWIDTH_1         0
#define PT6312_PULSEWIDTH_2         1
#define PT6312_PULSEWIDTH_4         2
#define PT6312_PULSEWIDTH_10        3
#define PT6312_PULSEWIDTH_11        4
#define PT6312_PULSEWIDTH_12        5
#define PT6312_PULSEWIDTH_13        6
#define PT6312_PULSEWIDTH_14        7
#define PT6312_PULSEWIDTH_MAX       7
#define PT6312_DISPLAY_ON           (1 << 3)
#define PT6312_DISPLAY_OFF          0


typedef union
{
    struct {
        uint8_t seg_a : 1;
        uint8_t seg_b : 1;
        uint8_t seg_c : 1;
        uint8_t seg_d : 1;
        uint8_t seg_e : 1;
        uint8_t seg_f : 1;
        uint8_t seg_g : 1;
        uint8_t seg_1 : 1;
        uint8_t seg_2 : 1;
        uint8_t seg_3 : 1;
        uint8_t seg_4 : 1;
        uint8_t seg_p : 1;
    } segments;
    struct {
        uint8_t low_byte;
        uint8_t high_byte;
    } bytes;
    uint16_t complete;
} DigitData;

static DigitData digits[4];


//===========================================================================
// Modify display content
//===========================================================================

// value can be a number between 0 - 9 for now
static void setDigit(uint8_t n, char value) {
    if (n > 3) return;

    digits[n].complete |= 0x7f;        // initially turn on all segments needed for numbers

    switch (value) {
        case 0:
            digits[n].segments.seg_g = 0;
            break;
        case 1:
            digits[n].segments.seg_a = 0;
            digits[n].segments.seg_d = 0;
            digits[n].segments.seg_e = 0;
            digits[n].segments.seg_f = 0;
            digits[n].segments.seg_g = 0;
            break;
        case 2:
            digits[n].segments.seg_c = 0;
            digits[n].segments.seg_f = 0;
            break;
        case 3:
            digits[n].segments.seg_e = 0;
            digits[n].segments.seg_f = 0;
            break;
        case 4:
            digits[n].segments.seg_a = 0;
            digits[n].segments.seg_d = 0;
            digits[n].segments.seg_e = 0;
            break;
        case 5:
            digits[n].segments.seg_b = 0;
            digits[n].segments.seg_e = 0;
            break;
        case 6:
            digits[n].segments.seg_b = 0;
            break;
        case 7:
            digits[n].segments.seg_d = 0;
            digits[n].segments.seg_e = 0;
            digits[n].segments.seg_f = 0;
            digits[n].segments.seg_g = 0;
            break;
        case 8:
            break;
        case 9:
            digits[n].segments.seg_e = 0;
            break;
    }
}

static void setMinutes(uint8_t value) {
    if (value > 100) return;
    setDigit(0, value % 10);
    setDigit(1, value / 10);
}

static void setHours(uint8_t value) {
    if (value > 100) return;
    setDigit(2, value % 10);
    setDigit(3, value / 10);
}

// number can be a value between 1 and 4
static void enableAlarmNumber(uint8_t number, bool enabled) {
    if (enabled) digits[3].complete |= 1 << (6 + number);
    else digits[3].complete &= ~(uint16_t)(1 << (6 + number));
}

static void enableDecimalPoint(bool enabled) {
    digits[1].segments.seg_p = enabled;
}

static void enableFanIcon(bool enabled) {
    digits[1].segments.seg_4 = enabled;
}


//===========================================================================
// Display IC communication
//===========================================================================

static void pt6312SendCommand(uint8_t command, uint8_t options) {
    uint8_t txbuf[1] = {command | options};
    uint8_t rxbuf[1];

    spiSelect(&SPID2);          // strobe signal low
    spiExchange(&SPID2, 1, txbuf, rxbuf);         // send data
    spiUnselect(&SPID2);        // strobe signal high

    chThdSleepMicroseconds(1);  // PW_STB on page 13 of PT6312 datasheet
}

static void pt6312SendCompleteDigitData(void) {
    uint8_t txbuf[9] = {
        PT6312_COMMAND_3 | PT6312_ADDRESS_DIGIT0,
        digits[0].bytes.low_byte,
        digits[0].bytes.high_byte,
        digits[1].bytes.low_byte,
        digits[1].bytes.high_byte,
        digits[2].bytes.low_byte,
        digits[2].bytes.high_byte,
        digits[3].bytes.low_byte,
        digits[3].bytes.high_byte,
    };
    uint8_t rxbuf[9];

    pt6312SendCommand(PT6312_COMMAND_2, PT6312_AUTO_INCREMENT);

    spiSelect(&SPID2);          // strobe signal low
    spiExchange(&SPID2, 9, txbuf, rxbuf);         // send data
    chThdSleepMicroseconds(1);  // t_CLK-STB on page 13 of PT6312 datasheet
    spiUnselect(&SPID2);        // strobe signal high

    chThdSleepMicroseconds(1);  // PW_STB on page 13 of PT6312 datasheet
}


//===========================================================================
// Driver configs and callbacks
//===========================================================================

static const SPIConfig spicfg = {
  false,                    // no circular mode
  NULL,                     // end callback
  GPIOB,                    // chip select port
  GPIOB_SPI_STB,            // chip select pin
  SPI_CR1_MSTR | SPI_CR1_LSBFIRST | 0b101 << SPI_CR1_BR_Pos | SPI_CR1_CPOL | SPI_CR1_CPHA,      // SPI CR1 register
  0                         // SPI CR2 register
};

static const PWMConfig pwmcfg = {
  100000,                           // 100 kHz PWM clock frequency
  1000,                             // Initial PWM period 1000 ms -> 100 Hz
  NULL,                             // no callback for timer update interrupt
  {
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},  // channel 1 = PC6 = fil a line
   {PWM_OUTPUT_ACTIVE_LOW, NULL},   // channel 2 = PC7 = fil b line -> inverted
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL}
  },
  0,
  0
};


//===========================================================================
// Display thread main function
//===========================================================================

THD_FUNCTION(displayThd, arg) {
    (void)arg;

    chThdSleepMilliseconds(500);

    // power up dcdc converter
    palClearLine(LINE_DCDC_EN);

    // configure timer 8 to provide pwm signal on filament pins
    pwmStart(&PWMD8, &pwmcfg);
    pwmEnableChannel(&PWMD8, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD8, 5000));
    pwmEnableChannel(&PWMD8, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD8, 5000));

    digits[0].complete = 0;
    digits[1].complete = 0;
    digits[2].complete = 0;
    digits[3].complete = 0;
	
    // start SPI2 driver
    spiStart(&SPID2, &spicfg);

    // init display driver IC
    pt6312SendCompleteDigitData();
    pt6312SendCommand(PT6312_COMMAND_1, PT6312_MODE_4DIGITS);
    pt6312SendCommand(PT6312_COMMAND_4, PT6312_DISPLAY_ON | PT6312_PULSEWIDTH_MAX);

    uint8_t current_value = 0;
    uint8_t alarm_number = 1;
    bool foo = true;

    while(1) {
        chThdSleepMilliseconds(100);

        setMinutes(current_value);
        setHours(current_value);

        enableAlarmNumber(alarm_number, true);
        enableDecimalPoint(foo);
        enableFanIcon(foo);
        
        pt6312SendCompleteDigitData();

        enableAlarmNumber(alarm_number, false);

        if (++current_value > 100) current_value = 0;
        foo = !foo;
        if (++alarm_number > 4) alarm_number = 1;
    }
}
