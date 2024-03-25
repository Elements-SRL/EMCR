#include "ivgraphconsumer.h"
#include <QTime>

IvGraphConsumer::IvGraphConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer):
    DeviceDataConsumer(appStatus, producer) {
//    todo read from file this value?
    this->nBins = currentChannelsNum;
    ivChannels.reserve(currentChannelsNum);
    currentValues.reserve(currentChannelsNum);
    for(auto &&a: currentValues){
        a.reserve(binSize);
    }
}

IvGraphConsumer::~IvGraphConsumer() {
    this->onStopConsuming();
    this->clearData();
}

void IvGraphConsumer::onStartConsuming() {
    hook = producer->getDataHook();
    if (hook != nullptr) {
        this->start();
    }
}

void IvGraphConsumer::onStopConsuming() {
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

void IvGraphConsumer::onVoltageRangeChanged(RangedMeasurement_t range) {
    QMutexLocker locker(&voltageAxisMtx);
    pushedVoltageRange = range;
    pushedVoltageRangeFlag = true;
}

void IvGraphConsumer::onCurrentRangeChanged(RangedMeasurement_t range) {
    QMutexLocker locker(&currentAxisMtx);
    pushedCurrentRange = range;
    pushedCurrentRangeFlag = true;
}


void IvGraphConsumer::onSelectChannels(bool flag) {
    std::vector <uint16_t> selectedChannels = appStatus->getSelectedChannelsIndexes();
    std::vector <bool> values(selectedChannels.size(), flag);
    bool wasThisRunning = this->isRunning();
    if (wasThisRunning) {
        this->onStopConsuming();
    }

    for (auto channelIdx : selectedChannels) {
        plottedChannels[channelIdx] = !plottedChannels[channelIdx];
    }

    forceAxisUpdate();
    if (wasThisRunning) {
        this->onStartConsuming();
    }
}

void IvGraphConsumer::forceAxisUpdate() {
    pushedVoltageRangeFlag = true;
    pushedCurrentRangeFlag = true;
//    this->updateVoltageAxis();
//    this->updateCurrentAxis();
}


void IvGraphConsumer::run() {
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

        if (hook->getDataChunk(buffer, subSamplingRatio, minDataBatchSize)) {
//            this->updateTimeAxis();
//            this->updateRangeAxis();

            bufferIdx = 0;
            bufferLen = buffer.size();

            /*! Copy data in curves */
            while (bufferIdx < bufferLen) {
                for (channelIdx = 0; channelIdx < voltageChannelsNum; channelIdx++) {
                    if (plottedChannels[channelIdx]) {
                        auto voltage = buffer[bufferIdx];
//                      use the voltage value to index the currents
                        auto binIndex = scaleToBins(voltage);
                        bufferIdx++;
                        auto currentValue = buffer[bufferIdx];
                        ivChannels[channelIdx]->pushValue(binIndex, currentValue);
                        bufferIdx++;
                    } else {
                        bufferIdx+=2;
                    }
                }
            }

            currentTimeMs = updateDataTimer.elapsed();
            if (currentTimeMs-lastUpdateTimeMs > PCS_MIN_UPDATE_PLOT_TIME_MS) {
                emitPlotData();
                lastUpdateTimeMs = currentTimeMs;
            }
        }
    }
    emitPlotData();
    consumptionLock.relock();

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

// Function to scale a value into a number of bins
int IvGraphConsumer::scaleToBins(double value) {
    // Calculate the adjusted value to lie within [0, 2*v]
    double adjusted_value = (value + voltageRange.max) / nBins;

    // Calculate the bin index
    int bin_index = static_cast<int>(adjusted_value);

    // Ensure the bin index falls within [0, n_bins-1] range
    if (bin_index < 0) {
        bin_index = 0;
    } else if (bin_index >= nBins) {
        bin_index = nBins - 1;
    }

    return bin_index;
}

void IvGraphConsumer::allocateData() {
    for (int idx = 0; idx < this->currentChannelsNum; idx++) {
//        todo maybe get the n_bins from some type of configuration file
        ivChannels.push_back(new IvChannel(nBins, binSize, voltageRange.min));
    }
//    forceAxisUpdate();
}

//todo call this method when bin size changes or when voltage range changes
void IvGraphConsumer::calculateBinSize(int nBins){
    // Calculate the size of each bin
    binSize = 2 * voltageRange.max / nBins;
}

void IvGraphConsumer::emitPlotData() {
    auto voltageVaues = ivChannels[0]->getVoltages();
    for (int i=0; i<currentChannelsNum; i++){
        currentValues[i] = ivChannels[i]->getCurrents();
    }
    emit setPlotData(voltageVaues, currentValues, 0);
}

void IvGraphConsumer::clearData(){
    buffer.clear();
    ivChannels.clear();
    currentValues.clear();
    plottedChannels.clear();
}
