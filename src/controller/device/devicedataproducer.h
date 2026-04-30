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

class AbstractDataHook {
public:
    virtual void setBufferSize(unsigned int bufferSize, unsigned int bufferMask) = 0;
    virtual bool getDataChunk(std::vector <unsigned short> &buffer, unsigned int downsamplingRatio = 1, unsigned int minDataBatchSize = 0) = 0;
    virtual bool getDataChunk(std::vector <double> &buffer, unsigned int downsamplingRatio = 1, unsigned int minDataBatchSize = 0) = 0;
    virtual bool getDataChunks(std::vector <double>& doubleBuffer, std::vector <short>& intBuffer, unsigned int minDataBatchSize = 0) = 0;
    virtual void flush() = 0;
    virtual bool getSweepNewFlag() {
        return false;
    };
    virtual int getSweepIdx() {
        return 0;
    };
    virtual bool getProtocolEndedFlag() {
        return false;
    };

protected:
    virtual bool waitDataAvailable(unsigned int minDataBatchSize, unsigned int &dataPacketsMax) = 0;
};

class DataHook : public AbstractDataHook {
public:
    DataHook(unsigned int totalChannelsNum);
    virtual ~DataHook();

    void setBufferSize(unsigned int bufferSize, unsigned int bufferMask) override;
    bool getDataChunk(std::vector <unsigned short> &buffer, unsigned int downsamplingRatio = 1, unsigned int minDataBatchSize = 0) override;
    bool getDataChunk(std::vector <double> &buffer, unsigned int downsamplingRatio = 1, unsigned int minDataBatchSize = 0) override;
    bool getDataChunks(std::vector <double>& doubleBuffer, std::vector <short>& intBuffer, unsigned int minDataBatchSize = 0) override;
    void flush() override;

protected:
    bool waitDataAvailable(unsigned int minDataBatchSize, unsigned int &dataPacketsMax) override;

private:
    unsigned int totalChannelsNum;

    unsigned int dataIdx;
    unsigned int bufferSize;
    unsigned int halfBufferSize;
    unsigned int bufferMask;
};

class EpisodicDataHook : public AbstractDataHook {
public:
    EpisodicDataHook(unsigned int totalChannelsNum, unsigned int protocolId, unsigned int sweepsNum);
    virtual ~EpisodicDataHook();

    void setBufferSize(unsigned int bufferSize, unsigned int bufferMask) override;
    bool getDataChunk(std::vector <unsigned short> &buffer, unsigned int downsamplingRatio = 1, unsigned int minDataBatchSize = 0) override;
    bool getDataChunk(std::vector <double> &buffer, unsigned int downsamplingRatio = 1, unsigned int minDataBatchSize = 0) override;
    bool getDataChunks(std::vector <double>& doubleBuffer, std::vector <short>& intBuffer, unsigned int minDataBatchSize = 0) override;
    void flush() override;
    bool getSweepNewFlag() override;
    int getSweepIdx() override;
    bool getProtocolEndedFlag() override;

protected:
    bool waitDataAvailable(unsigned int minDataBatchSize, unsigned int &dataPacketsMax) override;

private:
    unsigned int totalChannelsNum;
    unsigned int protocolId;
    int sweepsNum;
    unsigned int nextItemIdx = 0;
    int currentSweepIdx = -1;

    bool newSweepFlag = false;
    bool pushedNewSweepFlag = false;
    bool newSweepBuffer = false;

    bool protocolEndedFlag = false;
    bool pushedProtocolEndedFlag = false;
    bool protocolEndedBuffer = false;

    bool protocolFoundFlag = false;

    unsigned int dataIdx;
    unsigned int bufferSize;
    unsigned int halfBufferSize;
    unsigned int bufferMask;
};

class DeviceDataProducer : public QThread {
    Q_OBJECT

public:
    DeviceDataProducer(ApplicationStatus * appStatus, QObject * parent = nullptr);
    virtual ~DeviceDataProducer();

    unsigned int getDataPacketsBufferLen();
    DataHook * getDataHook();
    EpisodicDataHook * getEpisodicDataHook(unsigned int protocolId, unsigned int sweepsNum);

public slots:
    void onStopProducing();

protected:
    virtual void run() override;

private:
    ApplicationStatus * appStatus = nullptr;

    int voltageChannelsNum;
    int currentChannelsNum;
    int totalChannelsNum;
    int temperatureChannelsNum;

    double * temperatureValuesDbl;
    std::vector <Measurement_t> temperatureValues;

    double onTimeValueDbl[1];
    Measurement_t onTimeValue;

    unsigned int currentProtIdx = 0;
    unsigned int nextItemIdx = 0;

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
    void sigTemperatureRead(std::vector <Measurement_t> temperatureValues);
    void sigOnTimeRead(Measurement_t onTimeValue);
    void sigSyncFaults(std::vector <bool> syncFaults);
};

#endif // DEVICEDATAPRODUCER_H
