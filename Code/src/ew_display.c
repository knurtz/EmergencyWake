/*
Functions for everything display related. Involves activating the DCDC converter for the negative display supply voltage, starting a timer in order
to generate a PWM signal for the display's filament and communicating with the PT6312 display driver IC via SPI.
*/

#include "ch.h"
#include "hal.h"

#include <string.h>
#include "chprintf.h"

#include "pt6312.h"

#include "ew_devicestatus.h"
#include "ew_events.h"
#include "ew_display.h"

#include "evtimer.h"

#define EVT_DISPLAY_EXT         0       // external request to display thread
#define EVT_DISPLAY_PERIODIC    1       // internal periodic second interrupt

static DigitData digits[4];

event_source_t display_event;
static event_timer_t periodic_event;

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
   {PWM_OUTPUT_ACTIVE_HIGH, NULL},  // channel 1 = PC6 = filament A line
   {PWM_OUTPUT_ACTIVE_LOW, NULL},   // channel 2 = PC7 = filament B line -> inverted
   {PWM_OUTPUT_DISABLED, NULL},
   {PWM_OUTPUT_DISABLED, NULL}
  },
  0,
  0
};

//===========================================================================
// Modify display content
//===========================================================================

static void setDigit(uint8_t digit, uint8_t value) {
    if (digit > 3)
        return;
    
    value = value % 10;                     // only the numbers from 0 - 9 are allowed

    digits[digit].complete |= 0x7f;         // initially turn on all number segments

    switch (value) {
        case 0:
            digits[digit].segments.seg_g = 0;
            break;

        case 1:
            digits[digit].segments.seg_a = 0;
        case 7:
            digits[digit].segments.seg_d = 0;
            digits[digit].segments.seg_g = 0;
        case 3:
            digits[digit].segments.seg_f = 0;
        case 9:
            digits[digit].segments.seg_e = 0;
            break;

        case 2:
            digits[digit].segments.seg_c = 0;
            digits[digit].segments.seg_f = 0;
            break;

        case 4:
            digits[digit].segments.seg_a = 0;
            digits[digit].segments.seg_d = 0;
            digits[digit].segments.seg_e = 0;
            break;

        case 5:
            digits[digit].segments.seg_e = 0;
        case 6:
            digits[digit].segments.seg_b = 0;
            break;
    }
}

static void setMinutes(uint8_t value) {
    setDigit(0, value % 10);
    setDigit(1, value / 10);
}

static void setHours(uint8_t value) {
    setDigit(2, value % 10);
    setDigit(3, value / 10);
}

// number can be a value between 1 and 4
static void setAlarmNumber(uint8_t number_mask, bool enabled) {
    if (number_mask > 0b1111) return;
    uint16_t mask = number_mask << 7;          // alarm numbers are part of digit 4, with a segment offset of 7
    if (enabled) digits[3].complete |= mask;
    else digits[3].complete &= ~mask;
}

static void toggleAlarmNumber(uint8_t number_mask) {
    uint16_t mask = number_mask << 7;                           // alarm numbers are part of digit 4, with a segment offset of 7
    setAlarmNumber(number_mask, digits[3].complete & mask);     // turn the required alarm number on or off depending on its current state
}

static void setDecimalPoint(bool enabled) {
    digits[1].segments.seg_p = enabled;
}

static void toggleDecimalPoint(void) {
    digits[1].segments.seg_p ^= 1;
}

static void setFanIcon(bool enabled) {
    digits[1].segments.seg_4 = enabled;
}

//===========================================================================
// Display IC communication
//===========================================================================

static void pt6312SendCommand(uint8_t command, uint8_t options) {
    uint8_t txbuf[1] = {command | options};

    spiSelect(&SPID2);          // strobe signal low
    spiExchange(&SPID2, 1, txbuf, txbuf);         // send data
    spiUnselect(&SPID2);        // strobe signal high

    chThdSleepMicroseconds(1);  // PW_STB on page 13 of PT6312 datasheet
}

static void pt6312SendCompleteDigitData(void) {
    uint8_t txbuf[9] = {
        PT6312_COMMAND_3 | PT6312_ADDRESS_DIGIT(0),
        digits[0].bytes.low_byte,
        digits[0].bytes.high_byte,
        digits[1].bytes.low_byte,
        digits[1].bytes.high_byte,
        digits[2].bytes.low_byte,
        digits[2].bytes.high_byte,
        digits[3].bytes.low_byte,
        digits[3].bytes.high_byte,
    };

    pt6312SendCommand(PT6312_COMMAND_2, PT6312_AUTO_INCREMENT);

    spiSelect(&SPID2);          // strobe signal low
    spiExchange(&SPID2, 9, txbuf, txbuf);         // send data, overwrite tx buffer with received data, since it is a local variable anyways
    chThdSleepMicroseconds(1);  // t_CLK-STB on page 13 of PT6312 datasheet
    spiUnselect(&SPID2);        // strobe signal high

    chThdSleepMicroseconds(1);  // PW_STB on page 13 of PT6312 datasheet
}

// n can be a value from 0 to 3
static void pt6312SendSingleDigitData(uint8_t digit) {
    if (digit > 3) return;
    uint8_t txbuf[3] = {
        PT6312_COMMAND_3 | PT6312_ADDRESS_DIGIT(digit),
        digits[digit].bytes.low_byte,
        digits[digit].bytes.high_byte,
    };

    pt6312SendCommand(PT6312_COMMAND_2, PT6312_AUTO_INCREMENT);

    spiSelect(&SPID2);          // strobe signal low
    spiExchange(&SPID2, 3, txbuf, txbuf);         // send data
    chThdSleepMicroseconds(1);  // t_CLK-STB on page 13 of PT6312 datasheet
    spiUnselect(&SPID2);        // strobe signal high

    chThdSleepMicroseconds(1);  // PW_STB on page 13 of PT6312 datasheet
}

