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
    std::vector<int16_t> rawData;
    double samplingRate;
    std::string samplingRateUom;

    Event(uint64_t eventIdx_, std::vector<int16_t> rawData_, double stimulus_, std::string stimulusUom_, double resolution_, std::string uom_, double samplingRate_, std::string samplingRateUom_)
        : eventIdx(eventIdx_), rawData(rawData_), stimulus(stimulus_), stimulusUom(stimulusUom_), resolution(resolution_), uom(uom_), samplingRate(samplingRate_), samplingRateUom(samplingRateUom_){}
};

#endif // EVENT_H