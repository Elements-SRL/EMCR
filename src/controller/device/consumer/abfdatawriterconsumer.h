#ifndef ABFDATAWRITERCONSUMER_H
#define ABFDATAWRITERCONSUMER_H

#define DWC_ABF_MAX_SAMPLES_FOR_BUFFERS 0x400000 // 4M
#define DWC_ABF_DATUM_RANGE 32768.0
#define DWC_ABF_CHANNEL_PER_FILE 2

#include <QUuid>

#include "datawriterconsumer.h"
#include "application_status.h"

class AbfDataWriterConsumer : public DataWriterConsumer {
    Q_OBJECT

public:
    AbfDataWriterConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);
    ~AbfDataWriterConsumer();

    virtual void setReadyForRecording(bool flag) override;
    virtual bool isReadyForRecording() override;

public slots:
//    virtual void onMarkTagTime() override;
//    virtual void onSaveTagString(QString tagString) override;

protected:
    AbstractDataHook * getDataHook() override;
    /*! bufferIdx is used to index the buffer to be written, it's mutable and referenced because it indicates where to start at the next call
        activeChannelsRatio is the total number of channels divided by the number of active channels
        returns the number of samples in the buffer not processed */
    virtual long long prepareBufferAndWriteToFile(long long &bufferIdx, double activeChannelsRatio);
    virtual void run() override;
    virtual void initAbfSections();
    virtual void initIVSections();
    void initISections();
    void initVSections();

    bool openFile() override;
    void manageConsumptionBegin() override;
    void manageConsumptionEnd() override;

    unsigned int blocksUsedBySection(ABF_Section section);
    unsigned int blocksUsedBySection(long long sectonSize);
//    void writeTagsInfo(ABF * abf);

    std::vector <unsigned short> buffer;

    unsigned short ** rawBuffers = nullptr;
    unsigned short ** rawVBuffers = nullptr;

    int voltageDecimationRatio = 1;

    unsigned int rawBuffersLen;
    long long maxDataSizeWritten;
    unsigned int maxMinPacketsPerBatch;
    QVector <ABF *> abfs;
    QVector <ABF *> abfsV;
    unsigned int blockIdx;
    unsigned int blockIdxV;
    int samplesFromTheBeginning = 0;

    float abfIntervalUsF32;
    float abfIntervalUsF32V;

//    int tagIdx = 0;
};

class EpisodicAbfDataWriterConsumer : public AbfDataWriterConsumer {
    Q_OBJECT

public:
    EpisodicAbfDataWriterConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);
    ~EpisodicAbfDataWriterConsumer();

    void setReadyForRecording(bool flag) override;
    bool isReadyForRecording() override;

    void setProtocolId(unsigned int protocolId);
    void setSweepsNum(unsigned int sweepsNum);

protected:
    AbstractDataHook * getDataHook() override;
    virtual long long prepareBufferAndWriteToFile(long long &bufferIdx, double activeChannelsRatio) override;
    void run() override;
    void initAbfSections() override;
    void initIVSections() override;

    void manageConsumptionBegin() override;
    void manageConsumptionEnd() override;

    void saveSynchInfo();
    void saveSynchInfo(ABF * abf);
    void writeSynchInfo(ABF * abf);

    int protocolId;
    int sweepIdx;
    int sweepsNum = 0;

    bool readyForRecording = false;

signals:
    void protocolFinished();
};

#endif // ABFDATAWRITERCONSUMER_H
