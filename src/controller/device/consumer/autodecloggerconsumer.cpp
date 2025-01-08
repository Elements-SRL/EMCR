#include "autodecloggerconsumer.h"
#include <QTime>

void AutodecloggerConsumer::onStartConsuming() {
    hook = producer->getDataHook();
    if (hook != nullptr) {
        QMutexLocker consumptionLock(&consumptionMtx);
        consumptionStopped = false;
        exitedDataConsumingLoop = false;

        this->start();
    }
}

void AutodecloggerConsumer::onStopConsuming() {
    if (this->isRunning()) {
        QMutexLocker consumptionLock(&consumptionMtx);
        consumptionStopped = true;
        while (!exitedDataConsumingLoop) {
            exitedDataConsumingLoopCv.wait(&consumptionMtx, 100);
        }
    }

    if (hook != nullptr) {
        delete hook;
        hook = nullptr;
    }
}

void AutodecloggerConsumer::onSamplingRateChanged(Measurement_t samplingRate) {
    QMutexLocker locker(&timeAxisMtx);
    samplingRate.convertValue(UnitPfxNone);
    pushedSamplingRateHz = samplingRate.value;
    pushedSamplingRateFlag = true;
}

void AutodecloggerConsumer::onDownsamplingRatioChanged(unsigned int ratio) {
    QMutexLocker locker(&timeAxisMtx);
    pushedDownsamplingRatio = ratio;
    pushedDownsamplingRatioFlag = true;
}

void AutodecloggerConsumer::onVoltageRangeChanged(RangedMeasurement_t range) {
    QMutexLocker locker(&rangeAxisMtx);
    pushedVoltageRange = range;
    pushedVoltageRangeFlag = true;
}

void AutodecloggerConsumer::onCurrentRangeChanged(RangedMeasurement_t range) {
    QMutexLocker locker(&rangeAxisMtx);
    pushedCurrentRange = range;
    pushedCurrentRangeFlag = true;
}

AutodecloggerConsumer::AutodecloggerConsumer(ApplicationStatus* appStatus, DeviceDataProducer* producer):
    DeviceDataConsumer(appStatus, producer) {
    for (int i = 0; i < currentChannelsNum; i++) {
        currentValues.push_back({});
        timers.push_back(std::nullopt);
        originalStimului.push_back(0.0);
    }
}
AutodecloggerConsumer::~AutodecloggerConsumer() {
    currentValues.clear();
    timers.clear();
    buffer.clear();
    originalStimului.clear();
    model = nullptr;
    delete model;
}

void AutodecloggerConsumer::run() {
    consumptionStopped = false;
    exitedDataConsumingLoop = false;
    int bufferIdx;
    int bufferLen = 0;
    int channelIdx;
    QElapsedTimer updateDataTimer;
    updateDataTimer.start();

    int lastUpdateTimeMs = updateDataTimer.elapsed();
    int currentTimeMs;

    QMutexLocker consumptionLock(&consumptionMtx);
    consumptionLock.unlock();

    while (true) {
        consumptionLock.relock();
        if (consumptionStopped) {
            consumptionLock.unlock();
            break;
        }
        consumptionLock.unlock();
        if (hook->getDataChunk(buffer, 1, minDataBatchSize)) {
            bufferIdx = 0;
            bufferLen = buffer.size();

            while (bufferIdx + voltageChannelsNum < bufferLen) {
                for (channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                    const auto currentValue = buffer[bufferIdx + voltageChannelsNum];
                    currentValues[channelIdx].push_back(currentValue);
                    bufferIdx++;
                }
                bufferIdx += currentChannelsNum;
            }
            currentTimeMs = updateDataTimer.elapsed();
            std::vector<int> completed = {};
            std::vector<int> started = {};
            if (currentTimeMs - lastUpdateTimeMs > MIN_UPDATE_PLOT_TIME_MS) {
                for (int i = 0; i < currentChannelsNum; i++) {
                    if (timers[i].has_value()) {
                        if (timers[i].value()->elapsed() > model->msTimes[i]) {
                            auto msgDisp = appStatus->getMessageDispatcher();
                            completed.push_back(i);
                        }
                    }
                    else {
                        const auto nElements = currentValues[i].size();
                        auto sum = 0.0;
                        for (auto &v: currentValues[i]) {
                            sum += v;
                        }
                        if (nElements == 0) {
                            break;
                        }
                        const auto avg = sum / (double)nElements;
                        const auto absAvg = abs(avg);
                        if (absAvg < model->thresholds[i]) {

                            
                        }
                    }
                }
                if (completed.size() > 0) {
                    //come back to original stimulus
                    //signal that this channel is not declogging anymore
                    emit sigDecloggingCompleted(completed);
                }
                if (started.size() > 0) {
                    // get voltage
                    // add voltage to old voltages
                    //set voltage
                    //init timer
                    emit sigDecloggingStarted(started);
                }
                lastUpdateTimeMs = currentTimeMs;
                //this data has already been analyzed, so I can get rid of it
                for (auto c : currentValues) {
                    c.clear();
                }
            }
        }
    }
    consumptionLock.relock();
    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

void AutodecloggerConsumer::setModel(AutoDecloggerModel* model) {
    this->model = model;
}
