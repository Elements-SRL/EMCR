#include "devicedataproducer.h"

#include <qmath.h>
#include <QReadWriteLock>

#define PROTS_BUFFER_SIZE 0x10
#define PROTS_BUFFER_MASK (PROTS_BUFFER_SIZE-1)
#define ITEMS_BUFFER_SIZE 0x10000
#define ITEMS_BUFFER_MASK (ITEMS_BUFFER_SIZE-1)

typedef struct ItemCoords {
    unsigned int protId = 0;
    unsigned int sweepIdx = 0;
    unsigned int itemIdx = 0;
    unsigned int repsIdx = 0;
    unsigned int dataPacketsIdx = 0;
    bool available = false;
} ItemCoords_t;

static bool exitedDataProducingLoop = true;
static QReadWriteLock dataLock;
static QWaitCondition dataCv;
static int16_t ** dataSamplesBuffer;
static double ** floatDataSamplesBuffer;
static double * floatLiquidJunctionBuffer;
static unsigned int dataPacketsIdx = 0;
static ItemCoords_t items[PROTS_BUFFER_SIZE][ITEMS_BUFFER_SIZE];

DeviceDataProducer::DeviceDataProducer(ApplicationStatus * appStatus, QObject * parent) :
    QThread(parent),
    appStatus(appStatus) {

    bitRateTmr = new QTimer();
    bitRateTmr->setSingleShot(false);
    bitRateTmr->setInterval(bitRateComputationIntervalS*1000); /*!< Input in ms */
    connect(bitRateTmr, &QTimer::timeout, this, &DeviceDataProducer::onComputeBitRate);
    bitRateTmr->start();
    samplesReceived = 0;
    voltageChannelsNum = appStatus->getVoltageChannelsNum();
    currentChannelsNum = appStatus->getCurrentChannelsNum();
    totalChannelsNum = voltageChannelsNum+currentChannelsNum;

    dataPacketsBufferLen = 1U << (unsigned int)qFloor(log2((double)DDP_MAX_SAMPLES_FOR_BUFFER/(double)totalChannelsNum));
    dataPacketsBufferMask = dataPacketsBufferLen-1U;

    currentProtIdx = (-1 & PROTS_BUFFER_MASK);

    dataSamplesBuffer = new int16_t * [dataPacketsBufferLen];
    floatDataSamplesBuffer = new double * [dataPacketsBufferLen];
    dataSamplesBuffer[0] = new int16_t[dataPacketsBufferLen*totalChannelsNum];
    floatDataSamplesBuffer[0] = new double[dataPacketsBufferLen*totalChannelsNum];
    for (unsigned int packetIdx = 1; packetIdx < dataPacketsBufferLen; packetIdx++) {
        dataSamplesBuffer[packetIdx] = dataSamplesBuffer[packetIdx-1]+totalChannelsNum;
        floatDataSamplesBuffer[packetIdx] = floatDataSamplesBuffer[packetIdx-1]+totalChannelsNum;
    }
    floatLiquidJunctionBuffer = new double[currentChannelsNum];

    exitedDataProducingLoop = true;
    dataPacketsIdx = 0;
}

DeviceDataProducer::~DeviceDataProducer() {
    bitRateTmr->stop();
    this->onStopProducing();
    this->wait();
    delete [] dataSamplesBuffer[0];
    delete [] floatDataSamplesBuffer[0];
    delete [] dataSamplesBuffer;
    delete [] floatDataSamplesBuffer;
    delete [] floatLiquidJunctionBuffer;
}

unsigned int DeviceDataProducer::getDataPacketsBufferLen() {
    return dataPacketsBufferLen;
}

DataHook * DeviceDataProducer::getDataHook() {
    DataHook * hook;

    hook = new DataHook(totalChannelsNum);
    hook->setBufferSize(dataPacketsBufferLen, dataPacketsBufferMask);

    return hook;
}

EpisodicDataHook * DeviceDataProducer::getEpisodicDataHook(unsigned int protocolId) {
    EpisodicDataHook * hook;

    hook = new EpisodicDataHook(totalChannelsNum, protocolId);
    hook->setBufferSize(dataPacketsBufferLen, dataPacketsBufferMask);

    return hook;
}

void DeviceDataProducer::onStopProducing() {
    if (!exitedDataProducingLoop) {
        QMutexLocker connectionLock(&connectionMtx);
        deviceConnected = false;
        while (!exitedDataProducingLoop) {
            exitedDataProducingLoopCv.wait(&connectionMtx);
        }
    }
    const auto msgDisp = appStatus->getMessageDispatcher();
    msgDisp->deallocateRxDataBuffer(datain);
}

