#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "ch.h"
#include "ew_time.h"
#include "ew_devicestatus.h"

#define EW_TIMEOUT_SHORT            10U      // short timeout, after starting snooze
#define EW_TIMEOUT_LONG             300U     // long timeout, after showing current time

// main state machine handler
void handleEvent(uint16_t new_event, uint16_t flags);

ew_state_t enterAlarmRinging(void);
ew_state_t enterIdle(int display_timeout);

#endif /* STATEMACHINE_H */
