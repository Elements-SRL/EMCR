#include "plotconsumer.h"

#include <QTime>

PlotConsumer::PlotConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    DeviceDataConsumer(appStatus, producer) {

    voltageRange.prefix = UnitPfxNone;
    currentRange.prefix = UnitPfxNone;

    /*! Allocate buffer max size once and for all, so we avoid real time memory reallocations */
    buffer.reserve(producer->getDataPacketsBufferLen()*totalChannelsNum);

    this->plotAllChannels(true);
}

PlotConsumer::~PlotConsumer() {

}

void PlotConsumer::setProtocolId(unsigned int protocolId) {
    this->protocolId = protocolId;
}

void PlotConsumer::setSweepsNum(unsigned int sweepsNum) {
    this->sweepsNum = sweepsNum;
}

void PlotConsumer::onStartConsuming() {
    hook = producer->getDataHook();
    if (hook != nullptr) {
        QMutexLocker consumptionLock(&consumptionMtx);
        consumptionStopped = false;
        exitedDataConsumingLoop = false;

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

void PlotConsumer::plotAllChannels(bool flag) {
    if (flag) {
        for (int idx = 0; idx < appStatus->getCurrentChannelsNum(); idx++) {
            expandedChannels.push_back(idx);
        }
    }
    else {
        expandedChannels.clear();
    }
}

void PlotConsumer::onPlotSelectedChannels(bool flag) {
    expandedChannels = appStatus->getExpandedChannelsIndexes();
    forceAxisUpdate();
}

void PlotConsumer::updateRangeAxis() {
    bool anyPushed = false;
    QMutexLocker locker(&rangeAxisMtx);

    if (pushedVoltageRangeFlag) {
        anyPushed = true;
        pushedVoltageRangeFlag = false;
        voltageRange.max = 1.0;
        voltageRange.convertValues(pushedVoltageRange.prefix);
        double coeff = voltageRange.max;
        for (auto channelIdx : expandedChannels) {
            for (int sampleIdx = 0; sampleIdx < dataSize; sampleIdx++) {
                voltageValues[channelIdx][sampleIdx] *= coeff;
            }
        }
        voltageRange = pushedVoltageRange;
    }

    if (pushedCurrentRangeFlag) {
        anyPushed = true;
        pushedCurrentRangeFlag = false;
        double coeff;
        currentRange.max = 1.0;
        currentRange.convertValues(pushedCurrentRange.prefix);
        coeff = currentRange.max;
        for (auto channelIdx : expandedChannels) {
            for (int sampleIdx = 0; sampleIdx < dataSize; sampleIdx++) {
                currentValues[channelIdx][sampleIdx] *= coeff;
            }
        }
        currentRange = pushedCurrentRange;
    }

    if (anyPushed) {
        this->emitPlotData();
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

void GapFreePlotConsumer::forceAxisUpdate() {
    pushedDurationFlag = true;
    pushedVoltageRangeFlag = true;
    pushedCurrentRangeFlag = true;
    this->updateTimeAxis();
    this->updateRangeAxis();
}

void GapFreePlotConsumer::setMaxSamplesPerPlot(int samples) {
    bool wasThisRunning = this->isRunning();
    if (wasThisRunning) {
        this->onStopConsuming();
    }

    this->clearData();
    maxSamples = samples;
    this->allocateData();

    if (wasThisRunning) {
        this->onStartConsuming();
    }
}

void GapFreePlotConsumer::run() {
    int bufferIdx;
    int bufferLen = 0;
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
        if (hook == nullptr) {
            msleep(20);
            continue;
        }
        if (hook->getDataChunk(buffer, subSamplingRatio, minDataBatchSize)) {
            this->updateTimeAxis();
            this->updateRangeAxis();

            bufferIdx = 0;
            bufferLen = buffer.size();

            /*! Copy data in curves */
            while (bufferIdx < bufferLen) {
                for (auto channelIdx : expandedChannels) {
                    voltageValues[channelIdx][gapFreeTimeIdx] = buffer[bufferIdx+channelIdx];
                    currentValues[channelIdx][gapFreeTimeIdx] = buffer[bufferIdx+channelIdx+voltageChannelsNum];
                }
                bufferIdx += totalChannelsNum;

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

void GapFreePlotConsumer::updateTimeAxis() {
    QMutexLocker locker(&timeAxisMtx);
    if (pushedDurationFlag) {
        pushedDurationFlag = false;
        xAxisDuration = pushedDuration;

        if (!pushedSamplingRateFlag && !pushedDownsamplingRatioFlag) { // if any of these is true the locker is still needed and the computeTimeAxisMethod is performed later
            locker.unlock();
            this->computeTimeAxis();
        }
    }

    if (pushedSamplingRateFlag || pushedDownsamplingRatioFlag) {
        pushedSamplingRateFlag = false;
        pushedDownsamplingRatioFlag = false;
        samplingRateHz = pushedSamplingRateHz/(double)pushedDownsamplingRatio;

        locker.unlock();
        this->computeTimeAxis();
    }
}

void GapFreePlotConsumer::computeTimeAxis() {
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

EpisodicPlotConsumer::EpisodicPlotConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    PlotConsumer(appStatus, producer) {

    this->allocateData();
    this->updateTimeAxis();
}

EpisodicPlotConsumer::~EpisodicPlotConsumer() {

}

void EpisodicPlotConsumer::forceAxisUpdate() {
    pushedDurationFlag = true;
    pushedVoltageRangeFlag = true;
    pushedCurrentRangeFlag = true;
    this->updateTimeAxis();
    // this->updateRangeAxis();
}

void EpisodicPlotConsumer::setMaxSamplesPerPlot(int samples) {
    bool wasThisRunning = this->isRunning();
    if (wasThisRunning) {
        this->onStopConsuming();
    }

    this->clearData();
    maxSamples = samples;
    this->allocateData();

    if (wasThisRunning) {
        this->onStartConsuming();
    }
}

void EpisodicPlotConsumer::onStartConsuming() {
    episodicHook = producer->getEpisodicDataHook(protocolId, sweepsNum);
    if (episodicHook != nullptr) {
        QMutexLocker consumptionLock(&consumptionMtx);
        consumptionStopped = false;
        exitedDataConsumingLoop = false;

        this->start();
    }
}

void EpisodicPlotConsumer::onStopConsuming() {
    if (this->isRunning()) {
        QMutexLocker consumptionLock(&consumptionMtx);
        consumptionStopped = true;
        while (!exitedDataConsumingLoop) {
            exitedDataConsumingLoopCv.wait(&consumptionMtx, 100);
        }
    }

    if (episodicHook != nullptr) {
        delete episodicHook;
        episodicHook = nullptr;
    }
}

void EpisodicPlotConsumer::run() {
    int bufferIdx;
    int bufferLen = 0;
    int timeIdx = 0;

    episodicMessage.newProtocolFlag = true;
    episodicMessage.durationS = pushedDuration;

    QMutexLocker consumptionLock(&consumptionMtx);
    consumptionLock.unlock();

    this->updateTimeAxis();
    // this->updateRangeAxis();

    while (true) {
        consumptionLock.relock();
        if (consumptionStopped) {
            consumptionLock.unlock();
            break;
        }
        consumptionLock.unlock();
        if (episodicHook == nullptr) {
            msleep(20);
            continue;
        }
        if (episodicHook->getDataChunk(buffer, subSamplingRatio, minDataBatchSize)) {
            // this->updateRangeAxis(); /*! \todo FCON aggiornare il range in episodico ha senso? */
            episodicMessage.newSweepFlag = episodicHook->getSweepNewFlag();
            if (episodicMessage.newSweepFlag) {
                if (episodicHook->getProtocolEndedFlag()) {
                    consumptionStopped = true;
                    continue;
                }
                timeIdx = 0;
            }

            bufferIdx = 0;
            bufferLen = buffer.size();

            episodicMessage.timeValues.clear();

            for (auto & values : episodicMessage.voltageValues) {
                values.clear();
            }

            for (auto & values : episodicMessage.currentValues) {
                values.clear();
            }

            while (bufferIdx < bufferLen) {
                if (timeIdx >= dataSize) {
                    bufferIdx = bufferLen;
                    break;
                }
                episodicMessage.timeValues.push_back(timeValues[timeIdx++]);
                for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
                    episodicMessage.voltageValues[channelIdx].push_back(buffer[bufferIdx+channelIdx]);
                    episodicMessage.currentValues[channelIdx].push_back(buffer[bufferIdx+channelIdx+voltageChannelsNum]);
                }
                bufferIdx += totalChannelsNum;
            }

            emit setPlotData(episodicMessage);

            episodicMessage.newProtocolFlag = false;
        }
    }
    consumptionLock.relock();

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

void EpisodicPlotConsumer::allocateData() {
    episodicMessage.voltageValues.resize(voltageChannelsNum);
    for (int idx = 0; idx < voltageChannelsNum; idx++) {
        episodicMessage.voltageValues[idx].reserve(PCS_MAX_SAMPLES_PER_EPISODIC_PLOT);
    }
    episodicMessage.currentValues.resize(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        episodicMessage.currentValues[idx].reserve(PCS_MAX_SAMPLES_PER_EPISODIC_PLOT);
    }
    timeValues = new double[maxSamples];
    forceAxisUpdate();
}

void EpisodicPlotConsumer::clearData() {
    episodicMessage.voltageValues.clear();
    episodicMessage.currentValues.clear();

    if (timeValues != nullptr) {
        delete [] timeValues;
        timeValues = nullptr;
    }
}

void EpisodicPlotConsumer::emitPlotData() {
    emit setPlotData(episodicMessage);
}

void EpisodicPlotConsumer::updateTimeAxis() {
    QMutexLocker locker(&timeAxisMtx);
    if (pushedDurationFlag) {
        pushedDurationFlag = false;
        xAxisDuration = pushedDuration;

        if (!pushedSamplingRateFlag && !pushedDownsamplingRatioFlag) { // if any of these is true the locker is still needed and the computeTimeAxisMethod is performed later
            locker.unlock();
            this->computeTimeAxis();
        }
    }

    if (pushedSamplingRateFlag || pushedDownsamplingRatioFlag) {
        pushedSamplingRateFlag = false;
        pushedDownsamplingRatioFlag = false;
        samplingRateHz = pushedSamplingRateHz/(double)pushedDownsamplingRatio;

        locker.unlock();
        this->computeTimeAxis();
        /*! \todo FCON può dare che serva fare un repaint del plot, vedere ez patch */
    }
}

void EpisodicPlotConsumer::computeTimeAxis() {
    dataSize = qRound(samplingRateHz*xAxisDuration);
    minDataBatchSize = qMin(qRound(samplingRateHz*PCS_MIN_DATA_BATCH_DURATION_S), (int)producer->getDataPacketsBufferLen()/16);

    subSamplingRatio = (dataSize-1)/maxSamples+1;
    dataSize /= subSamplingRatio;

    subSamplingIdx = 0;

    double dt = ((double)subSamplingRatio)/samplingRateHz;
    for (int idx = 0; idx < dataSize; idx++) {
        timeValues[idx] = dt*(double)idx;
    }

    // this->emitPlotData(); /*! \todo FCON in teoria serve solo il setPlotData con i vettori */
}