void DeviceDataProducer::run() {
    RxOutput_t dataHeader;

    QMutexLocker connectionLock(&connectionMtx);
    deviceConnected = true;
    exitedDataProducingLoop = false;

    ErrorCodes_t ret;
    const auto msgDisp = appStatus->getMessageDispatcher();
    ret = msgDisp->allocateRxDataBuffer(datain);
    /*! \todo what to do if the memory is not initialized? */

    unsigned int chIdx;
    unsigned int dataSampleBufferIdx;

    connectionLock.unlock();

    QMutexLocker bitRateLock(&bitRateMtx);
    bitRateLock.unlock();

    while (true) {
        connectionLock.relock();
        if (!deviceConnected) {
            break;
        }
        connectionLock.unlock();

        ret = msgDisp->getNextMessage(dataHeader, datain);

        if (ret == Success) {
            switch (dataHeader.msgTypeId) {
            case MsgDirectionDeviceToPc+MsgTypeIdAcquisitionHeader:
                if (currentProtIdx != (dataHeader.protocolId & PROTS_BUFFER_MASK)) {
                    nextItemIdx = 0;
                }
                currentProtIdx = dataHeader.protocolId & PROTS_BUFFER_MASK;
                dataLock.lockForWrite();
                 /*! \todo FCON da capire se il dataPacketsIdx è corretto o ci va messo un -1 o qualcosa del genere */
                items[currentProtIdx][nextItemIdx] = {dataHeader.protocolId, dataHeader.protocolSweepIdx, dataHeader.protocolItemIdx, dataHeader.protocolRepsIdx, dataPacketsIdx, true};
                nextItemIdx = (nextItemIdx+1) & ITEMS_BUFFER_MASK;
                items[currentProtIdx][nextItemIdx].available = false;
                dataLock.unlock();
                break;

            case MsgDirectionDeviceToPc+MsgTypeIdAcquisitionData:
                dataSampleBufferIdx = dataPacketsIdx;
                for (unsigned long wordsIdx = 0; wordsIdx < dataHeader.dataLen; wordsIdx += totalChannelsNum) {
                    for (chIdx = 0; chIdx < voltageChannelsNum; chIdx++) {
                        dataSamplesBuffer[dataSampleBufferIdx][chIdx] = datain[wordsIdx+chIdx];
                    }
                    msgDisp->convertVoltageValues(datain+wordsIdx, floatDataSamplesBuffer[dataSampleBufferIdx], voltageChannelsNum);

                    for (; chIdx < totalChannelsNum; chIdx++) {
                        dataSamplesBuffer[dataSampleBufferIdx][chIdx] = datain[wordsIdx+chIdx];
                    }
                    msgDisp->convertCurrentValues(datain+wordsIdx+voltageChannelsNum, floatDataSamplesBuffer[dataSampleBufferIdx]+voltageChannelsNum, currentChannelsNum);

                    dataSampleBufferIdx = (dataSampleBufferIdx+1) & dataPacketsBufferMask;
                }

                dataLock.lockForWrite();
                dataPacketsIdx = dataSampleBufferIdx;
                dataCv.wakeAll();
                dataLock.unlock();

                bitRateLock.relock();
                samplesReceived += dataHeader.dataLen;
                bitRateLock.unlock();
                break;
            }
        }
        else {
            QThread::msleep(1);
        }
    }

    dataLock.lockForWrite();
    dataPacketsIdx = (dataPacketsIdx+(dataPacketsBufferLen >> 4)) & dataPacketsBufferMask;
    dataCv.wakeAll();
    dataLock.unlock();

    exitedDataProducingLoop = true;
    exitedDataProducingLoopCv.wakeAll();
}

void DeviceDataProducer::onComputeBitRate() {
    QMutexLocker bitRateLock(&bitRateMtx);
    emit bitRateComputed((double)(samplesReceived/bitRateComputationIntervalS/totalChannelsNum));
    samplesReceived = 0;
}

DataHook::DataHook(unsigned int totalChannelsNum) :
    totalChannelsNum(totalChannelsNum) {

    this->flush();
}

DataHook::~DataHook() {

}

void DataHook::setBufferSize(unsigned int bufferSize, unsigned int bufferMask) {
    this->bufferSize = bufferSize;
    this->bufferMask = bufferMask;
    halfBufferSize = bufferSize/2;
}

