#ifndef EVENTINFO_H
#define EVENTINFO_H

#include "event.h"

struct EventInfo {
    double amplitude;
    double duration;
    Event event;

    EventInfo(double amplitude_, double duration_, Event event_)
        : amplitude(amplitude_), duration(duration_), event(event_) {}
};

#endif // EVENTINFO_H