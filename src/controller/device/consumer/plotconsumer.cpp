#include "plotconsumer.h"

#include <QTime>
#include <iostream>

PlotConsumer::PlotConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    DeviceDataConsumer(appStatus, producer) {

    voltageRange.prefix = UnitPfxNone;
    currentRange.prefix = UnitPfxNone;

    /*! Allocate buffer max size once and for all, so we avoid real time memory reallocations */
    buffer = new double[producer->getDataPacketsBufferLen() * totalChannelsNum];

    plottedChannels.resize(currentChannelsNum);
    plottedChannels.fill(true);
}

PlotConsumer::~PlotConsumer() {
    delete[] buffer;
}

void PlotConsumer::forceAxisUpdate() {
    pushedDurationFlag = true;
    pushedVoltageRangeFlag = true;
    pushedCurrentRangeFlag = true;
    this->updateTimeAxis();
    this->updateRangeAxis();
}

void PlotConsumer::setMaxSamplesPerPlot(int samples) {
    bool wasThisRunning = this->isRunning();
    if(wasThisRunning){
        this->onStopConsuming();
    }

    this->clearData();
    maxSamples = samples;
    this->allocateData();

    if(wasThisRunning){
        this->onStartConsuming();
    }
}

void PlotConsumer::onStartConsuming() {
    hook = producer->getDataHook();
    if (hook != nullptr) {
        this->start();
    }
}

void PlotConsumer::onStopConsuming() {
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

void PlotConsumer::onSamplingRateChanged(Measurement_t samplingRate) {
    QMutexLocker locker(&timeAxisMtx);
    samplingRate.convertValue(UnitPfxNone);
    pushedSamplingRateHz = samplingRate.value;
    pushedSamplingRateFlag = true;
}

void PlotConsumer::onDownsamplingRatioChanged(unsigned int ratio) {
    QMutexLocker locker(&timeAxisMtx);
    pushedDownsamplingRatio = ratio;
    pushedDownsamplingRatioFlag = true;
}

void PlotConsumer::onVoltageRangeChanged(RangedMeasurement_t range) {
    QMutexLocker locker(&rangeAxisMtx);
    pushedVoltageRange = range;
    pushedVoltageRangeFlag = true;
}

void PlotConsumer::onCurrentRangeChanged(RangedMeasurement_t range) {
    QMutexLocker locker(&rangeAxisMtx);
    pushedCurrentRange = range;
    pushedCurrentRangeFlag = true;
}

void PlotConsumer::onDurationChanged(Measurement_t duration) {
    QMutexLocker locker(&timeAxisMtx);
    duration.convertValue(UnitPfxNone);
    pushedDuration = duration.value;
    pushedDurationFlag = true;
}

void PlotConsumer::onSelectChannels(bool flag) {
    std::vector <uint16_t> selectedChannels = appStatus->getSelectedChannelsIndexes();
    if (flag) {
        for (auto channelIdx : selectedChannels) {
            if (!plottedChannels[channelIdx]) {
                plottedChannels[channelIdx] = true;
            }
        }

    } else {
        for (auto channelIdx : selectedChannels) {
            if (plottedChannels[channelIdx]) {
                plottedChannels[channelIdx] = false;
            }
        }
    }
    forceAxisUpdate();
}

void PlotConsumer::updateTimeAxis() {
    QMutexLocker locker(&timeAxisMtx);
    if (pushedDurationFlag) {
        pushedDurationFlag = false;
        xAxisDuration = pushedDuration;

        locker.unlock();
        this->computeTimeAxis();
    }

    if (pushedSamplingRateFlag || pushedDownsamplingRatioFlag) {
        pushedSamplingRateFlag = false;
        pushedDownsamplingRatioFlag = false;
        samplingRateHz = pushedSamplingRateHz/(double)pushedDownsamplingRatio;

        locker.unlock();
        this->computeTimeAxis();
    }
}

void PlotConsumer::computeTimeAxis() {
    dataSize = qRound(samplingRateHz*xAxisDuration);
    minDataBatchSize = qMin(qRound(samplingRateHz*PCS_MIN_DATA_BATCH_DURATION_S), (int)producer->getDataPacketsBufferLen()/16);

    subSamplingRatio = (dataSize-1)/maxSamples+1;
    dataSize /= subSamplingRatio;

    subSamplingIdx = 0;

    double dt = ((double)subSamplingRatio)/samplingRateHz;
    for (int idx = 0; idx < dataSize; idx++) {
        timeValues[idx] = dt*(double)idx;
    }

    this->emitPlotData();
}

void PlotConsumer::updateRangeAxis() {
    QMutexLocker locker(&rangeAxisMtx);
    if (pushedVoltageRangeFlag) {
        pushedVoltageRangeFlag = false;
        voltageRange.max = 1.0;
        voltageRange.convertValues(pushedVoltageRange.prefix);
        double coeff = voltageRange.max;
        for (int channelIdx = 0; channelIdx < voltageChannelsNum; channelIdx++) {
            if (plottedChannels[channelIdx]) {
                for (int sampleIdx = 0; sampleIdx < dataSize; sampleIdx++) {
                    voltageValues[channelIdx][sampleIdx] *= coeff;
                }
            }
        }
        voltageRange = pushedVoltageRange;

        emit voltageRangeUpdated(voltageRange);
    }

    if (pushedCurrentRangeFlag) {
        pushedCurrentRangeFlag = false;
        double coeff;
        currentRange.max = 1.0;
        currentRange.convertValues(pushedCurrentRange.prefix);
        coeff = currentRange.max;
        for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
            if (plottedChannels[channelIdx]) {
                for (int sampleIdx = 0; sampleIdx < dataSize; sampleIdx++) {
                    currentValues[channelIdx][sampleIdx] *= coeff;
                }
            }
        }
        currentRange = pushedCurrentRange;

        emit currentRangeUpdated(currentRange);
    }
}

