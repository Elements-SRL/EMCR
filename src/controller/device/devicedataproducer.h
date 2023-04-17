#ifndef DEVICEDATAPRODUCER_H
#define DEVICEDATAPRODUCER_H

#include <QThread>
#include <QMutex>
#include <QWaitCondition>
#include <QTimer>

#include "modeldevice.h"

#define DDP_DATA_PACKETS_BUFFER_LEN 0x40000 // 250k /*! \todo questo valore va abbassato, i plotconsumer prendono troppa memoria */
#define DDP_DATA_PACKETS_BUFFER_MASK (DDP_DATA_PACKETS_BUFFER_LEN-1)
#define DDP_MAX_WAIT_COUNT (10)

class DataHook;

class DeviceDataProducer : public QThread {
    Q_OBJECT

public:
    DeviceDataProducer(ModelDevice * mDev, QObject * parent = nullptr);
    virtual ~DeviceDataProducer();

    DataHook * getDataHook();

public slots:
    void onStopProducing();

protected:
    virtual void run() override;

private:
    ModelDevice * mDev;

    int voltageChannelsNum;
    int currentChannelsNum;
    int totalChannelsNum;

    bool deviceConnected = false;

    int16_t * datain;

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
    bool getDataChunk(QVector <unsigned short> &buffer, unsigned int downsamplingRatio = 1, unsigned int minDataBatchSize = 0);
    bool getDataChunk(QVector <double> &buffer, unsigned int downsamplingRatio = 1, unsigned int minDataBatchSize = 0);
    void flush();

protected:
    int voltageChannelsNum;
    int currentChannelsNum;
    int totalChannelsNum;

    bool initialized = false;

    unsigned int dataIdx;
};

#endif // DEVICEDATAPRODUCER_H
