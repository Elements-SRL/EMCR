#ifndef EVENTPACKET_H
#define EVENTPACKET_H

#include <vector>
#include <string>
#include "enventinfo.h"
#include "baseline.h"

struct EventPacket {
    std::vector<EventInfo> eventsinfo;
    Baseline baseline;
    double eventPerSecond;
    EventPacket(std::vector<EventInfo> eventsinfo_, Baseline baseline_, double eventPerSecond_)
        : eventsinfo(eventsinfo_), baseline(baseline_), eventPerSecond(eventPerSecond_) {}
};

#endif // EVENTPACKET_H