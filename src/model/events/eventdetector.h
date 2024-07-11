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
#include "eventsdirection.h"

constexpr uint32_t EVENT_PADDING = 3;

struct PartialEvent {
    uint32_t eventBegin;
    uint32_t eventEnd;
    uint32_t realLen;
    int16_t baseline;
};

class EventDetector {
public:
    EventDetector(Measurement samplingRate, double highCutoffFrequency, uint32_t minEventLen, uint32_t maxEventLen, double stdMultiplier_, double maxAmplitude, EventsDirection eventsDirection);

    EventPacket consumeEventsAndBaseline();

    void setChunk(std::vector<int16_t> intBuffer, std::vector<double> doubleBuffer, std::vector<double> voltages, uint32_t chunkSize, RangedMeasurement currentRange, RangedMeasurement voltageRange, Measurement samplingRate);
    void clear();

    void setMinEventDurationInSamples(uint32_t);
    void setMaxEventDurationInSamples(uint32_t);
    void setHighCutoffFrquency(double);
    void setStdMultiplier(double newValue);
    void setMaxAmplitude(double);
    void setEventsDirection(EventsDirection ed);
    double getMaxAmplitude();

private:
    double threshold = -1;
    double calculateThreshold(const std::vector<double>& data);
    bool eventAlreadyBegun = false;
    uint32_t eventLen = 0;
    uint32_t eventBeginIdx = 0;

    uint32_t minEventLen;
    uint32_t maxEventLen;

    double highCutoffFrequency;
    double stdMultiplier;
    EventsDirection eventsDirection;

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
    std::vector<int16_t> baselineStimulus;
    double maxAmplitude;

    uint32_t chunkSize = 0;
    double estimatedInterEventTime = 0.0;
    long long prevEventStartIdx = 0;

    uint32_t remainingChunkSize = 0;
    std::vector<int16_t> remainingIntBuffer;
    std::vector<double> remainingDoubleBuffer;
    std::vector<double> remainingVoltages;

    double calcStdDev(const std::vector<double>& data);
    std::optional<PartialEvent> analyze(double currentValue, double voltage, uint32_t idx, uint32_t bufferSize);
    void processEvent(const PartialEvent, std::vector<int16_t>& , double , uint32_t );

    uint64_t timeCount = 0;
};


#endif // EVENTDETECTIONCHANNEL_H
