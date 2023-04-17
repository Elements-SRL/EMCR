#include "devicedataproducer.h"

static bool exitedDataProducingLoop = true;
static QMutex dataMtx;
static QWaitCondition dataCv;
static int16_t ** dataSamplesBuffer;
static double ** floatDataSamplesBuffer;
static unsigned int dataPacketsIdx = 0;

DeviceDataProducer::DeviceDataProducer(ModelDevice * mDev, QObject * parent) :
    QThread(parent),
    mDev(mDev) {

    bitRateTmr = new QTimer();
    bitRateTmr->setSingleShot(false);
    bitRateTmr->setInterval(bitRateComputationIntervalS*1000); /*!< Input in ms */
    connect(bitRateTmr, &QTimer::timeout, this, &DeviceDataProducer::onComputeBitRate);
    bitRateTmr->start();
    samplesReceived = 0;

    mDev->getChannelsNumberFeatures(voltageChannelsNum, currentChannelsNum);
    totalChannelsNum = voltageChannelsNum+currentChannelsNum;

    dataSamplesBuffer = new int16_t * [DDP_DATA_PACKETS_BUFFER_LEN];
    floatDataSamplesBuffer = new double * [DDP_DATA_PACKETS_BUFFER_LEN];
    dataSamplesBuffer[0] = new int16_t[DDP_DATA_PACKETS_BUFFER_LEN*totalChannelsNum];
    floatDataSamplesBuffer[0] = new double[DDP_DATA_PACKETS_BUFFER_LEN*totalChannelsNum];
    for (unsigned int packetIdx = 1; packetIdx < DDP_DATA_PACKETS_BUFFER_LEN; packetIdx++) {
        dataSamplesBuffer[packetIdx] = dataSamplesBuffer[packetIdx-1]+totalChannelsNum;
        floatDataSamplesBuffer[packetIdx] = floatDataSamplesBuffer[packetIdx-1]+totalChannelsNum;
    }
}

DeviceDataProducer::~DeviceDataProducer() {
    bitRateTmr->stop();
    this->onStopProducing();
    this->wait();
    delete [] dataSamplesBuffer[0];
    delete [] floatDataSamplesBuffer[0];
    delete [] dataSamplesBuffer;
    delete [] floatDataSamplesBuffer;
}

DataHook * DeviceDataProducer::getDataHook() {
    DataHook * hook;

    QMutexLocker locker(&dataMtx);
    hook = new DataHook(totalChannelsNum);
    hook->setInitialOffset(dataPacketsIdx);

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
    mDev->getMessageDispatcher()->deallocateRxDataBuffer(datain);
}

void DeviceDataProducer::run() {
    MessageDispatcher * messageDispather = mDev->getMessageDispatcher();
    RxOutput dataHeader;

    QMutexLocker connectionLock(&connectionMtx);
    deviceConnected = true;
    exitedDataProducingLoop = false;

    ErrorCodes_t ret;
    ret = messageDispather->allocateRxDataBuffer(datain);
    /*! \todo what to do if the memory is not initialized? */

    int chIdx;

    connectionLock.unlock();

    QMutexLocker dataLock(&dataMtx);
    dataLock.unlock();

    QMutexLocker bitRateLock(&bitRateMtx);
    bitRateLock.unlock();

    while (true) {
        connectionLock.relock();
        if (!deviceConnected) {
            break;
        }
        connectionLock.unlock();

        dataLock.relock();

        ret = messageDispather->getNextMessage(dataHeader, datain);

        if (ret == Success) {
            for (unsigned long wordsIdx = 0; wordsIdx < dataHeader.dataLen; wordsIdx += totalChannelsNum) {
                for (chIdx = 0; chIdx < voltageChannelsNum; chIdx++) {
                    dataSamplesBuffer[dataPacketsIdx][chIdx] = datain[wordsIdx+chIdx];
                    messageDispather->convertVoltageValue(datain[wordsIdx+chIdx], floatDataSamplesBuffer[dataPacketsIdx][chIdx]);
                }

                for (; chIdx < totalChannelsNum; chIdx++) {
                    dataSamplesBuffer[dataPacketsIdx][chIdx] = datain[wordsIdx+chIdx];
                    messageDispather->convertCurrentValue(datain[wordsIdx+chIdx], floatDataSamplesBuffer[dataPacketsIdx][chIdx]);
                }

                dataPacketsIdx = (dataPacketsIdx+1) & DDP_DATA_PACKETS_BUFFER_MASK;
            }

            dataCv.wakeAll();
            dataLock.unlock();

            bitRateLock.relock();
            samplesReceived += dataHeader.dataLen;
            bitRateLock.unlock();

        } else {
            dataLock.unlock();
            QThread::msleep(1);
        }
    }

    dataLock.relock();
    dataPacketsIdx = (dataPacketsIdx+(DDP_DATA_PACKETS_BUFFER_LEN >> 4))&DDP_DATA_PACKETS_BUFFER_MASK;
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

}

DataHook::~DataHook() {

}

void DataHook::setInitialOffset(unsigned int offset) {
    if (!initialized) {
        dataIdx = offset;
        initialized = true;
    }
}

