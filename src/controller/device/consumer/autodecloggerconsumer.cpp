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
        currentValues[i] = {};
    }
}

AutodecloggerConsumer::~AutodecloggerConsumer() {
    currentValues.clear();
    timers.clear();
    buffer.clear();
    originalVoltages.clear();
    tunerResetValues.clear();
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
                    // if there is no previous voltage, read it
                    // could this have problems if I read old voltages?
                    if (!originalVoltages[channelIdx].has_value()) {
                        originalVoltages[channelIdx] = buffer[bufferIdx];
                    }
                    const auto currentValue = buffer[bufferIdx + voltageChannelsNum];
                    currentValues[channelIdx].push_back(currentValue);
                    bufferIdx++;
                }
                bufferIdx += currentChannelsNum;
            }
            currentTimeMs = updateDataTimer.elapsed();
            std::vector<unsigned short> toStart, toComplete = {};
            if (currentTimeMs - lastUpdateTimeMs > MIN_UPDATE_PLOT_TIME_MS) {
                for (int i = 0; i < currentChannelsNum; i++) {
                    if (timers.count(i) == 1 && timers[i].has_value()) {
                        if (timers[i].value()->elapsed() > model->msTimes[i]) {
                            auto msgDisp = appStatus->getMessageDispatcher();
                            toComplete.push_back(i);
                            timers[i] = std::nullopt;
                            originalVoltages[i] = std::nullopt;
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
                            toStart.push_back(i);
                        }
                    }
                }
                lastUpdateTimeMs = currentTimeMs;
                //this data has already been analyzed, so I can get rid of it
                for (auto &[k, v] : currentValues) {
                    v.clear();
                }
            }
            if (toComplete.size() > 0) {
                //come back to original stimulus
                std::vector<Measurement> resetValues;
                for (auto& ch : toComplete) {
                    const auto v = tunerResetValues[ch];
                    if (v.has_value()) {
                        resetValues.push_back(v.value());
                    }
                }
                appStatus->getMessageDispatcher()->setVoltageHoldTuner(toComplete, resetValues, true);
                //signal that this channel is not declogging anymore
                emit sigDecloggingCompleted(toComplete);
            }
            if (toStart.size() > 0) {
                const auto vr = appStatus->getVoltageRange();
                std::vector<Measurement> voltages, tuners, voltagesToApply;
                std::vector<int> chIndexes;
                appStatus->getMessageDispatcher()->getVoltageHoldTuner(tuners);
                for (const auto& ch : toStart) {
                    tunerResetValues[ch] = tuners[ch];
                    if (originalVoltages[ch].has_value()) {
                        const Measurement v = { originalVoltages[ch].value(), vr.prefix, "V" };
                        voltages.push_back(v);
                        chIndexes.push_back(ch);
                    }
                }

                for (int vCh = 0; vCh < appStatus->getVoltageChannelsNum(); vCh++) {
                    const auto vNoTuner = voltages[vCh] - tuners[vCh];
                    const Measurement vToApply = { vNoTuner.value - model->stimuli[vCh], vNoTuner.prefix, vNoTuner.unit };
                    voltagesToApply.push_back(vToApply);
                    timers[vCh] = new QElapsedTimer;
                }
                //set voltages to declog the pore
                appStatus->getMessageDispatcher()->setVoltageHoldTuner(toStart, voltagesToApply, true);
                // init timer
                for (const auto& [key, value] : timers) {
                    if (value.has_value()) {
                        value.value()->start();
                    }
                }
                emit sigDecloggingStarted(toStart);
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

template <typename Lambda>
void AutodecloggerConsumer::safeUpdate(Lambda lambda) {
    if (model == nullptr) {
        return;
    }
    const auto wasRunning = isRunning();
    if (wasRunning) {
        onStopConsuming();
    }
    lambda();
    if (wasRunning) {
        onStartConsuming();
    }
}

void AutodecloggerConsumer::setThresholds(std::map<int, double> thresholds) {
    safeUpdate([=]() { 
         for (const auto& [key, value] : thresholds) {
            model->thresholds[key] = value;
        }
     });
}

void AutodecloggerConsumer::setTimes(std::map<int, double> times) {
    safeUpdate([=]() {
        for (const auto& [key, value] : times) {
            model->thresholds[key] = value;
        }
        });
}

void AutodecloggerConsumer::setVoltages(std::map<int, double> voltages) {
    safeUpdate([=]() {
        for (const auto& [key, value] : voltages) {
            model->thresholds[key] = value;
        }
        });
}