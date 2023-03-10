#include "plotconsumer.h"

#include <QTime>

PlotConsumer::PlotConsumer(ModelDevice * mDev, DeviceDataProducer * producer) :
    DeviceDataConsumer(mDev, producer) {

    voltageRange.prefix = UnitPfxNone;
    currentRange.prefix = UnitPfxNone;

    /*! Allocate buffer max size once and for all, so we avoid real time memory reallocations */
    buffer.reserve(DDP_DATA_PACKETS_BUFFER_LEN*totalChannelsNum);

    selectedChannels.resize(currentChannelsNum);
    selectedChannels.fill(true);
}

PlotConsumer::~PlotConsumer() {

}

void PlotConsumer::forceAxisUpdate() {
    /*! This function is to be used only during class initialization, not when a consuming process is already running */
    this->updateTimeAxis();
    this->updateRangeAxis();
}

void PlotConsumer::setMaxSamplesPerPlot(int samples) {
    if (!this->isRunning()) {
        this->clearData();
        maxSamples = samples;
        this->allocateData();
    }
}

void PlotConsumer::selectChannels(QVector <bool> channels) {
    if (!this->isRunning()) {
        this->clearData();
        selectedChannels = channels;
        this->allocateData();
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
        consumptionLock.unlock();
    }

    if (hook != nullptr) {
        delete hook;
        hook = nullptr;
    }
}

void PlotConsumer::onSamplingRateChanged(Measurement_t samplingRate) {
    QMutexLocker locker(&timeAxisMtx);
    samplingRate.convertValue(UnitPfxNone);
    pushedSamplingRate = samplingRate.value;
    pushedSamplingRateFlag = true;
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

void PlotConsumer::updateTimeAxis() {
    QMutexLocker locker(&timeAxisMtx);
    if (pushedDurationFlag || pushedSamplingRateFlag) {
        if (pushedDurationFlag) {
            pushedDurationFlag = false;
            sweepDuration = pushedDuration;
        }

        if (pushedSamplingRateFlag) {
            pushedSamplingRateFlag = false;
            sweepSamplingRate = pushedSamplingRate;
        }

        locker.unlock();
        this->computeTimeAxis();
        emit durationUpdated(sweepDuration);
    }
}

void PlotConsumer::computeTimeAxis() {
    dataSize = qRound(sweepSamplingRate*sweepDuration);
    minDataBatchSize = qRound(sweepSamplingRate*PCS_MIN_DATA_BATCH_DURATION_S);

    subSamplingRatio = (dataSize-1)/maxSamples+1;
    dataSize /= subSamplingRatio;

    subSamplingIdx = 0;

    double dt = ((double)subSamplingRatio)/sweepSamplingRate;
    for (int idx = 0; idx < dataSize; idx++) {
        timeValues[idx] = dt*(double)idx;
    }

    this->emitPlotData();

    pushedDuration = sweepDuration;
}

void PlotConsumer::updateRangeAxis() {
    QMutexLocker locker(&rangeAxisMtx);
    if (pushedVoltageRangeFlag) {
        pushedVoltageRangeFlag = false;
        voltageRange.max = 1.0;
        voltageRange.convertValues(pushedVoltageRange.prefix);
        double coeff = voltageRange.max;
        int counter;
        for (int sampleIdx = 0; sampleIdx < dataSize; sampleIdx++) {
            counter = 0;
            for (int channelIdx = 0; channelIdx < voltageChannelsNum; channelIdx++) {
                if (selectedChannels[channelIdx]) {
                    voltageValues[counter++][sampleIdx] *= coeff;
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
        int counter;
        for (int sampleIdx = 0; sampleIdx < dataSize; sampleIdx++) {
            counter = 0;
            for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                currentValues[counter++][sampleIdx] *= coeff;
            }
        }
        currentRange = pushedCurrentRange;

        emit currentRangeUpdated(currentRange);
    }
}

GapFreePlotConsumer::GapFreePlotConsumer(ModelDevice * mDev, DeviceDataProducer * producer) :
    PlotConsumer(mDev, producer) {

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
    int counter;

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

        if (hook->getDataChunk(buffer, subSamplingRatio, minDataBatchSize)) {
            this->updateTimeAxis();
            this->updateRangeAxis();

            bufferIdx = 0;
            bufferLen = buffer.size();

            /*! Copy data in curves */
            while (bufferIdx < bufferLen) {
                counter = 0;
                for (channelIdx = 0; channelIdx < voltageChannelsNum; channelIdx++) {
                    if (selectedChannels[channelIdx]) {
                        voltageValues[counter++][gapFreeTimeIdx] = buffer[bufferIdx];
                    }
                    bufferIdx++;
                }

                counter = 0;
                for (channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                    if (selectedChannels[channelIdx]) {
                        currentValues[counter++][gapFreeTimeIdx] = buffer[bufferIdx];
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

        } else {
            /*! \todo FCON al momento questa cosa non accade mai, getDataChunk ritorna sempre true */
            break;
        }
    }
    emit plotDataUpdated();
    consumptionLock.relock();

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

void GapFreePlotConsumer::allocateData() {
    for (int idx = 0; idx < this->voltageChannelsNum; idx++) {
        if (selectedChannels[idx]) {
            voltageValues.append(new double[maxSamples]());
        }
    }

    for (int idx = 0; idx < this->currentChannelsNum; idx++) {
        if (selectedChannels[idx]) {
            currentValues.append(new double[maxSamples]());
        }
    }

    timeValues = new double[maxSamples];
}

void GapFreePlotConsumer::clearData() {
    int counter = 0;
    for (int idx = 0; idx < voltageChannelsNum; idx++) {
        if (selectedChannels[idx]) {
            delete [] voltageValues[counter++];
        }
    }
    voltageValues.clear();

    counter = 0;
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        if (selectedChannels[idx]) {
            delete [] currentValues[counter++];
        }
    }
    currentValues.clear();

    if (timeValues != nullptr) {
        delete [] timeValues;
        timeValues = nullptr;
    }
}

void GapFreePlotConsumer::emitPlotData() {
    emit setPlotData(timeValues, &voltageValues, &currentValues, dataSize);
}
