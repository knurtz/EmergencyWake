#ifndef EW_DISPLAY_H
#define EW_DISPLAY_H

#include "ch.h"
#include "hal.h"

THD_WORKING_AREA(display_wa, 128);
THD_FUNCTION(displayThd, arg);

#endif /* EW_DISPLAY_H */