bool DataHook::getDataChunk(std::vector <unsigned short> &buffer, unsigned int, unsigned int minDataBatchSize) {
    unsigned int dataPacketsMax;
    if (!this->waitDataAvailable(minDataBatchSize, dataPacketsMax)) {
        return false;
    }

    unsigned int dataPacketsToBuffer;
    if (dataIdx <= dataPacketsMax) {
        dataPacketsToBuffer = dataPacketsMax-dataIdx;

    } else {
        dataPacketsToBuffer = dataPacketsMax+bufferSize-dataIdx;
    }

    buffer.resize(dataPacketsToBuffer*totalChannelsNum);
    unsigned int count = 0;
    unsigned int chIdx;
    while (dataIdx != dataPacketsMax) {
        for (chIdx = 0; chIdx < totalChannelsNum; chIdx++) {
            buffer[count++] = dataSamplesBuffer[dataIdx][chIdx];
        }
        dataIdx = (dataIdx+1) & bufferMask;
    }
    return true;
}

bool DataHook::getDataChunk(std::vector <double> &buffer, unsigned int downsamplingRatio, unsigned int minDataBatchSize) {
    unsigned int dataPacketsMax;
    if (!this->waitDataAvailable(minDataBatchSize, dataPacketsMax)) {
        return false;
    }

    unsigned int dataPacketsToBuffer;
    unsigned int downsamplingSize = downsamplingRatio << 1; /*! to reduce size by x we take data in chunks of 2*x and then take max and min in the interval */
    if (downsamplingRatio > 1) {
        if (dataIdx <= dataPacketsMax) {
            dataPacketsToBuffer = ((dataPacketsMax-dataIdx)/downsamplingSize) << 1;

        } else {
            dataPacketsToBuffer = ((dataPacketsMax+bufferSize-dataIdx)/downsamplingSize) << 1;
        }

    } else {
        if (dataIdx <= dataPacketsMax) {
            dataPacketsToBuffer = dataPacketsMax-dataIdx;

        } else {
            dataPacketsToBuffer = dataPacketsMax+bufferSize-dataIdx;
        }
    }
    unsigned int dataSamplesToBuffer = dataPacketsToBuffer*totalChannelsNum;
    buffer.resize(dataSamplesToBuffer);
    unsigned int count = 0;

    unsigned int chIdx;
    double value;
    unsigned int countPlusChIdx;
    if (downsamplingRatio > 1) {
        while (count+totalChannelsNum < dataSamplesToBuffer) {
            for (chIdx = 0; chIdx < totalChannelsNum; chIdx++) {
                value = floatDataSamplesBuffer[dataIdx][chIdx];
                countPlusChIdx = count+chIdx;
                buffer[countPlusChIdx] = value; /*! Initialize max */
                buffer[countPlusChIdx+totalChannelsNum] = value; /*! Initialize min */
            }
            dataIdx = (dataIdx+1) & bufferMask;

            for (unsigned int downsamplingIdx = 1; downsamplingIdx < downsamplingSize; downsamplingIdx++) {
                for (chIdx = 0; chIdx < totalChannelsNum; chIdx++) {
                    value = floatDataSamplesBuffer[dataIdx][chIdx];
                    countPlusChIdx = count+chIdx;
                    if (value > buffer[countPlusChIdx]) {
                        buffer[countPlusChIdx] = value;

                    } else if (value < buffer[countPlusChIdx+totalChannelsNum]) {
                        buffer[countPlusChIdx+totalChannelsNum] = value;
                    }
                }
                dataIdx = (dataIdx+1) & bufferMask;
            }
            count += totalChannelsNum << 1;
        }

    } else {
        while (count < dataSamplesToBuffer) {
            for (chIdx = 0; chIdx < totalChannelsNum; chIdx++) {
                buffer[count++] = floatDataSamplesBuffer[dataIdx][chIdx];
            }
            dataIdx = (dataIdx+1) & bufferMask;
        }
    }
    return true;
}

bool DataHook::getDataChunks(std::vector <double>& doubleBuffer, std::vector <short>& intBuffer, unsigned int minDataBatchSize) {
    unsigned int dataPacketsMax;
    if (!this->waitDataAvailable(minDataBatchSize, dataPacketsMax)) {
        return false;
    }

    unsigned int dataPacketsToBuffer;
    if (dataIdx <= dataPacketsMax) {
        dataPacketsToBuffer = dataPacketsMax - dataIdx;

    }
    else {
        dataPacketsToBuffer = dataPacketsMax + bufferSize - dataIdx;
    }

    doubleBuffer.resize(dataPacketsToBuffer * totalChannelsNum);
    intBuffer.resize(dataPacketsToBuffer * totalChannelsNum);
    int count = 0;
    int chIdx;
    while (dataIdx != dataPacketsMax) {
        for (chIdx = 0; chIdx < totalChannelsNum; chIdx++) {
            doubleBuffer[count] = floatDataSamplesBuffer[dataIdx][chIdx];
            intBuffer[count++] = dataSamplesBuffer[dataIdx][chIdx];
        }
        dataIdx = (dataIdx + 1) & bufferMask;
    }
    return true;
}

