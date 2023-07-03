#ifndef ABFDATAWRITERCONSUMER_H
#define ABFDATAWRITERCONSUMER_H

#define DWC_ABF_MAX_SAMPLES_FOR_BUFFERS 0x400000 // 4M
#define DWC_ABF_DATUM_RANGE 32768.0
#define DWC_ABF_CHANNEL_PER_FILE 2

#include <QUuid>

#include "datawriterconsumer.h"
#include "messagedispatcher.h"

class AbfDataWriterConsumer : public DataWriterConsumer {
    Q_OBJECT

public:
    AbfDataWriterConsumer(MessageDispatcher * msgDisp, DeviceDataProducer * producer);
    ~AbfDataWriterConsumer();

public slots:
//    virtual void onMarkTagTime() override;
//    virtual void onSaveTagString(QString tagString) override;

protected:
    void run() override;
    void initAbfSections();

    bool openFile() override;
    void manageConsumptionBegin() override;
    void manageConsumptionEnd() override;

    unsigned int blocksUsedBySection(ABF_Section section);
    unsigned int blocksUsedBySection(long long sectonSize);
//    void saveSynchInfo();
//    void saveSynchInfo(ABF * abf);
//    void writeSynchInfo(ABF * abf);
//    void writeTagsInfo(ABF * abf);

    QVector <unsigned short> buffer;

    unsigned short ** rawBuffers = nullptr;
    unsigned int rawBuffersLen;
    unsigned int minPacketsPerBatch;
    QVector <ABF *> abfs;
    unsigned int blockIdx;
    int samplesFromTheBeginning = 0;

    float abfIntervalUsF32;

//    unsigned short sweepIdx;
//    int sweepsNum = 0;
//    int episodeStart;
//    int episodeLength;

//    int tagIdx = 0;
};

#endif // ABFDATAWRITERCONSUMER_H
