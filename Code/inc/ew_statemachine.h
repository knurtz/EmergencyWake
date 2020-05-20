#ifndef STATEMACHINE_H
#define STATEMACHINE_H


#define EW_TIMEOUT_SHORT            10U      // short timeout, after starting snooze
#define EW_TIMEOUT_LONG             300U     // long timeout, after showing current time

// main state machine handler
ew_state_t handleEvent(uint16_t new_event, uint16_t flags);


#endif /* STATEMACHINE_H */
