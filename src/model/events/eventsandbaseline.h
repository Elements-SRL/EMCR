#ifndef EVENT_H
#define EVENT_H

#include <vector>

struct Event {
    uint64_t eventIdx;
    std::vector<int16_t> event;

    Event(uint64_t eventIdx_, std::vector<int16_t> event_, double stimulus_, std::string stimulusUom_)
        : eventIdx(eventIdx_), event(event_), stimulus(stimulus_), stimulusUom(stimulusUom_) {}
};

#endif // EVENT_H