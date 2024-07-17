#ifndef EVENT_H
#define EVENT_H

#include <vector>
#include <string>

struct Event {
    uint64_t eventIdx;
    int16_t stimulus;
    std::vector<int16_t> rawData;

    Event(uint64_t eventIdx_, 
        std::vector<int16_t> rawData_,
        double stimulus_,
        std::string stimulusUom_,
        double resolution_, 
        std::string uom_,
        double samplingRate_,
        std::string samplingRateUom_, 
        double multiplier_, 
        double stimulusMultiplier_
): 
        eventIdx(eventIdx_), 
        rawData(rawData_),
        stimulus(stimulus_){}
};

#endif // EVENT_H