#ifndef EVENT_H
#define EVENT_H

#include <vector>
#include <string>

struct Event {
    uint64_t eventIdx;
    double stimulus;
    std::string stimulusUom;
    double resolution;
    std::string uom;
    std::vector<int16_t> event;

    Event(uint64_t eventIdx_, std::vector<int16_t> event_, double stimulus_, std::string stimulusUom_, double resolution_, std::string uom_)
        : eventIdx(eventIdx_), event(event_), stimulus(stimulus_), stimulusUom(stimulusUom_), resolution(resolution_), uom(uom_){}
};

#endif // EVENT_H