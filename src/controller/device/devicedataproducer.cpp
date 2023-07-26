#include "devicedataproducer.h"

#include <qmath.h>
#include <QReadWriteLock>

static bool exitedDataProducingLoop = true;
static QReadWriteLock dataLock;
static QWaitCondition dataCv;
static QReadWriteLock ljLock;
static QWaitCondition ljCv;
static int16_t ** dataSamplesBuffer;
static double ** floatDataSamplesBuffer;
static double * floatLiquidJunctionBuffer;
static unsigned int dataPacketsIdx = 0;
static bool newLiquidJunctionData = false;

DeviceDataProducer::DeviceDataProducer(MessageDispatcher * msgDisp, QObject * parent) :
    QThread(parent),
    msgDisp(msgDisp) {

    bitRateTmr = new QTimer();
    bitRateTmr->setSingleShot(false);
    bitRateTmr->setInterval(bitRateComputationIntervalS*1000); /*!< Input in ms */
    connect(bitRateTmr, &QTimer::timeout, this, &DeviceDataProducer::onComputeBitRate);
    bitRateTmr->start();
    samplesReceived = 0;

    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);
    totalChannelsNum = voltageChannelsNum+currentChannelsNum;

    dataPacketsBufferLen = 1U << (unsigned int)qFloor(log2((double)DDP_MAX_SAMPLES_FOR_BUFFER/(double)totalChannelsNum));
    dataPacketsBufferMask = dataPacketsBufferLen-1U;

    dataSamplesBuffer = new int16_t * [dataPacketsBufferLen];
    floatDataSamplesBuffer = new double * [dataPacketsBufferLen];
    dataSamplesBuffer[0] = new int16_t[dataPacketsBufferLen*totalChannelsNum];
    floatDataSamplesBuffer[0] = new double[dataPacketsBufferLen*totalChannelsNum];
    for (unsigned int packetIdx = 1; packetIdx < dataPacketsBufferLen; packetIdx++) {
        dataSamplesBuffer[packetIdx] = dataSamplesBuffer[packetIdx-1]+totalChannelsNum;
        floatDataSamplesBuffer[packetIdx] = floatDataSamplesBuffer[packetIdx-1]+totalChannelsNum;
    }
    floatLiquidJunctionBuffer = new double[currentChannelsNum];
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
    dataLock.lockForRead();
    hook->setInitialOffset(dataPacketsIdx);
    dataLock.unlock();
    hook->setBufferSize(dataPacketsBufferLen, dataPacketsBufferMask);

    return hook;
}

LiquidJunctionHook * DeviceDataProducer::getLiquidJunctionHook() {
    return new LiquidJunctionHook(currentChannelsNum);
}

void DeviceDataProducer::onStopProducing() {
    if (!exitedDataProducingLoop) {
        QMutexLocker connectionLock(&connectionMtx);
        deviceConnected = false;
        while (!exitedDataProducingLoop) {
            exitedDataProducingLoopCv.wait(&connectionMtx);
        }
    }
    msgDisp->deallocateRxDataBuffer(datain);
}

void DeviceDataProducer::run() {
    RxOutput_t dataHeader;

    QMutexLocker connectionLock(&connectionMtx);
    deviceConnected = true;
    exitedDataProducingLoop = false;

    ErrorCodes_t ret;
    ret = msgDisp->allocateRxDataBuffer(datain);
    /*! \todo what to do if the memory is not initialized? */

    int chIdx;
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
            if (dataHeader.msgTypeId == MsgDirectionDeviceToPc+MsgTypeIdAcquisitionData) {
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

            } else if (dataHeader.msgTypeId == MsgDirectionDeviceToPc+MsgTypeIdDigitalOffsetComp) {
                ljLock.lockForWrite();
                msgDisp->convertLiquidJunctionValues(datain, floatLiquidJunctionBuffer, currentChannelsNum);
                newLiquidJunctionData = true;
                ljCv.wakeAll();
                ljLock.unlock();
            }

        } else {
            QThread::msleep(1);
        }
    }

    dataLock.lockForWrite();
    dataPacketsIdx = (dataPacketsIdx+(dataPacketsBufferLen >> 4)) & dataPacketsBufferMask;
    dataCv.wakeAll();
    dataLock.unlock();

    ljLock.lockForWrite();
    newLiquidJunctionData = true;
    ljCv.wakeAll();
    ljLock.unlock();

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

void DataHook::setBufferSize(unsigned int bufferSize, unsigned int bufferMask) {
    this->bufferSize = bufferSize;
    this->bufferMask = bufferMask;
    halfBufferSize = bufferSize/2;
}

