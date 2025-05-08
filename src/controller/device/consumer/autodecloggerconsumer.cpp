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
    resetStim();
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

void AutodecloggerConsumer::onVoltageRangeChanged() {
    QMutexLocker locker(&rangeAxisMtx);
    pushedVoltageRange = this->getAppStatus()->getVoltageRanges();
    pushedVoltageRangeFlag = true;
}

void AutodecloggerConsumer::onCurrentRangeChanged() {
    QMutexLocker locker(&rangeAxisMtx);
    pushedCurrentRange = this->getAppStatus()->getCurrentRanges();
    pushedCurrentRangeFlag = true;
}

void AutodecloggerConsumer::onClampingModalityChanged(ClampingModality_t mode) {
    switch (mode) {
    case e384CommLib::VOLTAGE_CLAMP:
        break;
    case e384CommLib::CURRENT_CLAMP:
    case e384CommLib::CURRENT_CLAMP_CURRENT_READ:
    case e384CommLib::VOLTAGE_CLAMP_VOLTAGE_READ:
    case e384CommLib::ZERO_CURRENT_CLAMP:
        onStopConsuming();
        break;
    }
}

AutodecloggerConsumer::AutodecloggerConsumer(ApplicationStatus* appStatus, DeviceDataProducer* producer):
    DeviceDataConsumer(appStatus, producer) {
    minDataBatchSize = 10000;
    //assuming there are an equal amount of current and voltage channels
    for (int i = 0; i < currentChannelsNum; i++) {
        clogInfo[i] = std::nullopt;
        belowThresholdTimer[i] = std::nullopt;
    }
}

AutodecloggerConsumer::~AutodecloggerConsumer() {
    buffer.clear();
    originalVoltages.clear();
    clogInfo.clear();
    belowThresholdTimer.clear();
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
            std::map<int, std::vector<double>> currentValues;
            const auto vr = appStatus->getVoltageRanges();
            while (bufferIdx + voltageChannelsNum < bufferLen) {
                for (channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                    // if there is no previous voltage, read it
                    // could this have problems if I read old voltages?
                    if (!originalVoltages[channelIdx].has_value()) {
                        const Measurement m = { buffer[bufferIdx], vr[channelIdx].prefix, vr[channelIdx].unit };
                        originalVoltages[channelIdx] = m;
                    }
                    const auto currentValue = buffer[bufferIdx + voltageChannelsNum];
                    //adding current current values in the array, these values will be used to calculate the avg
                    currentValues[channelIdx].push_back(currentValue);
                    bufferIdx++;
                }
                bufferIdx += currentChannelsNum;
            }
            //vector of channels that need to start declogging or need to complete it
            std::vector<unsigned short> toStart = {};
            for (auto& [k, v] : belowThresholdTimer) {
                //if clogInfo has an entry, the declogging is being performed, no measurement must be done
                if (clogInfo[k].has_value()) {
                    break;
                }
                const auto nElements = currentValues[k].size();
                if (nElements == 0) {
                    break;
                }
                const auto sum = accumulate(currentValues[k].begin(), currentValues[k].end(), 0.0);
                const auto avg = sum / (double)nElements;
                const auto absAvg = abs(avg);
                if (absAvg < model->thresholds[k]) {
                    //initialize the timer
                    if (!v.has_value()) {
                        const auto timer = new QElapsedTimer();
                        timer->start();
                        v = std::make_optional(timer);
                    }
                    else if (v.value()->elapsed() > model->belowThresholdsTimes[k]) {
                        toStart.push_back(k);
                        v = std::nullopt;
                    } 
                } else {
                    v = std::nullopt;
                }
            }
            // check that at least 300ms have passed before reading the current again
            // once the timer has elapsed, we can safely delete the declog structure and 
            // restart monitoring current
            for (auto& [k, v] : clogInfo) {
                if (v.has_value() && v.value().decloggingComplete && v.value().timer->elapsed() > 300.0) {
                    v = std::nullopt;
                }
            }
            complete();
            startDeclogging(toStart);
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
            model->decloggingTimes[key] = value;
        }
        });
}

