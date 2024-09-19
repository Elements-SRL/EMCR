#include "eventdetector.h"
#include <cmath>
#include <iostream>
#include <cstdint>

EventDetector::EventDetector(Measurement samplingRate, double highCutoffFrequency, uint32_t minEventLen, uint32_t maxEventLen, double stdMultiplier_, double maxAmplitude, EventsDirection eventsDirection) {
    const auto lowCutoffFrequency = 100.0;
    this->minEventLen = minEventLen;
    this->maxEventLen = maxEventLen;
    this->highCutoffFrequency = highCutoffFrequency;
    this->stdMultiplier = stdMultiplier_;
    this->maxAmplitude = maxAmplitude;
    this->eventsDirection = eventsDirection;
    high = new FirstOrderIirFilter(samplingRate.getNoPrefixValue(), highCutoffFrequency);
    low = new FirstOrderIirFilter(samplingRate.getNoPrefixValue(), lowCutoffFrequency);
    baselineSamplingRate = samplingRate.getNoPrefixValue() / (lowCutoffFrequency * 5.0);
    baselineSamplingRateCounter = 0;

    const auto finalPadding = maxEventLen * EVENT_PADDING;
}

EventPacket EventDetector::consumeEventsAndBaseline() {
    uint32_t numberOfEvents = eventsInfo.size();
    const auto eventPeakBeginFactor = ((double)EVENT_PADDING / (double)((EVENT_PADDING * 2) + 1));
    for (auto& ei : eventsInfo) {
        const auto realEventIdx = (uint64_t)((double) ei.event.eventIdx + (double) ei.event.rawData.size() * eventPeakBeginFactor);
        estimatedInterEventTime = estimatedInterEventTime * 0.9 + 0.1 * ( realEventIdx - prevEventStartIdx);
        prevEventStartIdx = ei.event.eventIdx;
    }
    const auto bi = Baseline(currentRange.step, baseline, currentRange.unit);
    const auto bv = Baseline(voltageRange.step, baselineStimulus, voltageRange.unit);
    double eventPerSecond = 1.0 / estimatedInterEventTime * samplingRate.getNoPrefixValue();
    eventPerSecond = std::isnan(eventPerSecond) ? 0.0 : eventPerSecond;
    const EventPacket ep = EventPacket(eventsInfo, bi, bv, eventPerSecond);
    eventsInfo.clear();
    baseline.clear();
    baselineStimulus.clear();
    return ep;
}

double EventDetector::calculateThreshold(const std::vector<double>& data) {
    const auto stdDev = calcStdDev(data);
    return stdDev != -1 ? stdMultiplier * stdDev : stdDev;
}

double EventDetector::calcStdDev(const std::vector<double>& data) {
    const size_t n = data.size();
    if (n <= 1) {
        return -1.0; // Standard deviation is undefined for one or zero elements
    }

    // Calculate the mean and sum of squared differences
    double sum = 0.0;
    double sum_squared_diffs = 0.0;

#pragma omp parallel for reduction(+:sum,sum_squared_diffs)
    for (int i = 0; i < n; ++i) {
        sum += data[i];
    }

    const double mean = sum / static_cast<double>(n);

#pragma omp parallel for reduction(+:sum_squared_diffs)
    for (int i = 0; i < n; ++i) {
        const double diff = data[i] - mean;
        sum_squared_diffs += diff * diff;
    }

    // Calculate the variance
    const double variance = sum_squared_diffs / static_cast<double>(n - 1); // using Bessel's correction

    // Standard deviation is the square root of variance
    return std::sqrt(variance);
}

