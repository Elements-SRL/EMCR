#ifndef EVENT_H
#define EVENT_H

#include <vector>
#include <optional>

struct Event {
    uint64_t eventIdx;
    std::vector<double> event;

    Event(uint64_t eventIdx_, std::vector<double> event_)
        : eventIdx(eventIdx_), event(event_) {}
};

#endif // EVENT_H