GapFreePlotConsumer::GapFreePlotConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    PlotConsumer(appStatus, producer) {

    this->allocateData();
    this->updateTimeAxis();
}

GapFreePlotConsumer::~GapFreePlotConsumer() {
    this->onStopConsuming();

    this->clearData();
}

void GapFreePlotConsumer::run() {
    consumptionStopped = false;
    exitedDataConsumingLoop = false;

    int bufferIdx;
    int bufferLen = 0;
    int channelIdx;
    QTime updateDataTimer = QTime::currentTime();
    updateDataTimer.start();

    int lastUpdateTimeMs = updateDataTimer.elapsed();
    int currentTimeMs;

    QMutexLocker consumptionLock(&consumptionMtx);
    consumptionLock.unlock();

    while (true) {
        consumptionLock.relock();
        if (consumptionStopped) {
            break;
        }
        consumptionLock.unlock();

        bufferLen = hook->getDataChunk(buffer, subSamplingRatio, minDataBatchSize);
        if (bufferLen > 0) {
            this->updateTimeAxis();
            this->updateRangeAxis();

            bufferIdx = 0;

            /*! Copy data in curves */
            while (bufferIdx < bufferLen) {
                for (channelIdx = 0; channelIdx < voltageChannelsNum; channelIdx++) {
                    if (plottedChannels[channelIdx]) {
                        voltageValues[channelIdx][gapFreeTimeIdx] = buffer[bufferIdx];
                    }
                    bufferIdx++;
                }

                for (channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                    if (plottedChannels[channelIdx]) {
                        currentValues[channelIdx][gapFreeTimeIdx] = buffer[bufferIdx];
                    }
                    bufferIdx++;
                }

                gapFreeTimeIdx++;
                if (gapFreeTimeIdx >= dataSize) {
                    gapFreeTimeIdx = 0;
                }
            }

            currentTimeMs = updateDataTimer.elapsed();
            if (currentTimeMs-lastUpdateTimeMs > PCS_MIN_UPDATE_PLOT_TIME_MS) {
                emit plotDataUpdated();
                lastUpdateTimeMs = currentTimeMs;
            }
        }
    }
    emit plotDataUpdated();
    consumptionLock.relock();

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

void GapFreePlotConsumer::allocateData() {
    for (int idx = 0; idx < this->voltageChannelsNum; idx++) {
        voltageValues.push_back(new double[maxSamples]);
    }
    for (int idx = 0; idx < this->currentChannelsNum; idx++) {
        currentValues.push_back(new double[maxSamples]);
    }
    currentValues.reserve(maxSamples);

    timeValues = new double[maxSamples];
    forceAxisUpdate();
}

void GapFreePlotConsumer::clearData() {
    for (int idx = 0; idx < voltageChannelsNum; idx++) {
        delete [] voltageValues[idx];
    }
    voltageValues.clear();

    for (int idx = 0; idx < currentChannelsNum; idx++) {
        delete [] currentValues[idx];
    }
    currentValues.clear();

    if (timeValues != nullptr) {
        delete [] timeValues;
        timeValues = nullptr;
    }
}

void GapFreePlotConsumer::emitPlotData() {
    GapFreeMessage message = {timeValues, voltageValues, currentValues, dataSize};
    emit setPlotData(message);
}