std::optional<PartialEvent> EventDetector::analyze(double currentValue, double voltage, uint32_t idx, uint32_t clipValue) {
    //begin event analysis
    const auto lowParams = low->getParams();
    const auto singleBaseline = low->sfilt(currentValue);
    const auto s_no_baseline = currentValue - singleBaseline;
    const auto re_filtered = high->sfilt(s_no_baseline);
    const int16_t currentBaseline = round(singleBaseline / currentRange.step);

    if (++baselineSamplingRateCounter >= baselineSamplingRate) {
        baseline.push_back(currentBaseline);
        baselineStimulus.push_back(voltage);
        baselineSamplingRateCounter = 0;
    }

    // threshold is not initialized yet
    if (threshold == -1) {
        bandPassFilterData.push_back(re_filtered);
        return std::nullopt;
    }

    bool isEvent = false;
    switch (eventsDirection)
    {
    case DOWN:
        isEvent = re_filtered < -threshold;
        break;
    case UP:
        isEvent = re_filtered > threshold;
        break;
    case BOTH:
        //NOT IMPLEMENTED YET
        break;
    default:
        break;
    }
    if (isEvent && !eventAlreadyBegun) {
        eventAlreadyBegun = true;
        eventBeginIdx = idx;
        return std::nullopt;
    }
    if (isEvent && eventAlreadyBegun) {
        eventLen++;
        low->init(lowParams.second);
        return std::nullopt;
    }
    bandPassFilterData.push_back(re_filtered);
    if (!eventAlreadyBegun) {
        return std::nullopt;
    }
    //event already begun
    if (eventLen >= maxEventLen) {
        low->init(currentValue);
        eventAlreadyBegun = false;
        return std::nullopt;
    }
    if (abs(re_filtered) > abs(maxAmplitude)) {
        eventAlreadyBegun = false;
        eventLen = 0;
        return std::nullopt;
    }
    if (eventLen > minEventLen) {
        const auto e0 = eventBeginIdx - (EVENT_PADDING * eventLen);
        const auto e1 = eventBeginIdx + eventLen + (EVENT_PADDING * eventLen);
        eventAlreadyBegun = false;
        const PartialEvent res = { (e0 < 0) ? 0 : e0, (e1 >= clipValue) ? clipValue - 1 : e1, eventLen, currentBaseline };
        eventLen = 0;
        return res;
    }
    eventAlreadyBegun = false;
    eventLen = 0;
    return std::nullopt;
}

void EventDetector::setChunk(std::vector<int16_t> intBuffer, std::vector<double> doubleBuffer, std::vector<double> voltages, uint32_t chunkSize, RangedMeasurement currentRange, RangedMeasurement voltageRange, Measurement samplingRate) {
    //TODO, if sampling rate changes rebuild the filters
    //managing remaining stuff from older chunk
    chunkSize += remainingChunkSize;
    const auto oldTh = threshold;
    if (oldTh == -1 && doubleBuffer.size() > 0) {
        low->init(doubleBuffer[0]);
        high->init(doubleBuffer[0]);
    }
    intBuffer.insert(intBuffer.begin(), remainingIntBuffer.begin(), remainingIntBuffer.end());
    doubleBuffer.insert(doubleBuffer.begin(), remainingDoubleBuffer.begin(), remainingDoubleBuffer.end());
    voltages.insert(voltages.begin(), remainingVoltages.begin(), remainingVoltages.end());

    this->chunkSize = chunkSize;
    threshold = calculateThreshold(bandPassFilterData);
    this->currentRange = currentRange;
    this->voltageRange = voltageRange;
    this->samplingRate = samplingRate;
    //now that we have an updated value throw away the old ones
    bandPassFilterData.clear();
    eventAlreadyBegun = false;
    eventLen = 0;
    eventBeginIdx = 0;

    const auto finalPadding = maxEventLen * EVENT_PADDING;
    //received chunk smaller than event * padding
    if (chunkSize < finalPadding) {
        remainingChunkSize = chunkSize;
        remainingIntBuffer = intBuffer;
        remainingDoubleBuffer = doubleBuffer;
        remainingVoltages = voltages;
        return;
    }
    const auto earlyStop = chunkSize - finalPadding;
    for (uint32_t idx = 0; idx < earlyStop; idx++) {
        const auto currentValue = doubleBuffer[idx];
        const auto voltage = voltages[idx];
        const auto optEvent = analyze(currentValue, voltage, idx, chunkSize);
        if (optEvent.has_value()) {
            const auto& event = optEvent.value();
            processEvent(event, intBuffer, voltages[idx], chunkSize);
        }
    }
    //Try to stop early
    if (eventAlreadyBegun) {
        //if an event is being process continue unitl it has been processed
        uint32_t idx = earlyStop;
        while (idx < chunkSize) {
            const auto currentValue = doubleBuffer[idx];
            const auto voltage = voltages[idx];
            const auto optEvent = analyze(currentValue, voltage, idx, chunkSize);
            if (optEvent.has_value()) {
                const auto& event = optEvent.value();
                processEvent(event, intBuffer, voltages[idx], chunkSize);
                break;
            }
            idx++;
        }
        timeCount += idx;
        remainingChunkSize = chunkSize - idx;
        remainingIntBuffer.resize(remainingChunkSize);
        remainingDoubleBuffer.resize(remainingChunkSize);
        remainingVoltages.resize(remainingChunkSize);
        std::copy(intBuffer.begin() + idx, intBuffer.begin() + chunkSize, remainingIntBuffer.begin());
        std::copy(doubleBuffer.begin() + idx, doubleBuffer.begin() + chunkSize, remainingDoubleBuffer.begin());
        std::copy(voltages.begin() + idx, voltages.begin() + chunkSize, remainingVoltages.begin());
    } else {
        timeCount += earlyStop;
        remainingChunkSize = finalPadding;
        remainingIntBuffer.resize(remainingChunkSize);
        remainingDoubleBuffer.resize(remainingChunkSize);
        remainingVoltages.resize(remainingChunkSize);
        std::copy(intBuffer.begin() + earlyStop, intBuffer.begin() + chunkSize, remainingIntBuffer.begin());
        std::copy(doubleBuffer.begin() + earlyStop, doubleBuffer.begin() + chunkSize, remainingDoubleBuffer.begin());
        std::copy(voltages.begin() + earlyStop, voltages.begin() + chunkSize, remainingVoltages.begin());
    }
}

