#include "eventdetectionconsumer.h"
#include <QTime>
#include <iostream>
#include <QDebug>

EventDetectionConsumer::EventDetectionConsumer(ApplicationStatus* appStatus, DeviceDataProducer* producer) :
    PlotConsumer(appStatus, producer) {
    minDataBatchSize = currentChannelsNum * appStatus->getSamplingRate().value * MINIMUM_DATA_FOR_ANALYSIS;
    intBuffer.reserve(producer->getDataPacketsBufferLen() * totalChannelsNum);

    this->nBins = 3201;
    calculateBinSize();
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
    calculateBinSize();
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
            if (currentTimeMs - lastUpdateTimeMs > PCS_MIN_UPDATE_PLOT_TIME_MS) {
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

// Function to scale a value into a number of bins
int EventDetectionConsumer::scaleToBins(double value) {
    // Calculate the adjusted value to lie within [0, 2*v]
    return static_cast<int>((value - pushedVoltageRange.min) / binSize);
}

void EventDetectionConsumer::allocateData() {
    bool wasThisRunning = isRunning();
    onStopConsuming();
    clearData();

    double* precalculatedVoltages = new double[nBins];
    for (int i = 0; i < nBins; i++) {
        precalculatedVoltages[i] = ((double)i) * binSize + pushedVoltageRange.min;
    }
    voltageData.resize(currentChannelsNum);
    for (int i = 0; i < currentChannelsNum; i++) {
        voltageData[i] = new double[nBins];
    }
    dataSize.resize(currentChannelsNum);
    officialDataSize.resize(currentChannelsNum);
    voltageBins.resize(nBins);
    for (int i = 0; i < nBins; i++) {
        voltageBins[i] = ((double)i) * binSize + pushedVoltageRange.min;
    }
    for (int idx = 0; idx < this->currentChannelsNum; idx++) {
        currentValuesInt.push_back(std::vector<int16_t>(maxSamples));
        currentValuesDouble.push_back(std::vector<double>(maxSamples));
        const Measurement fakeMeasurement = { 1.0e6, UnitPfxNone, "s" };
        eventDetectionChannels.push_back(new EventDetector(fakeMeasurement));
    }
    for (int idx = 0; idx < this->voltageChannelsNum; idx++) {
        voltageValues.push_back(std::vector<double>(maxSamples));
    }
    if (wasThisRunning) {
        onStartConsuming();
    }
}

//todo call this method when bin size changes or when voltage range changes
void EventDetectionConsumer::calculateBinSize() {
    // Calculate the size of each bin
    binSize = pushedVoltageRange.delta() / ((double)(nBins - 1));
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
    for (int i = 0; i < voltageData.size(); i++) {
        delete[] voltageData[i];
    }
    voltageData.clear();
    voltageBins.clear();
    dataSize.clear();
    officialDataSize.clear();
}

void EventDetectionConsumer::onSamplingRateChanged(Measurement_t samplingRate) {
    PlotConsumer::onSamplingRateChanged(samplingRate);
    minDataBatchSize = samplingRate.value * currentChannelsNum * MINIMUM_DATA_FOR_ANALYSIS;
}