void AutodecloggerConsumer::setTimeBelowThreshold(std::map<int, double> times) {
    safeUpdate([=]() {
        for (const auto& [key, value] : times) {
            model->belowThresholdsTimes[key] = value;
        }
        });
}

void AutodecloggerConsumer::setVoltages(std::map<int, double> voltages) {
    safeUpdate([=]() {
        for (const auto& [key, value] : voltages) {
            model->stimuli[key] = value;
        }
        });
}

void AutodecloggerConsumer::complete() {
    bool declogging = false;
    for (auto& [k, v] : clogInfo) {
        if (v.has_value()) {
            declogging = true;
            continue;
        }
    }
    if (!declogging) {
        return;
    }
    //come back to original stimulus
    std::vector<Measurement> resetValues;
    std::vector<unsigned short> chIndexes;
    for (auto& [k, v] : clogInfo) {
        if (v.has_value()) {
            auto &ci = v.value();
            if (!ci.decloggingComplete && ci.timer->elapsed() > model->decloggingTimes[k]) {
                resetValues.push_back(ci.originalVoltageHoldTuner);
                chIndexes.push_back((unsigned int)k);
                ci.decloggingComplete = true;
                ci.timer->restart();
            }
        }
    }
    for (auto& [k, v] : originalVoltages) {
        v = std::nullopt;
    }
    appStatus->getMessageDispatcher()->setVoltageHoldTuner(chIndexes, resetValues, true);
    //signal that this channel is not declogging anymore
    emit sigDecloggingCompleted(chIndexes);
}

void AutodecloggerConsumer::resetStim() {
    bool declogging = false;
    for (auto& [k, v] : clogInfo) {
        if (v.has_value()) {
            declogging = true;
            continue;
        }
    }
    if (!declogging) {
        return;
    }
    //come back to original stimulus without further checks
    std::vector<Measurement> resetValues;
    std::vector<unsigned short> chIndexes;
    for (auto& [k, v] : clogInfo) {
        if (v.has_value()) {
            auto& ci = v.value();
            resetValues.push_back(ci.originalVoltageHoldTuner);
            chIndexes.push_back((unsigned int)k);
            v = std::nullopt;
        }
    }
    appStatus->getMessageDispatcher()->setVoltageHoldTuner(chIndexes, resetValues, true);
    //signal that this channel is not declogging anymore
    emit sigDecloggingCompleted(chIndexes);
}

void AutodecloggerConsumer::startDeclogging(std::vector<unsigned short> toStart) {
    if (toStart.size() == 0) {
        return;
    }
    std::vector<Measurement> tuners, voltagesToApply;
    appStatus->getMessageDispatcher()->getVoltageHoldTuner(tuners);
    for (const auto& ch : toStart) {
        if (!clogInfo[ch].has_value() && originalVoltages[ch].has_value()) {
            const auto tuner = tuners[ch];
            const auto voltage = originalVoltages[ch].value();
            const auto vNoTuner = voltage - tuner;
            const auto userVoltage = model->stimuli[ch];
            const auto timer = new QElapsedTimer();
            const Measurement vToApply = { userVoltage - vNoTuner.value, vNoTuner.prefix, vNoTuner.unit };
            const auto ci = ClogInfo::create(tuner, vToApply, timer);
            clogInfo[ch] = std::make_optional(ci);
        }
    }
    for (auto& [k, v] : clogInfo) {
        if (v.has_value()) {
            ClogInfo ci = v.value();
            voltagesToApply.push_back(ci.vToApply);
            ci.timer->start();
        }
    }
    //set voltages to declog the pore
    appStatus->getMessageDispatcher()->setVoltageHoldTuner(toStart, voltagesToApply, true);
    emit sigDecloggingStarted(toStart);
}
