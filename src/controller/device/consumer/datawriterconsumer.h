#ifndef DATAWRITERCONSUMER_H
#define DATAWRITERCONSUMER_H

#define DWC_TIME_FORMAT "hh:mm:ss.zzz"

#define DWC_DAT_RAW_BUFFER_LEN 4096
#define DWC_MIN_BATCH_DURATION 0.1

#include <QDateTime>

#include "devicedataconsumer.h"
#include "recordsettingsdialog.h"
#include "application_status.h"
#include "axon.h"

class DataWriterConsumer : public DeviceDataConsumer {
    Q_OBJECT

public:
    DataWriterConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer);
    virtual ~DataWriterConsumer();

public slots:
    virtual void onStartConsuming() override;
    virtual void onStopConsuming() override;

    void onRecordingSettingsSet(RecordSettingsDialog::RecordSettings_t settings);
    void onRecordSelectedChannels(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void onFilenameSet(QString);
    void onFilePathSet(QString);
    virtual void onSamplingRateChanged(Measurement_t samplingRate) override;
    virtual void onDownsamplingRatioChanged(unsigned int downsamplingRatio) override;
    virtual void onVoltageRangeChanged(RangedMeasurement_t range) override;
    virtual void onCurrentRangeChanged(RangedMeasurement_t range) override;
//    virtual void onMarkTagTime();
//    virtual void onSaveTagString(QString tagString);

protected:
    virtual bool openFile() = 0;
    virtual void manageConsumptionBegin();
    virtual void manageConsumptionEnd();
//    bool createHeaderFile();
    void computeSamples();
    void findValidPathName();

    long long samplesToBeSaved = 0;
    long long samplesPerChunk = -1;
    QString fileNameExtension = "";
    QString headerExtension = ".edh";
    QString channelIdxSuffix = "";
    bool recordingInitialized = false;

    int channelsPerFile = 1;
    RecordSettingsDialog::RecordFileFormat_t dataFormat = RecordSettingsDialog::RecordFileNone;

    QFile headerFid;
    std::vector <int> pushedActiveChannels;
    QVector <bool> pushedActiveChannelsFlag;
    int pushedActiveChannelsNum;
    std::vector<int> activeChannels;
    QVector <bool> activeChannelsFlag;
    int activeChannelsNum;
    QString filename;
    QString recordPath;
    RecordSettingsDialog::RecordSettings_t settings;
    QString baseFileName;
    QString validFullFileName;
    QString validFilePath;

    QDateTime recordStartTime;
    QDateTime lastRecordingInterruptionTime;
    long long msSinceRecordStarted;
//    bool tagTimeMarked = false;
//    int unnamedTagCount = 1;
    long long savedValues = 0;
    int bufferLen = 0;
    long long totalValuesToBeSaved;
    long long valuesToBeSaved;
    long long valuesPerChunk;
    bool splittingFlag = false;
    bool splitFlag = false;
    bool unlimitedFlag;
    bool chunkFlag;

    int chunkIdx;

signals:
    void sigRecording(bool on);
    void sigError(QString message);
};

#endif // DATAWRITERCONSUMER_H