static void pt6312Startup(void) {
    // power up dcdc converter
    palClearLine(LINE_DCDC_EN);

    // configure timer 8 to provide pwm signal on filament pins
    pwmStart(&PWMD8, &pwmcfg);
    pwmEnableChannel(&PWMD8, 0, PWM_PERCENTAGE_TO_WIDTH(&PWMD8, 5000));
    pwmEnableChannel(&PWMD8, 1, PWM_PERCENTAGE_TO_WIDTH(&PWMD8, 5000));
	
    // start SPI2 driver
    spiStart(&SPID2, &spicfg);

    // init display driver IC
    pt6312SendCompleteDigitData();
    pt6312SendCommand(PT6312_COMMAND_1, PT6312_MODE_4DIGITS);
    pt6312SendCommand(PT6312_COMMAND_4, PT6312_DISPLAY_ON | PT6312_PULSEWIDTH_MAX);
}

//===========================================================================
// UI generation
//===========================================================================

/*
EW_IDLE             -   hide alarm numbers, show hours and minutes from RTC module, blink decimal point slowly
EW_SHOW_ALARM       -   show corresponding alarm number, hours and minutes from corresponding stored time
EW_SET_HOURS        -   show modified hours value from system time or selected alarm. if alarm one or two, also show corresponding alarm number
EW_SET_MINUTES      -   same as above, but for minutes
EW_ALARM_RINGING    -   show stored time for corresponding alarm, blink alarm number rapidly
EW_STANDBY          -   clear display
*/
static void rebuildUI(void) {
    uint8_t temp;

    switch (device_status.state) {

        case EW_IDLE:
            setAlarmNumber(0b1111, false);      // disable all alarms numbers
            setHours(5);                        // TODO: get values from RTC unit
            setMinutes(22);                     
            break;

        case EW_ALARM_RINGING:
            device_status.toggle_state = device_status.next_alarm;
        case EW_SHOW_ALARM:
            setAlarmNumber(0b1111, false);
            setAlarmNumber(ALARM_NUMBER_MASK(device_status.toggle_state), true);
            setDecimalPoint(false);
            setHours(device_status.alarms[device_status.toggle_state].saved_time.hours);
            setMinutes(device_status.alarms[device_status.toggle_state].saved_time.minutes);
            break;

        case EW_SET_HOURS:
            digits[0].complete = digits[1].complete = 0;        // turn off minutes segments
            setAlarmNumber(0b1111, false);                      // turn off alarm numbers
            if (device_status.toggle_state == EW_ALARM_NONE) temp = device_status.modified_time.hours;
            else {
                temp = device_status.alarms[device_status.toggle_state].modified_time.hours;
                setAlarmNumber(ALARM_NUMBER_MASK(device_status.toggle_state), true);
            }
            setHours(temp);
            break;

        case EW_SET_MINUTES:
            digits[2].complete = digits[3].complete = 0;        // turn off hours segments
            setAlarmNumber(0b1111, false);                      // turn off alarm numbers
            if (device_status.toggle_state == EW_ALARM_NONE) temp = device_status.modified_time.minutes;
            else {
                temp = device_status.alarms[device_status.toggle_state].modified_time.minutes;
                setAlarmNumber(ALARM_NUMBER_MASK(device_status.toggle_state), true);
            }
            setMinutes(temp);
            break;

        case EW_STANDBY:
            digits[0].complete = digits[1].complete = digits[2].complete = digits[3].complete = 0;
            break;

        default:
            break;
    }
}

//===========================================================================
// Display thread main function
//===========================================================================

THD_FUNCTION(displayThd, arg) {
    event_listener_t el_display;
    uint8_t periodic_counter = 0;
    
    chEvtObjectInit(&display_event);
    chEvtRegister(&display_event, &el_display, EVT_DISPLAY_EXT);

    evtObjectInit(&periodic_event, TIME_MS2I(200));
    chEvtRegister(&periodic_event.et_es, &el_display, EVT_DISPLAY_PERIODIC);

    evtStart(&periodic_event);

    while(1) {
        eventmask_t evt = chEvtWaitAny(0);

        if (evt & EVENT_MASK(EVT_DISPLAY_EXT)) {
            eventflags_t flags = chEvtGetAndClearFlags(&el_display);

            if (flags & EVENT_MASK(EW_DISPLAY_TURNON)) {
                pt6312Startup();
            }

            if (flags & EVENT_MASK(EW_DISPLAY_REFRESH)) {
                // rebuild screen depending on current device_status
                rebuildUI();
                pt6312SendCompleteDigitData();
            }
        }

        if (evt & EVENT_MASK(EVT_DISPLAY_PERIODIC)) {
            // stuff to do periodically
                    
            switch (device_status.state) {
                case EW_IDLE:
                    periodic_counter++;
                    if (periodic_counter >= 5) {
                        toggleDecimalPoint();
                        periodic_counter = 0;
                    }
                    break;
                case EW_ALARM_RINGING:
                    toggleAlarmNumber(device_status.active_alarm + 1);
            }
        }

    }
}
