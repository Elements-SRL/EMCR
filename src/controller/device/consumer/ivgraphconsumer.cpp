#include "ivgraphconsumer.h"
#include <QTime>
#include <iostream>

IvGraphConsumer::IvGraphConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer):
    PlotConsumer(appStatus, producer) {
//    todo read from file this value?
    this->nBins = 1000;
    this->emitPlotData();
}

IvGraphConsumer::~IvGraphConsumer() {
    this->onStopConsuming();
    this->clearData();
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
                for (channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
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
                for (int i=0; i<currentChannelsNum; i++) {
                    auto currents = ivChannels[i]->getCurrents();
                    for (int j=0; j< nBins; j++){
                        currentValues[i][j] = currents[j];
                    }
                }
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
    ivChannels.reserve(currentChannelsNum);
    voltageData.resize(nBins);
    for (int i = 0; i<nBins; i++){
        voltageData[i] = ((double) i) * binSize;
    }
    for (int idx = 0; idx < this->currentChannelsNum; idx++) {
        currentValues.push_back(new double[nBins]);
//        todo maybe get the n_bins from some type of configuration file
        ivChannels.push_back(new IvChannel(nBins, binSize, voltageRange.min));
    }
}

//todo call this method when bin size changes or when voltage range changes
void IvGraphConsumer::calculateBinSize(int nBins){
    // Calculate the size of each bin
    binSize = 2 * voltageRange.max / nBins;
}

void IvGraphConsumer::emitPlotData() {
    IvMessage message = {voltageData, currentValues, nBins};
    emit setPlotData(message);
}

void IvGraphConsumer::clearData(){
    ivChannels.clear();
    currentValues.clear();
    voltageData.clear();
}
