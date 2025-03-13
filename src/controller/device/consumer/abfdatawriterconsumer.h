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

public slots:
//    virtual void onMarkTagTime() override;
//    virtual void onSaveTagString(QString tagString) override;

protected:
    void run() override;
    void initAbfSections();
    void initIVSections();
    void initISections();
    void initVSections();

    bool openFile() override;
    void manageConsumptionBegin() override;
    void manageConsumptionEnd() override;

    unsigned int blocksUsedBySection(ABF_Section section);
    unsigned int blocksUsedBySection(long long sectonSize);
//    void saveSynchInfo();
//    void saveSynchInfo(ABF * abf);
//    void writeSynchInfo(ABF * abf);
//    void writeTagsInfo(ABF * abf);

    std::vector <unsigned short> buffer;

    unsigned short ** rawBuffers = nullptr;
    unsigned short ** rawVBuffers = nullptr;

    int voltageDecimationRatio = 1;

    unsigned int rawBuffersLen;
    unsigned int maxMinPacketsPerBatch;
    QVector <ABF *> abfs;
    QVector <ABF *> abfsV;
    unsigned int blockIdx;
    unsigned int blockIdxV;
    int samplesFromTheBeginning = 0;

    float abfIntervalUsF32;
    float abfIntervalUsF32V;

//    unsigned short sweepIdx;
//    int sweepsNum = 0;
//    int episodeStart;
//    int episodeLength;

//    int tagIdx = 0;
};

class EpisodicAbfDataWriterConsumer : public AbfDataWriterConsumer {
    Q_OBJECT

public:
    EpisodicAbfDataWriterConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);
    ~EpisodicAbfDataWriterConsumer();

public slots:
    void setDataHook(unsigned int protocolId, unsigned int sweepsNum);

protected:
    void run() override;
};

#endif // ABFDATAWRITERCONSUMER_H
