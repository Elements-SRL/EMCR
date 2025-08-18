#ifndef EVENTINFO_H
#define EVENTINFO_H

#include "event.h"

struct EventInfo {
    double amplitude;
    double duration;
    Event_t event;

    EventInfo(double amplitude_, double duration_, Event_t event_)
        : amplitude(amplitude_), duration(duration_), event(event_) {}
};

#endif // EVENTINFO_H
