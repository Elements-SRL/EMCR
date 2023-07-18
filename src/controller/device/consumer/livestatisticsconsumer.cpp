#include "livestatisticsconsumer.h"

#include <qmath.h>
#include <QFile>
#include <QDir>
#include <QTextStream>

#include "measurementsoverviewdockwidget.h"

LiveStatisticsConsumer::LiveStatisticsConsumer(MessageDispatcher * msgDisp, MainWindow * mainWindow, DeviceDataProducer * producer) :
    DeviceDataConsumer(msgDisp, producer) {
    getNewActiveChannels(activeChannelsIdxs);
    int currentChannels;
    int voltageChannels;
    msgDisp->getChannelNumberFeatures(voltageChannels, currentChannels);
    modw = new MeasurementsOverviewDockWidget(activeChannelsIdxs, voltageChannels, currentChannels);

    analysisBuffer.reserve(qRound(LSC_MIN_BATCH_INTERVAL_S*1.2*totalChannelsNum));
    connect(this, &LiveStatisticsConsumer::sigResult, modw, &MeasurementsOverviewDockWidget::onResult);

    mainWindow->setMeasurementOverviewDw(modw);
}

LiveStatisticsConsumer::~LiveStatisticsConsumer() {

}

void LiveStatisticsConsumer::onStartConsuming() {
    hook = producer->getDataHook();
    if (hook != nullptr) {
        this->start();
    }
}

