#ifndef EVENTPACKET_H
#define EVENTPACKET_H

#include <vector>
#include "eventinfo.h"
#include "baseline.h"

struct EventPacket {
    std::vector<EventInfo> eventsinfo;
    Baseline iBaseline;
    Baseline vBaseline;
    double eventPerSecond;
    EventPacket(std::vector<EventInfo> eventsinfo_, Baseline iBaseline_, Baseline vBaseline_, double eventPerSecond_)
        : eventsinfo(eventsinfo_), iBaseline(iBaseline_), vBaseline(vBaseline_), eventPerSecond(eventPerSecond_) {}
};

#endif // EVENTPACKET_H
