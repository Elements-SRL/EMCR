#include "eventdetectionconsumer.h"
#include <QTime>
#include <iostream>

EventDetectionConsumer::EventDetectionConsumer(ApplicationStatus* appStatus, DeviceDataProducer* producer) :
    PlotConsumer(appStatus, producer) {
    //check abfdatawriter for a suggest size
    //minDataBatchSize = ;

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

    while (true) {
        consumptionLock.relock();
        if (consumptionStopped) {
            break;
        }
        consumptionLock.unlock();
        if (hook->getDataChunk(buffer, subSamplingRatio, minDataBatchSize)) {
            idx = 0;
            bufferIdx = 0;
            bufferLen = buffer.size();

            /*! Copy data in curves */
            while (bufferIdx < bufferLen) {
                //let's ignore voltages for now
                bufferIdx += voltageChannelsNum;
                //currents
                for (channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                    currentValues[channelIdx].clear();
                    if (plottedChannels[channelIdx]) {
                        const auto currentValue = buffer[bufferIdx];
                        //current copied in currentValues
                        currentValues[channelIdx].push_back(currentValue);
                        const auto optEvent = eventDetectionChannels[channelIdx]->analyze(currentValue, idx, bufferLen);
                        if (optEvent.has_value()) {
                            const auto event = optEvent.value();
                            const auto eventBegin = event.first;
                            const auto eventEnd = event.second;
                            const auto eventLen = eventEnd - eventBegin;
                            eventBuffer.clear();
                            for (int i = eventBegin; i < eventEnd; i++) {
                                eventBuffer.push_back(currentValues[channelIdx][i]);
                            }
                            //WARNING MODIFY THIS WITH THE time counter
                            const auto eventIdx = bufferLen - eventLen;
                            const auto eventData(eventBuffer);
                            eventDetectionChannels[channelIdx]->pushEvent(Event(eventIdx, eventData));
                        }
                    }
                    bufferIdx++;
                }

                idx++;
            }

            /*currentTimeMs = updateDataTimer.elapsed();
            if (currentTimeMs - lastUpdateTimeMs > PCS_MIN_UPDATE_PLOT_TIME_MS) {
                for (int i = 0; i < currentChannelsNum; i++) {
                    int counterOfSomeVariant = 0;
                    for (int c_idx = 0; c_idx < currents.size(); c_idx++) {
                        const auto current = currents[c_idx];
                        if (current.has_value()) {
                            voltageData[i][counterOfSomeVariant] = voltageBins[c_idx];
                            currentValues[i][counterOfSomeVariant] = current.value();
                            counterOfSomeVariant++;
                        }
                    }
                    dataSize[i] = counterOfSomeVariant;
                }
                bool isDataChanged = false;
                for (int i = 0; i < currentChannelsNum; i++) {
                    if (dataSize[i] != officialDataSize[i]) {
                        officialDataSize[i] = dataSize[i];
                        isDataChanged = true;
                    }
                }
                if (isDataChanged) {
                    emitPlotData();
                }
                emit plotDataUpdated();
                lastUpdateTimeMs = currentTimeMs;
            }*/
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

//todo call this method when bin size changes or when voltage range changes
void EventDetectionConsumer::calculateBinSize() {
    // Calculate the size of each bin
    binSize = pushedVoltageRange.delta() / ((double)(nBins - 1));
}

void EventDetectionConsumer::emitPlotData() {
    //IvMessage message = { voltageData, currentValues, dataSize };
    //emit setPlotData(message);
}

void EventDetectionConsumer::clearData() {
    currentValues.clear();
    for (int i = 0; i < voltageData.size(); i++) {
        delete[] voltageData[i];
    }
    voltageData.clear();
    voltageBins.clear();
    dataSize.clear();
    officialDataSize.clear();
}
