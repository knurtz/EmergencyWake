#ifndef EW_EVENTS_H
#define EW_EVENTS_H

#include "ch.h"

#define EW_USER_ALARM       0
#define EW_LEVER_DOWN       1
#define EW_LEVER_UP         2
#define EW_TOGGLE_CHANGE    3
#define EW_ENCODER_BUTTON   4
#define EW_ENCODER_CHANGED  5
#define EW_SNOOZE_TIMER     6
#define EW_STANDBY_TIMER    7

#define EVENT_FLAGS_OFFSET  16

// Project wide event sources.
// They might be subscribed and broadcasted to by many different threads within the project. All event sources are definced and init

// Usually in ChibiOS events are used differently. Each event (e.g. toggle change or lever down) should normally get a seperate event source.
// Therefore, there should be an equal amount of specific event listeners for each event source.
// In this project there are only three event sources. One for the main statemachine, one for the display thread and one for the audio thread.
// This means, there only have to be 3 event listeners instead of 10.
// Additionaly the option to pass flags together with an event is used, to further specify which type of event occured.
// For example, a statemachine event only tells the main statemachine, that something UI related happened (for example a button was pressed or an alarm went off).
// What exactly happened will be encoded in the event flags. These are 32 bit values. 
// The lower 16 bits are used to identify the exact type of event. The upper 16 bits can be used as an additional binary flag for each event type.
// This additional flag is not used often, currently only to specify the direction (up / down) for the encoder value changed event.

// UI related, processed by statemachine
extern event_source_t statemachine_event;

// Display related: defined, initialized and processed by display thread
extern event_source_t display_event;

// Audio related: defined, initialized and processed by audio thread
extern event_source_t audio_event;

#endif /* EW_EVENTS_H */
