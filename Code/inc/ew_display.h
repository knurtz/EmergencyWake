#ifndef EW_DISPLAY_H
#define EW_DISPLAY_H

#include "ch.h"

THD_FUNCTION(displayThd, arg);

#define ALARM_NUMBER_MASK(n)    (1 << n)

typedef union {
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

#endif /* EW_DISPLAY_H */