void DataHook::flush() {
    dataLock.lockForRead();
    dataIdx = dataPacketsIdx;
    dataLock.unlock();
}

bool DataHook::waitDataAvailable(unsigned int minDataBatchSize, unsigned int &dataPacketsMax) {
    int waitCount = 0;
    dataLock.lockForRead();
    while ((((dataIdx + minDataBatchSize - dataPacketsIdx) & bufferMask) <= halfBufferSize) &&
           (!exitedDataProducingLoop) &&
           waitCount++ < DDP_MAX_WAIT_COUNT) {
        dataCv.wait(&dataLock, 100);
    }

    /*! No data if after several tries no data is obtained. However, if at least one sample is obtained process it, don't require
     *  necessarily minDataBatchSize samples at this point */
    if (waitCount >= DDP_MAX_WAIT_COUNT && dataIdx == dataPacketsIdx) {
        dataLock.unlock();
        return false;
    }

    dataPacketsMax = dataPacketsIdx;
    dataLock.unlock();

    return true;
}

EpisodicDataHook::EpisodicDataHook(unsigned int totalChannelsNum, unsigned int protocolId) :
    totalChannelsNum(totalChannelsNum),
    protocolId(protocolId) {

    this->flush();
}

EpisodicDataHook::~EpisodicDataHook() {

}

void EpisodicDataHook::setBufferSize(unsigned int bufferSize, unsigned int bufferMask) {
    this->bufferSize = bufferSize;
    this->bufferMask = bufferMask;
    halfBufferSize = bufferSize/2;
}

bool EpisodicDataHook::getDataChunk(std::vector <unsigned short> &buffer, bool &newSweep, unsigned int, unsigned int minDataBatchSize) {
    unsigned int dataPacketsMax;
    if (!this->waitDataAvailable(minDataBatchSize, dataPacketsMax)) {
        return false;
    }

    unsigned int dataPacketsToBuffer;
    if (dataIdx <= dataPacketsMax) {
        dataPacketsToBuffer = dataPacketsMax-dataIdx;

    } else {
        dataPacketsToBuffer = dataPacketsMax+bufferSize-dataIdx;
    }

    buffer.resize(dataPacketsToBuffer*totalChannelsNum);
    unsigned int count = 0;
    unsigned int chIdx;
    while (dataIdx != dataPacketsMax) {
        for (chIdx = 0; chIdx < totalChannelsNum; chIdx++) {
            buffer[count++] = dataSamplesBuffer[dataIdx][chIdx];
        }
        dataIdx = (dataIdx+1) & bufferMask;
    }
    newSweep = pushedNewSweepFlag;
    pushedNewSweepFlag = newSweepFlag;
    if (newSweepFlag) {
        newSweepFlag = false;
    }
    return true;
}