void EventDetector::processEvent(const PartialEvent partialEvent, std::vector<int16_t>& intBuffer, double voltage, uint32_t chunkSize) {
    const auto eventBegin = partialEvent.eventBegin;
    const auto eventEnd = partialEvent.eventEnd;
    const auto realLen = partialEvent.realLen;
    const auto baseline = partialEvent.baseline;
    const uint32_t eventLen = eventEnd - eventBegin;
    if (eventEnd >= chunkSize || eventBegin > eventEnd) {
        return;
    }
    int16_t min = INT16_MAX;
    int16_t max = INT16_MIN;
    std::vector<int16_t> eventBuffer(eventLen);
    for (uint32_t i = 0; i < eventLen; i++) {
        const auto v = intBuffer[i + eventBegin] - baseline;
        eventBuffer[i] = v;
        if (v < min) { min = v; };
        if (v > max) { max = v; };
    }
    const Event e = Event(timeCount + eventBegin, eventBuffer, voltage, voltageRange.getFullUnit(), currentRange.step, currentRange.getFullUnit(), samplingRate.getNoPrefixValue(), samplingRate.unit, currentRange.multiplier(), voltageRange.multiplier());
    const EventInfo ei = EventInfo(((double)std::abs(max - min)) * currentRange.step, ((double) realLen) / samplingRate.getNoPrefixValue(), e);
    eventsInfo.push_back(ei);
}

void EventDetector::clear() {
    eventsInfo.clear();
}

void EventDetector::setMinEventDurationInSamples(uint32_t minDuration) {
    minEventLen = minDuration;
}

void EventDetector::setMaxEventDurationInSamples(uint32_t maxDuration) {
    maxEventLen = maxDuration;
}

void EventDetector::setHighCutoffFrquency(double cf) {
    highCutoffFrequency = cf;
    delete high;
    //TODO, think this better
    high = new FirstOrderIirFilter(samplingRate.value, highCutoffFrequency);
}

void EventDetector::setStdMultiplier(double newValue) {
    stdMultiplier = newValue;
}

double EventDetector::getMaxAmplitude() {
    return maxAmplitude;
}

void EventDetector::setMaxAmplitude(double maxAmplitude) {
    this->maxAmplitude = maxAmplitude;
}

void EventDetector::setEventsDirection(EventsDirection ed) {
    this->eventsDirection = ed;
}
