#include "eventdetectionconsumer.h"
#include <QTime>
#include <iostream>
#include <QDebug>

EventDetectionConsumer::EventDetectionConsumer(ApplicationStatus* appStatus, DeviceDataProducer* producer) :
    PlotConsumer(appStatus, producer) {
    minDataBatchSize = currentChannelsNum * appStatus->getSamplingRate().value * MINIMUM_DATA_FOR_ANALYSIS;

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
    std::vector<double> eventBuffer(minDataBatchSize);
    bool atLeastOneFound = false;
    while (true) {
        consumptionLock.relock();
        if (consumptionStopped) {
            break;
        }
        consumptionLock.unlock();
        if (hook->getDataChunk(buffer, subSamplingRatio, minDataBatchSize)) {
            for (int i = 0; i < currentChannelsNum; i++) {
                currentValues[i].clear();
            }
            idx = 0;
            bufferIdx = 0;
            bufferLen = buffer.size();
            /*! Copy data in curves */
            while (bufferIdx < bufferLen) {
                //let's ignore voltages for now
                bufferIdx += voltageChannelsNum;
                //currents
                for (channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                    if (plottedChannels[channelIdx]) {
                        //current copied in currentValues
                        currentValues[channelIdx].push_back(buffer[bufferIdx]);
                    }
                    bufferIdx++;
                }
                idx++;
            }

            for (channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                if (plottedChannels[channelIdx]) {
                    const auto valuesSize = currentValues[channelIdx].size();
                    for (idx = 0; idx < valuesSize; idx++) {
                        const auto currentValue = currentValues[channelIdx][idx];
                        const auto optEvent = eventDetectionChannels[channelIdx]->analyze(currentValue, idx, valuesSize);
                        if (optEvent.has_value()) {
                            atLeastOneFound = true;
                            //qDebug() << "found";
                            const auto event = optEvent.value();
                            processEvent(event, channelIdx, eventBuffer);
                        }
                    }
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
        currentValues.push_back(std::vector<double>(maxSamples));
        eventDetectionChannels.push_back(new EventDetector());
    }
    if (wasThisRunning) {
        onStartConsuming();
    }
}

void EventDetectionConsumer::processEvent(std::pair<int, int> evtBegingEnd, uint32_t chIdx, std::vector<double>& eventBuffer) {
    const uint32_t size = currentValues[chIdx].size();
    const uint32_t eventBegin = evtBegingEnd.first;
    const uint32_t eventEnd = evtBegingEnd.second;
    const uint32_t eventLen = eventEnd - eventBegin;
    if (eventBegin < 0 || eventEnd >= size || eventBegin > eventEnd) {
        return;
    }
    eventBuffer.clear();
    eventBuffer.resize(eventLen);
    for (uint32_t i = 0; i < eventLen; i++) {
        eventBuffer[i] = currentValues[chIdx][i+eventBegin];
    }
    ////WARNING MODIFY THIS WITH THE time counter
    const auto eventIdx = eventLen;
    eventDetectionChannels[chIdx]->pushEvent(Event(eventIdx, eventBuffer));
}

//todo call this method when bin size changes or when voltage range changes
void EventDetectionConsumer::calculateBinSize() {
    // Calculate the size of each bin
    binSize = pushedVoltageRange.delta() / ((double)(nBins - 1));
}

void EventDetectionConsumer::emitPlotData() {
    std::map<uint32_t, std::vector<Event>> events;
    for (int chIdx = 0; chIdx < currentChannelsNum; chIdx++) {
        events[chIdx] = eventDetectionChannels[chIdx]->getEvents();
    }
    EventDetectionMessage message = { events };
    emit setPlotData(message);
    for (int chIdx = 0; chIdx < currentChannelsNum; chIdx++) {
        eventDetectionChannels[chIdx]->clear();
    }
}

void EventDetectionConsumer::clearData() {
    currentValues.clear();
    eventDetectionChannels.clear();
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