bool EpisodicDataHook::getDataChunk(std::vector <double> &buffer, bool &newSweep, unsigned int downsamplingRatio, unsigned int minDataBatchSize) {
    unsigned int dataPacketsMax;
    if (!this->waitDataAvailable(minDataBatchSize, dataPacketsMax)) {
        return false;
    }

    unsigned int dataPacketsToBuffer;
    unsigned int downsamplingSize = downsamplingRatio << 1; /*! to reduce size by x we take data in chunks of 2*x and then take max and min in the interval */
    if (downsamplingRatio > 1) {
        if (dataIdx <= dataPacketsMax) {
            dataPacketsToBuffer = ((dataPacketsMax-dataIdx)/downsamplingSize) << 1;

        } else {
            dataPacketsToBuffer = ((dataPacketsMax+bufferSize-dataIdx)/downsamplingSize) << 1;
        }

    } else {
        if (dataIdx <= dataPacketsMax) {
            dataPacketsToBuffer = dataPacketsMax-dataIdx;

        } else {
            dataPacketsToBuffer = dataPacketsMax+bufferSize-dataIdx;
        }
    }
    unsigned int dataSamplesToBuffer = dataPacketsToBuffer*totalChannelsNum;
    buffer.resize(dataSamplesToBuffer);
    unsigned int count = 0;

    unsigned int chIdx;
    double value;
    unsigned int countPlusChIdx;
    if (downsamplingRatio > 1) {
        while (count+totalChannelsNum < dataSamplesToBuffer) {
            for (chIdx = 0; chIdx < totalChannelsNum; chIdx++) {
                value = floatDataSamplesBuffer[dataIdx][chIdx];
                countPlusChIdx = count+chIdx;
                buffer[countPlusChIdx] = value; /*! Initialize max */
                buffer[countPlusChIdx+totalChannelsNum] = value; /*! Initialize min */
            }
            dataIdx = (dataIdx+1) & bufferMask;

            for (unsigned int downsamplingIdx = 1; downsamplingIdx < downsamplingSize; downsamplingIdx++) {
                for (chIdx = 0; chIdx < totalChannelsNum; chIdx++) {
                    value = floatDataSamplesBuffer[dataIdx][chIdx];
                    countPlusChIdx = count+chIdx;
                    if (value > buffer[countPlusChIdx]) {
                        buffer[countPlusChIdx] = value;

                    } else if (value < buffer[countPlusChIdx+totalChannelsNum]) {
                        buffer[countPlusChIdx+totalChannelsNum] = value;
                    }
                }
                dataIdx = (dataIdx+1) & bufferMask;
            }
            count += totalChannelsNum << 1;
        }

    } else {
        while (count < dataSamplesToBuffer) {
            for (chIdx = 0; chIdx < totalChannelsNum; chIdx++) {
                buffer[count++] = floatDataSamplesBuffer[dataIdx][chIdx];
            }
            dataIdx = (dataIdx+1) & bufferMask;
        }
    }
    newSweep = pushedNewSweepFlag;
    pushedNewSweepFlag = newSweepFlag;
    if (newSweepFlag) {
        newSweepFlag = false;
    }
    return true;
}

bool EpisodicDataHook::getDataChunks(std::vector <double>& doubleBuffer, bool &newSweep, std::vector <short>& intBuffer, unsigned int minDataBatchSize) {
    unsigned int dataPacketsMax;
    if (!this->waitDataAvailable(minDataBatchSize, dataPacketsMax)) {
        return false;
    }

    unsigned int dataPacketsToBuffer;
    if (dataIdx <= dataPacketsMax) {
        dataPacketsToBuffer = dataPacketsMax - dataIdx;

    }
    else {
        dataPacketsToBuffer = dataPacketsMax + bufferSize - dataIdx;
    }

    doubleBuffer.resize(dataPacketsToBuffer * totalChannelsNum);
    intBuffer.resize(dataPacketsToBuffer * totalChannelsNum);
    int count = 0;
    int chIdx;
    while (dataIdx != dataPacketsMax) {
        for (chIdx = 0; chIdx < totalChannelsNum; chIdx++) {
            doubleBuffer[count] = floatDataSamplesBuffer[dataIdx][chIdx];
            intBuffer[count++] = dataSamplesBuffer[dataIdx][chIdx];
        }
        dataIdx = (dataIdx + 1) & bufferMask;
    }
    newSweep = pushedNewSweepFlag;
    pushedNewSweepFlag = newSweepFlag;
    if (newSweepFlag) {
        newSweepFlag = false;
    }
    return true;
}

void EpisodicDataHook::flush() {
    dataLock.lockForRead();
    dataIdx = dataPacketsIdx;
    dataLock.unlock();
}

bool EpisodicDataHook::waitDataAvailable(unsigned int minDataBatchSize, unsigned int &dataPacketsMax) {
    int waitCount = 0;
    dataLock.lockForRead();
    while ((((dataIdx + minDataBatchSize - dataPacketsIdx) & bufferMask) <= halfBufferSize) &&
           (!exitedDataProducingLoop) &&
           waitCount++ < DDP_MAX_WAIT_COUNT) {
        dataCv.wait(&dataLock, 100);
    }

    /*! No data if after several tries no data is obtained. However, if at least one sample is obtained process it, don't require
     *  necessarily minDataBatchSize samples at this point */
    if (waitCount >= DDP_MAX_WAIT_COUNT && dataIdx == dataPacketsIdx) {
        dataLock.unlock();
        return false;
    }

    auto item = items[protocolId][nextItemIdx];

    if (item.available) {
        if (currentSweepIdx != item.sweepIdx) {
            newSweepFlag = true;
            currentSweepIdx = item.sweepIdx;
            dataPacketsMax = item.dataPacketsIdx;
        }
        else {
            dataPacketsMax = dataPacketsIdx;
        }
        nextItemIdx = (nextItemIdx+1) & ITEMS_BUFFER_MASK;
    }
    else {
        dataPacketsMax = dataPacketsIdx;
    }
    dataLock.unlock();

    return true;
}