void LiveStatisticsConsumer::onStopConsuming() {
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

void LiveStatisticsConsumer::onSamplingRateChanged(Measurement_t samplingRate) {
    QMutexLocker locker(&samplingRateMtx);
    samplingRate.convertValue(UnitPfxNone);
    pushedSamplingRateHz = samplingRate.value;
    pushedSamplingRateFlag = true;
}

void LiveStatisticsConsumer::onDownsamplingRatioChanged(unsigned int ratio) {
    QMutexLocker locker(&samplingRateMtx);
    pushedDownsamplingRatio = ratio;
    pushedDownsamplingRatioFlag = true;
}

void LiveStatisticsConsumer::onVoltageRangeChanged(RangedMeasurement_t range) {
    QMutexLocker locker(&rangesMtx);
    pushedVoltageRange = range;
    pushedVoltageRangeFlag = true;
}

void LiveStatisticsConsumer::onCurrentRangeChanged(RangedMeasurement_t range) {
    QMutexLocker locker(&rangesMtx);
    pushedCurrentRange = range;
    pushedCurrentRangeFlag = true;
}

void LiveStatisticsConsumer::onExportLiveNoiseEstimates() {
    QString filename = "noise";
    QString filedir = QDir::currentPath() + "/";
    QString filepath = filedir + filename + ".csv";
    while (QFile::exists(filepath)) {
        filename += "_";
        filepath = filedir + filename + ".csv";
    }

    QFile file(filepath);
    file.open(QIODevice::WriteOnly);

    QTextStream stream(&file);
    for (auto noise : res->stdCurrent) {
        stream << noise << "\n";
    }
    file.close();
}

void LiveStatisticsConsumer::run() {
    this->initAnalysis();

    QMutexLocker consumptionLock(&consumptionMtx);
    consumptionStopped = false;
    exitedDataConsumingLoop = false;
    consumptionLock.unlock();

    pushedVoltageRangeFlag = true;
    pushedCurrentRangeFlag = true;
    pushedSamplingRateFlag = true;
    pushedDownsamplingRatioFlag = true;

    while (true) {
        consumptionLock.relock();
        if (consumptionStopped) {
            break;
        }
        consumptionLock.unlock();

        this->updateSamplingRate();
        this->updateRanges();

        if (hook->getDataChunk(buffer, 1, minDataBatchSize)) {
            analysisBuffer.append(buffer);

            this->performAnalysis();
            hook->flush(); /*! Get rid of some data, these analyses will work anyway */
        }
    }

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

void LiveStatisticsConsumer::initAnalysis() {
    res = new StatisticsResult(voltageChannelsNum, currentChannelsNum);
    voltageSum.resize(voltageChannelsNum);
    voltageSum2.resize(voltageChannelsNum);
    currentSum.resize(currentChannelsNum);
    currentSum2.resize(currentChannelsNum);
    this->lockAndResetAnalysis(currentChannelsNum);
}

void LiveStatisticsConsumer::lockAndResetAnalysis(int currentChannelIdx) {
    QMutexLocker locker(&mutex);
    this->resetAnalysis(currentChannelIdx);
}

void LiveStatisticsConsumer::resetAnalysis(int) {
    analysisBuffer.clear();

    minSamples = qRound(samplingRateHz*LSC_MIN_INTERVAL_S);
    minDataBatchSize = qRound(samplingRateHz*LSC_MIN_BATCH_INTERVAL_S);
    totalAnalysisSamples = 0;
}

void LiveStatisticsConsumer::performAnalysis() {
    QMutexLocker locker(&mutex);
    bufferSize = analysisBuffer.size();
    analysisSamples = bufferSize/totalChannelsNum;
    int channelIdx;

    if (totalAnalysisSamples == 0) {
        voltageSum.fill(0.0);
        voltageSum2.fill(0.0);
        currentSum.fill(0.0);
        currentSum2.fill(0.0);
    }

    for (analysisIdx = 0; analysisIdx < bufferSize; analysisIdx += totalChannelsNum) {
        for (voltageIdx = 0; voltageIdx < voltageChannelsNum; voltageIdx++) {
            channelIdx = analysisIdx+voltageIdx;
            voltageSum[voltageIdx] += analysisBuffer[channelIdx];
            voltageSum2[voltageIdx] += analysisBuffer[channelIdx]*analysisBuffer[channelIdx];
        }

        for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
            channelIdx = analysisIdx+voltageChannelsNum+currentIdx;
            currentSum[currentIdx] += analysisBuffer[channelIdx];
            currentSum2[currentIdx] += analysisBuffer[channelIdx]*analysisBuffer[channelIdx];
        }
    }
    totalAnalysisSamples += analysisSamples;
    analysisBuffer.clear();

    if (totalAnalysisSamples >= minSamples) {
        for (voltageIdx = 0; voltageIdx < voltageChannelsNum; voltageIdx++) {
            res->meanVoltage[voltageIdx] = voltageSum[voltageIdx]/((double)totalAnalysisSamples);
            res->stdVoltage[voltageIdx] = qSqrt((voltageSum2[voltageIdx]-voltageSum[voltageIdx]*res->meanVoltage[voltageIdx])/((double)totalAnalysisSamples))*voltageMultiplier;
            res->meanVoltage[voltageIdx] *= voltageMultiplier;
        }

        for (currentIdx = 0; currentIdx < currentChannelsNum; currentIdx++) {
            res->meanCurrent[currentIdx] = currentSum[currentIdx]/((double)totalAnalysisSamples);
            res->stdCurrent[currentIdx] = qSqrt((currentSum2[currentIdx]-currentSum[currentIdx]*res->meanCurrent[currentIdx])/((double)totalAnalysisSamples))*currentMultiplier;
            res->meanCurrent[currentIdx] *= currentMultiplier;
            if (res->meanVoltage[currentIdx]*res->meanCurrent[currentIdx] <= 0.0) {
                res->conductivity[currentIdx] = -1.0;

            } else {
                res->conductivity[currentIdx] = res->meanCurrent[currentIdx]/res->meanVoltage[currentIdx];
            }
        }
        totalAnalysisSamples = 0;

        emit sigResult(res);
    }
}

void LiveStatisticsConsumer::updateSamplingRate() {
    QMutexLocker locker(&samplingRateMtx);
    if (pushedSamplingRateFlag || pushedDownsamplingRatioFlag) {
        pushedSamplingRateFlag = false;
        pushedDownsamplingRatioFlag = false;
        samplingRateHz = pushedSamplingRateHz/(double)pushedDownsamplingRatio;
        this->lockAndResetAnalysis(currentChannelsNum);
    }
}

void LiveStatisticsConsumer::updateRanges() {
    QMutexLocker locker(&rangesMtx);
    if (pushedVoltageRangeFlag) {
        pushedVoltageRangeFlag = false;
        voltageRange = pushedVoltageRange;
        voltageMultiplier = voltageRange.multiplier();
        this->lockAndResetAnalysis(currentChannelsNum);
    }

    if (pushedCurrentRangeFlag) {
        pushedCurrentRangeFlag = false;
        currentRange = pushedCurrentRange;
        currentMultiplier = currentRange.multiplier();
        this->lockAndResetAnalysis(currentChannelsNum);
    }
}

bool LiveStatisticsConsumer::isInVec(std::vector<int> vec, int elem){
    auto it = std::find(vec.begin(), vec.end(), elem);
    return it == vec.end();
}

void removeElem(std::vector<int> vec, int elem){
    auto it = std::find(vec.begin(), vec.end(), elem);
    vec.erase(it);
}

void LiveStatisticsConsumer::getNewActiveChannels(std::vector <int>& newActiveChannels){
    newActiveChannels.clear();
    std::vector <bool> selectedChannels;
    msgDisp->getSelectedChannels(selectedChannels);
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        if (selectedChannels[channelIdx]){
            newActiveChannels.push_back(channelIdx);
        }
    }
}

void LiveStatisticsConsumer::onSingleChannelClicked(uint16_t chIdx, bool newState){
    getNewActiveChannels(activeChannelsIdxs);
    modw->updateActiveChannels(activeChannelsIdxs);
}

void LiveStatisticsConsumer::onOneBoardClicked(uint16_t brdIdx, bool newState) {
    getNewActiveChannels(activeChannelsIdxs);
    modw->updateActiveChannels(activeChannelsIdxs);
}

void LiveStatisticsConsumer::onOneRowClicked(uint16_t rowIdx, bool newState) {
    getNewActiveChannels(activeChannelsIdxs);
    modw->updateActiveChannels(activeChannelsIdxs);
}

void LiveStatisticsConsumer::onAllChannelsClicked(bool newState) {
    getNewActiveChannels(activeChannelsIdxs);
    modw->updateActiveChannels(activeChannelsIdxs);
}
