#ifndef STATEMACHINE_H
#define STATEMACHINE_H


#define EW_TIMEOUT_SHORT            10U      // short timeout, after starting snooze
#define EW_TIMEOUT_LONG             300U     // long timeout, after showing current time

// main state machine handler
ew_state_t handleEvent(eventmask_t new_event, ew_state_t current_state);


#endif /* STATEMACHINE_H */
