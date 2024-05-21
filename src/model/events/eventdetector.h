#ifndef EVENTDETECTIONCHANNEL_H
#define EVENTDETECTIONCHANNEL_H

#include <vector>
#include <optional>
#include "event.h"
#include "firstorderiirfilter.h"
#include "plotmessage.h"
#include "messagedispatcher.h"

constexpr uint32_t EVENT_TH = 3; /*To be considered an event the signal must be EVENT_TH times the std dev*/
constexpr uint32_t MIN_LEN = 80;
constexpr uint32_t MAX_LEN = 8000;
constexpr uint32_t EVENT_PADDING = 4;

class EventDetector {
public:
    EventDetector(Measurement samplingRate, int sizeHint = -1);

    std::pair<std::vector<Event>, Baseline> consumeEventsAndBaseline();

    void setChunk(std::vector<int16_t> intBuffer, std::vector<double> doubleBuffer, std::vector<double> voltages, uint32_t chunkSize, RangedMeasurement currentRange, RangedMeasurement voltageRange, Measurement samplingRate);
    void clear();

private:
    double threshold = -1;
    double calculateThreshold(const std::vector<double>& data);
    bool eventAlreadyBegun = false;
    uint32_t eventLen = 0;
    uint32_t eventBeginIdx = 0;

    RangedMeasurement currentRange;
    RangedMeasurement voltageRange;
    Measurement samplingRate;

    FirstOrderIirFilter* high = nullptr;
    FirstOrderIirFilter* low = nullptr;

    uint32_t baselineSamplingRate;
    uint32_t baselineSamplingRateCounter = 0;
    //INITIALIZE THOSE  vvvv
    std::vector<double> bandPassFilterData;
    std::vector<Event> events;
    std::vector<int16_t> baseline;

    double calcStdDev(const std::vector<double>& data);
    std::optional<std::pair<int, int>> analyze(double currentValue, uint32_t idx, uint32_t bufferSize);
    void processEvent(std::pair<int, int> evtBegingEnd, std::vector<int16_t>& intBuffer, double voltage, uint64_t offset, uint32_t chunkSize);
};


#endif // EVENTDETECTIONCHANNEL_H