bool DataHook::getDataChunk(QVector <unsigned short> &buffer, unsigned int, unsigned int minDataBatchSize) {
    int waitCount = 0;
    dataLock.lockForRead();
    while ((((dataIdx+minDataBatchSize-dataPacketsIdx) & bufferMask) <= halfBufferSize) &&
           (!exitedDataProducingLoop) &&
           waitCount++ < DDP_MAX_WAIT_COUNT) {
        dataCv.wait(&dataLock, 100);
    }

    if (waitCount >= DDP_MAX_WAIT_COUNT) {
        dataLock.unlock();
        return false;
    }

    unsigned int dataPacketsMax = dataPacketsIdx;
    dataLock.unlock();

    unsigned int dataPacketsToBuffer;
    if (dataIdx <= dataPacketsMax) {
        dataPacketsToBuffer = dataPacketsMax-dataIdx;

    } else {
        dataPacketsToBuffer = dataPacketsMax+bufferSize-dataIdx;
    }

    buffer.resize(dataPacketsToBuffer*totalChannelsNum);
    int count = 0;
    unsigned int chIdx;
    while (dataIdx != dataPacketsMax) {
        for (chIdx = 0; chIdx < totalChannelsNum; chIdx++) {
            buffer[count++] = dataSamplesBuffer[dataIdx][chIdx];
        }
        dataIdx = (dataIdx+1) & bufferMask;
    }
    return true;
}

bool DataHook::getDataChunk(QVector <double> &buffer, unsigned int downsamplingRatio, unsigned int minDataBatchSize) {
    int waitCount = 0;
    dataLock.lockForRead();
    while ((((dataIdx+minDataBatchSize-dataPacketsIdx) & bufferMask) <= halfBufferSize) &&
           (!exitedDataProducingLoop) &&
           waitCount++ < DDP_MAX_WAIT_COUNT) {
        dataCv.wait(&dataLock, 100);
    }

    if (waitCount >= DDP_MAX_WAIT_COUNT) {
        dataLock.unlock();
        return false;
    }

    unsigned int dataPacketsMax = dataPacketsIdx;
    dataLock.unlock();

    int dataPacketsToBuffer;
    unsigned int downsamplingSize = downsamplingRatio << 1; /*! to reduce size by x we take data in chunks of 2*x and then take max and min in the interval */
    if (downsamplingRatio > 1) {
        if (dataIdx <= dataPacketsMax) {
            dataPacketsToBuffer = (int)(((dataPacketsMax-dataIdx)/downsamplingSize) << 1);

        } else {
            dataPacketsToBuffer = (int)(((dataPacketsMax+bufferSize-dataIdx)/downsamplingSize) << 1);
        }

    } else {
        if (dataIdx <= dataPacketsMax) {
            dataPacketsToBuffer = (int)(dataPacketsMax-dataIdx);

        } else {
            dataPacketsToBuffer = (int)(dataPacketsMax+bufferSize-dataIdx);
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
            dataIdx = (dataIdx+1) & bufferMask;

            for (unsigned int downsamplingIdx = 1; downsamplingIdx < downsamplingSize; downsamplingIdx++) {
                for (chIdx = 0; chIdx < totalChannelsNum; chIdx++) {
                    value = floatDataSamplesBuffer[dataIdx][chIdx];
                    if (value > buffer[count+(int)chIdx]) {
                        buffer[count+(int)chIdx] = value;

                    } else if (value < buffer[count+(int)(chIdx+totalChannelsNum)]) {
                        buffer[count+(int)(chIdx+totalChannelsNum)] = value;
                    }
                }
                dataIdx = (dataIdx+1) & bufferMask;
            }
            count += (int)(totalChannelsNum << 1);
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

void DataHook::flush() {
    dataLock.lockForRead();
    dataIdx = dataPacketsIdx;
    dataLock.unlock();
}

LiquidJunctionHook::LiquidJunctionHook(unsigned int currentChannelsNum) :
    currentChannelsNum(currentChannelsNum) {

}

LiquidJunctionHook::~LiquidJunctionHook() {

}

bool LiquidJunctionHook::getLiquidJunctionValues(QVector<double> &buffer) {
    ljLock.lockForRead();
    while (!newLiquidJunctionData) {
        ljCv.wait(&ljLock, 100);
    }
    newLiquidJunctionData = false;

    buffer.resize(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        buffer[idx] = floatLiquidJunctionBuffer[idx];
    }
    ljLock.unlock();
    return true;
}
