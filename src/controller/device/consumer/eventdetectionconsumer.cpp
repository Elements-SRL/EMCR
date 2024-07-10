#include "eventdetectionconsumer.h"
#include <QTime>
#include <iostream>

EventDetectionConsumer::EventDetectionConsumer(ApplicationStatus* appStatus, DeviceDataProducer* producer, uint32_t minEventSamples_, uint32_t maxEventSamples_, double highCutoffFrequency_, double defaultStdMultiplier_):
    PlotConsumer(appStatus, producer), minEventSamples(minEventSamples_), maxEventSamples(maxEventSamples_), highCutoffFrequency(highCutoffFrequency_) , stdMultiplier(defaultStdMultiplier_){
    minDataBatchSize = currentChannelsNum * appStatus->getSamplingRate().getNoPrefixValue() * MINIMUM_DATA_FOR_ANALYSIS;
    intBuffer.reserve(producer->getDataPacketsBufferLen() * totalChannelsNum);
    allocateData();
}

EventDetectionConsumer::~EventDetectionConsumer() {
    this->onStopConsuming();
    this->clearData();
}

void EventDetectionConsumer::forceAxisUpdate() {
    pushedCurrentRangeFlag = true;
    updateRangeAxis();
    emitPlotData();
}

void EventDetectionConsumer::onVoltageRangeChanged(RangedMeasurement_t range) {
    PlotConsumer::onVoltageRangeChanged(range);
    allocateData();
    emitPlotData();
}

void EventDetectionConsumer::run() {
    consumptionStopped = false;
    exitedDataConsumingLoop = false;
    emitPlotData();
    uint32_t bufferIdx;
    int bufferLen = 0;
    int channelIdx;
    QTime updateDataTimer = QTime::currentTime();
    updateDataTimer.start();

    int lastUpdateTimeMs = updateDataTimer.elapsed();
    int currentTimeMs;

    //event detection stuff
    std::vector<std::pair<int, int>> events;
    auto const size = buffer.size();
    uint32_t idx = 0;
    std::vector<double> high_pass(size);

    QMutexLocker consumptionLock(&consumptionMtx);
    consumptionLock.unlock();

    //vector used only to accomodate events
    std::vector<int16_t> eventBuffer(minDataBatchSize);
    bool atLeastOneFound = false;
    while (true) {
        consumptionLock.relock();
        if (consumptionStopped) {
            break;
        }
        consumptionLock.unlock();
        if (hook->getDataChunks(doubleBuffer, intBuffer, minDataBatchSize)) {
            for (int i = 0; i < currentChannelsNum; i++) {
                currentValuesInt[i].clear();
                currentValuesDouble[i].clear();
                voltageValues[i].clear();
            }
            bufferIdx = 0;
            bufferLen = doubleBuffer.size();
            /*! Copy data in curves */
            while (bufferIdx < bufferLen) {
                //voltages
                for (channelIdx = 0; channelIdx < voltageChannelsNum; channelIdx++) {
                    if (plottedChannels[channelIdx]) {
                        voltageValues[channelIdx].push_back(doubleBuffer[bufferIdx]);
                    }
                    bufferIdx++;
                }
                //currents
                for (channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                    if (plottedChannels[channelIdx]) {
                        //current copied in currentValues
                        currentValuesInt[channelIdx].push_back(intBuffer[bufferIdx]);
                        currentValuesDouble[channelIdx].push_back(doubleBuffer[bufferIdx]);
                    }
                    bufferIdx++;
                }
            }
            for (channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                if (plottedChannels[channelIdx]) {
                    const auto valuesSize = currentValuesDouble[channelIdx].size();
                    eventDetectionChannels[channelIdx]->setChunk(currentValuesInt[channelIdx], currentValuesDouble[channelIdx], voltageValues[channelIdx], valuesSize, currentRange, voltageRange, appStatus->getSamplingRate());
                }
            }
            currentTimeMs = updateDataTimer.elapsed();
            if (currentTimeMs - lastUpdateTimeMs > PCS_MIN_UPDATE_PLOT_TIME_MS && atLeastOneFound) {
                emitPlotData();
                emit plotDataUpdated();
                lastUpdateTimeMs = currentTimeMs;
                atLeastOneFound = false;
            }
        }
    }
    emit plotDataUpdated();
    consumptionLock.relock();

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

void EventDetectionConsumer::allocateData() {
    bool wasThisRunning = isRunning();
    onStopConsuming();
    clearData();

    dataSize.resize(currentChannelsNum);
    for (int idx = 0; idx < this->currentChannelsNum; idx++) {
        currentValuesInt.push_back(std::vector<int16_t>(maxSamples));
        currentValuesDouble.push_back(std::vector<double>(maxSamples));
        auto sr = appStatus->getSamplingRate();
        eventDetectionChannels.push_back(new EventDetector(sr, highCutoffFrequency, minEventSamples, maxEventSamples, stdMultiplier));
    }
    for (int idx = 0; idx < this->voltageChannelsNum; idx++) {
        voltageValues.push_back(std::vector<double>(maxSamples));
    }
    if (wasThisRunning) {
        onStartConsuming();
    }
}

void EventDetectionConsumer::emitPlotData() {
    std::map<uint32_t, EventPacket> eventPackets;
    for (uint32_t chIdx = 0; chIdx < currentChannelsNum; chIdx++) {
        if (eventDetectionChannels[chIdx]) {
            eventPackets.emplace(
                chIdx,
                eventDetectionChannels[chIdx]->consumeEventsAndBaseline()
            );
        }
    }
    EventDetectionMessage message = { eventPackets };
    emit setPlotData(message);
}

void EventDetectionConsumer::clearData() {
    currentValues.clear();
    eventDetectionChannels.clear();
    currentValuesDouble.clear();
    currentValuesInt.clear();
    voltageValues.clear();
    dataSize.clear();
}

void EventDetectionConsumer::onSamplingRateChanged(Measurement_t samplingRate) {
    PlotConsumer::onSamplingRateChanged(samplingRate);
    minDataBatchSize = samplingRate.value * currentChannelsNum * MINIMUM_DATA_FOR_ANALYSIS;
}

void EventDetectionConsumer::setMinEventDurationInSamples(uint32_t newValue) {
    minEventSamples = newValue;
    for (const auto& ed : eventDetectionChannels) {
        ed->setMinEventDurationInSamples(minEventSamples);
    }
}

void EventDetectionConsumer::setMaxEventDurationInSamples(uint32_t newValue) {
    maxEventSamples = newValue;
    for (const auto& ed : eventDetectionChannels) {
        ed->setMaxEventDurationInSamples(maxEventSamples);
    }
}

void EventDetectionConsumer::setHighCutoffFrequency(double newValue) {
    highCutoffFrequency = newValue;
    for (const auto& ed : eventDetectionChannels) {
        ed->setHighCutoffFrquency(highCutoffFrequency);
    }
}

void EventDetectionConsumer::setStdMultiplier(double newValue) {
    stdMultiplier = newValue;
    for (const auto& ed : eventDetectionChannels) {
        ed->setStdMultiplier(stdMultiplier);
    }
}
