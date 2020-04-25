#ifndef EW_AUDIO_H
#define EW_AUDIO_H

#include "ch.h"
#include "hal.h"

THD_WORKING_AREA(audio_wa, 2048);
THD_FUNCTION(audioThd, arg);

void playSnooze(void);
void playDisabled(void);
void playRingtone(void);
void stopRingtone(void);

#endif /* EW_AUDIO_H */
