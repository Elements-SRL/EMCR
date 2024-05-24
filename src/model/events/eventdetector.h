#ifndef EVENTDETECTIONCHANNEL_H
#define EVENTDETECTIONCHANNEL_H

#include <vector>
#include <optional>
#include "enventinfo.h"
#include "eventpacket.h"
#include "firstorderiirfilter.h"
#include "plotmessage.h"
#include "messagedispatcher.h"
#include <tuple>

constexpr uint32_t EVENT_TH = 3; /*To be considered an event the signal must be EVENT_TH times the std dev*/
constexpr uint32_t MIN_LEN = 10;
constexpr uint32_t MAX_LEN = 8000;
constexpr uint32_t EVENT_PADDING = 4;

class EventDetector {
public:
    EventDetector(Measurement samplingRate, int sizeHint = -1);

    EventPacket consumeEventsAndBaseline();

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
    std::vector<EventInfo> eventsInfo;
    std::vector<int16_t> baseline;
    uint32_t chunkSize = 0;
    double estimatedInterEventTime = 0.0;
    long long prevEventStartIdx = 0;

    uint32_t remainingChunkSize = 0;
    std::vector<int16_t> remainingIntBuffer;
    std::vector<double> remainingDoubleBuffer;
    std::vector<double> remainingVoltages;

    double calcStdDev(const std::vector<double>& data);
    std::optional<std::tuple<uint32_t, uint32_t, uint32_t>> analyze(double currentValue, uint32_t idx, uint32_t bufferSize);
    void processEvent(const std::tuple<uint32_t, uint32_t, uint32_t> evtBegingEnd, std::vector<int16_t>& intBuffer, double voltage, uint32_t chunkSize);

    uint64_t timeCount = 0;
};


#endif // EVENTDETECTIONCHANNEL_H
