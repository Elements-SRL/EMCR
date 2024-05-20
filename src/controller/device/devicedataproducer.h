#ifndef DEVICEDATAPRODUCER_H
#define DEVICEDATAPRODUCER_H

#include <QVector>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTimer>

#include "application_status.h"

#define DDP_MAX_SAMPLES_FOR_BUFFER 0x2000000 // 32M
#define DDP_MAX_WAIT_COUNT (10)

class DataHook;

class DeviceDataProducer : public QThread {
    Q_OBJECT

public:
    DeviceDataProducer(ApplicationStatus * appStatus, QObject * parent = nullptr);
    virtual ~DeviceDataProducer();

    unsigned int getDataPacketsBufferLen();
    DataHook * getDataHook();

public slots:
    void onStopProducing();

protected:
    virtual void run() override;

private:
    ApplicationStatus * appStatus = nullptr;

    int voltageChannelsNum;
    int currentChannelsNum;
    int totalChannelsNum;

    bool deviceConnected = false;

    int16_t * datain;

    unsigned int dataPacketsBufferLen;
    unsigned int dataPacketsBufferMask;

    /*! Threads synchronization variables */
    mutable QMutex connectionMtx;
    QWaitCondition exitedDataProducingLoopCv;

    QTimer * bitRateTmr;
    int bitRateComputationIntervalS = 1; /*!< s */
    int samplesReceived;
    QMutex bitRateMtx;

private slots:
    void onComputeBitRate();

signals:
    void bitRateComputed(double); /*! ksps */
    void disconnectDevice();
    void resetDevice();
};

class DataHook {
public:
    DataHook(unsigned int totalChannelsNum);
    virtual ~DataHook();

    void setInitialOffset(unsigned int offset);
    void setBufferSize(unsigned int bufferSize, unsigned int bufferMask);
    bool getDataChunk(std::vector <unsigned short> &buffer, unsigned int downsamplingRatio = 1, unsigned int minDataBatchSize = 0);
    bool getDataChunk(std::vector <double> &buffer, unsigned int downsamplingRatio = 1, unsigned int minDataBatchSize = 0);
    bool getDataChunks(std::vector <double>& doubleBuffer, std::vector <short>& intBuffer, unsigned int minDataBatchSize = 0);
    void flush();

protected:
    int voltageChannelsNum;
    int currentChannelsNum;
    int totalChannelsNum;

    bool initialized = false;

    unsigned int dataIdx;
    unsigned int bufferSize;
    unsigned int halfBufferSize;
    unsigned int bufferMask;
};

#endif // DEVICEDATAPRODUCER_H
