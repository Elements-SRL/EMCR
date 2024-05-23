#include "eventdetector.h"
#include <cmath>
#include <iostream>
#include <cstdint>

EventDetector::EventDetector(Measurement samplingRate, int sizeHint) {
    const auto lowCutoffFrequency = 100.0;
    high = new FirstOrderIirFilter(samplingRate.value, 250.0e3);
    low = new FirstOrderIirFilter(samplingRate.value, lowCutoffFrequency);
    if (sizeHint != -1) {
        eventsInfo.reserve(sizeHint);
    }
    baselineSamplingRate = samplingRate.getNoPrefixValue() / (lowCutoffFrequency * 5.0);
    baselineSamplingRateCounter = 0;
}

EventPacket EventDetector::consumeEventsAndBaseline() {
    uint32_t acc = 0;
    uint32_t numberOfEvents = eventsInfo.size();
    for (auto& ei : eventsInfo) {
        acc += ei.event.rawData.size();
    }
    const auto b = Baseline(currentRange.step, baseline, currentRange.unit);
    const auto eps = (prevEventPerSecond * 0.9) + (((double)numberOfEvents) * (samplingRate.getNoPrefixValue() / ((double)chunkSize)) * 0.1);
    prevEventPerSecond = std::isnan(eps) ? 0.0 : eps;
    const EventPacket ep = EventPacket(eventsInfo, b, prevEventPerSecond);
    eventsInfo.clear();
    baseline.clear();
    return ep;
}

double EventDetector::calculateThreshold(const std::vector<double>& data) {
    const auto stdDev = calcStdDev(data);
    return stdDev != -1 ? EVENT_TH * stdDev : stdDev;
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

std::optional<std::tuple<uint32_t, uint32_t, uint32_t>> EventDetector::analyze(double currentValue, uint32_t idx, uint32_t clipValue) {
    //begin event analysis
    const auto lowParams = low->getParams();
    const auto singleBaseline = low->sfilt(currentValue);
    const auto s_no_baseline = currentValue - singleBaseline;
    const auto re_filtered = high->sfilt(s_no_baseline);

    if (++baselineSamplingRateCounter >= baselineSamplingRate) {
        baseline.push_back(singleBaseline / currentRange.step);
        baselineSamplingRateCounter = 0;
    }

    // threshold is not initialized yet
    if (threshold == -1) {
        bandPassFilterData.push_back(re_filtered);
        return std::nullopt;
    }

    // WARNING THERE IS A - BEFORE THRESHOLD, ONLY WORKS WITH EVENTS GOING DOWN
    const auto isEvent = re_filtered < -threshold;
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
    if (eventLen >= MAX_LEN) {
        low->init(currentValue);
        eventAlreadyBegun = false;
        return std::nullopt;
    }
    if (eventLen > MIN_LEN) {
        const auto e0 = eventBeginIdx - (EVENT_PADDING * eventLen);
        const auto e1 = eventBeginIdx + eventLen + (EVENT_PADDING * eventLen);
        eventAlreadyBegun = false;
        const std::tuple<uint32_t, uint32_t, uint32_t> res((e0 < 0) ? 0 : e0, (e1 >= clipValue) ? clipValue - 1 : e1, eventLen);
        eventLen = 0;
        return res;
    }
    eventAlreadyBegun = false;
    eventLen = 0;
}

void EventDetector::setChunk(std::vector<int16_t> intBuffer, std::vector<double> doubleBuffer, std::vector<double> voltages, uint32_t chunkSize, RangedMeasurement currentRange, RangedMeasurement voltageRange, Measurement samplingRate) {
    //TODO, For now just reinit everything
    //TODO, if sampling rate changes rebuild the filters
    this->chunkSize = chunkSize;
    const auto oldTh = threshold;
    threshold = calculateThreshold(bandPassFilterData);
    this->currentRange = currentRange;
    this->voltageRange = voltageRange;
    this->samplingRate = samplingRate;
    //now that we have an updated value throw away the old ones
    bandPassFilterData.clear();
    eventAlreadyBegun = false;
    eventLen = 0;
    eventBeginIdx = 0;
    if (oldTh == -1 && doubleBuffer.size() > 0) {
        low->init(doubleBuffer[0]);
    }
    for (uint32_t idx = 0; idx < chunkSize; idx++) {
        const auto currentValue = doubleBuffer[idx];
        const auto optEvent = analyze(currentValue, idx, chunkSize);
        if (optEvent.has_value()) {
            const auto& event = optEvent.value();
            processEvent(event, intBuffer, voltages[idx], chunkSize);
        }
    }
    timeCount += chunkSize;
}

void EventDetector::processEvent(const std::tuple<uint32_t, uint32_t, uint32_t> evtBegingEndLen, std::vector<int16_t>& intBuffer, double voltage, uint32_t chunkSize) {
    const uint32_t eventBegin = std::get<0>(evtBegingEndLen);
    const uint32_t eventEnd = std::get<1>(evtBegingEndLen);
    const auto realLen = std::get<2>(evtBegingEndLen);
    const uint32_t eventLen = eventEnd - eventBegin;
    if (eventEnd >= chunkSize || eventBegin > eventEnd) {
        return;
    }
    int16_t min = INT16_MAX;
    int16_t max = INT16_MIN;
    std::vector<int16_t> eventBuffer(eventLen);
    for (uint32_t i = 0; i < eventLen; i++) {
        const auto v = intBuffer[i + eventBegin];
        eventBuffer[i] = v;
        if (v < min) { min = v; };
        if (v > max) { max = v; };
    }
    const Event e = Event(timeCount + eventBegin, eventBuffer, voltage, voltageRange.getFullUnit(), currentRange.step, currentRange.getFullUnit(), samplingRate.value, samplingRate.getFullUnit());
    const EventInfo ei = EventInfo(((double)std::abs(max - min)) * currentRange.step, ((double) realLen) / samplingRate.getNoPrefixValue(), e);
    eventsInfo.push_back(ei);
}

void EventDetector::clear() {
    eventsInfo.clear();
}