bool DataHook::getDataChunk(QVector <unsigned short> &buffer, unsigned int, unsigned int minDataBatchSize) {
    int waitCount = 0;
    QMutexLocker locker(&dataMtx);
    while ((((dataIdx+minDataBatchSize-dataPacketsIdx)&DDP_DATA_PACKETS_BUFFER_MASK) <= (DDP_DATA_PACKETS_BUFFER_LEN >> 1)) &&
           (!exitedDataProducingLoop) &&
           waitCount++ < DDP_MAX_WAIT_COUNT) {
        dataCv.wait(&dataMtx, 100);
    }

    if (waitCount >= DDP_MAX_WAIT_COUNT) {
        return false;
    }

    unsigned int dataPacketsMax = dataPacketsIdx;
    locker.unlock();

    unsigned int dataPacketsToBuffer;
    if (dataIdx <= dataPacketsMax) {
        dataPacketsToBuffer = dataPacketsMax-dataIdx;

    } else {
        dataPacketsToBuffer = dataPacketsMax+DDP_DATA_PACKETS_BUFFER_LEN-dataIdx;
    }

    buffer.resize(dataPacketsToBuffer*totalChannelsNum);
    int count = 0;
    unsigned int chIdx;
    while (dataIdx != dataPacketsMax) {
        for (chIdx = 0; chIdx < totalChannelsNum; chIdx++) {
            buffer[count++] = dataSamplesBuffer[dataIdx][chIdx];
        }
        dataIdx = (dataIdx+1)&DDP_DATA_PACKETS_BUFFER_MASK;
    }
    return true;
}

bool DataHook::getDataChunk(QVector <double> &buffer, unsigned int downsamplingRatio, unsigned int minDataBatchSize) {
    int waitCount = 0;
    QMutexLocker locker(&dataMtx);
    while ((((dataIdx+minDataBatchSize-dataPacketsIdx)&DDP_DATA_PACKETS_BUFFER_MASK) <= (DDP_DATA_PACKETS_BUFFER_LEN >> 1)) &&
           (!exitedDataProducingLoop) &&
           waitCount++ < DDP_MAX_WAIT_COUNT) {
        dataCv.wait(&dataMtx, 100);
    }

    if (waitCount >= DDP_MAX_WAIT_COUNT) {
        return false;
    }

    unsigned int dataPacketsMax = dataPacketsIdx;
    locker.unlock();

    int dataPacketsToBuffer;
    unsigned int downsamplingSize = downsamplingRatio << 1; /*! to reduce size by x we take data in chunks of 2*x and then take max and min in the interval */
    if (downsamplingRatio > 1) {
        if (dataIdx <= dataPacketsMax) {
            dataPacketsToBuffer = (int)(((dataPacketsMax-dataIdx)/downsamplingSize) << 1);

        } else {
            dataPacketsToBuffer = (int)(((dataPacketsMax+DDP_DATA_PACKETS_BUFFER_LEN-dataIdx)/downsamplingSize) << 1);
        }

    } else {
        if (dataIdx <= dataPacketsMax) {
            dataPacketsToBuffer = (int)(dataPacketsMax-dataIdx);

        } else {
            dataPacketsToBuffer = (int)(dataPacketsMax+DDP_DATA_PACKETS_BUFFER_LEN-dataIdx);
        }
    }

    int dataSamplesToBuffer = dataPacketsToBuffer*(int)totalChannelsNum;
    buffer.resize(dataSamplesToBuffer);
    int count = 0;

    unsigned int chIdx;
    double value;
    if (downsamplingRatio > 1) {
        while (count < dataSamplesToBuffer-(int)totalChannelsNum) {
            for (chIdx = 0; chIdx < totalChannelsNum; chIdx++) {
                value = floatDataSamplesBuffer[dataIdx][chIdx];
                buffer[count+(int)chIdx] = value; /*! Initialize max */
                buffer[count+(int)(chIdx+totalChannelsNum)] = value; /*! Initialize min */
            }
            dataIdx = (dataIdx+1)&DDP_DATA_PACKETS_BUFFER_MASK;

            for (unsigned int downsamplingIdx = 1; downsamplingIdx < downsamplingSize; downsamplingIdx++) {
                for (chIdx = 0; chIdx < totalChannelsNum; chIdx++) {
                    value = floatDataSamplesBuffer[dataIdx][chIdx];
                    if (value > buffer[count+(int)chIdx]) {
                        buffer[count+(int)chIdx] = value;

                    } else if (value < buffer[count+(int)(chIdx+totalChannelsNum)]) {
                        buffer[count+(int)(chIdx+totalChannelsNum)] = value;
                    }
                }
                dataIdx = (dataIdx+1)&DDP_DATA_PACKETS_BUFFER_MASK;
            }
            count += (int)(totalChannelsNum << 1);
        }

    } else {
        while (count < dataSamplesToBuffer) {
            for (chIdx = 0; chIdx < totalChannelsNum; chIdx++) {
                buffer[count++] = floatDataSamplesBuffer[dataIdx][chIdx];
            }
            dataIdx = (dataIdx+1)&DDP_DATA_PACKETS_BUFFER_MASK;
        }
    }
    return true;
}
