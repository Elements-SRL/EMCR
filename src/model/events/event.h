#ifndef EVENT_H
#define EVENT_H

#include <vector>
#include <string>

struct Event {
    uint64_t eventIdx;
    std::vector<int16_t> rawData;
    int16_t stimulus;
    double resolution;
    std::string uom;
    double samplingRate;
    std::string samplingRateUom;
    double multiplier;
    double stimulusMultiplier;

    Event(uint64_t eventIdx, 
        std::vector<int16_t> rawData,
        double stimulus,
        std::string stimulusUom,
        double resolution,
        std::string uom,
        double samplingRate,
        std::string samplingRateUom,
        double multiplier,
        double stimulusMultiplier) :

        eventIdx(eventIdx),
        rawData(rawData),
        stimulus(stimulus),
        resolution(resolution),
        uom(uom),
        samplingRate(samplingRate),
        samplingRateUom(samplingRateUom),
        multiplier(multiplier),
        stimulusMultiplier(stimulusMultiplier) {
    
    }
};

#endif // EVENT_H