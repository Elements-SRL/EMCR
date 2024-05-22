#ifndef EVENTPACKET_H
#define EVENTPACKET_H

#include <vector>
#include <string>
#include "enventinfo.h"
#include "baseline.h"

struct EventPacket {
    std::vector<EventInfo> eventsinfo;
    Baseline baseline;

    EventPacket(std::vector<EventInfo> eventsinfo_, Baseline baseline_)
        : eventsinfo(eventsinfo_), baseline(baseline_) {}
};

#endif // EVENTPACKET_H