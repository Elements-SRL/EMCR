#include "datawriterconsumer.h"

#include <QMessageBox>

#include "globaldefines.h"

using namespace std;

DataWriterConsumer::DataWriterConsumer(ModelDevice * mDev, DeviceDataProducer * producer) :
    DeviceDataConsumer(mDev, producer) {

    activeChannels.resize(currentChannelsNum);
    activeChannels.fill(true);
    activeChannelsNum = totalChannelsNum; /*! This has to take into account also the voltage channel */

    pushedActiveChannels.resize(currentChannelsNum);
    pushedActiveChannels.fill(true);
    pushedActiveChannelsNum = totalChannelsNum; /*! This has to take into account also the voltage channel */

    connect(this, &QThread::started, this, [=] () {
        emit sigRecording(true);
    });

    connect(this, &QThread::finished, this, [=] () {
        emit sigRecording(false);
    });
}

DataWriterConsumer::~DataWriterConsumer() {

}

void DataWriterConsumer::onStartConsuming() {
    this->onStopConsuming();

    /*! During the recording the number of saved channels cannot change
     *  so when the recording starts freeze the active channels and use
     *  throughout the recording */
    pushedActiveChannels = activeChannels;
    pushedActiveChannelsNum = activeChannelsNum;
    if (dataFormat == settings.fileFormat) {
        recordingInitialized = false;
        if (!(this->openFile())) {
            emit sigError("Couldn't open the recording file");
            return;
        }

        hook = producer->getDataHook();
        if (hook != nullptr) {
            voltageRange = pushedVoltageRange;
            currentRange = pushedCurrentRange;
            sweepSamplingRateHz = pushedSamplingRateHz;

            this->computeSamples();

            this->start();

        } else {
            emit sigError("Couldn't get data");
        }
    }
}

void DataWriterConsumer::onStopConsuming() {
    if (this->isRunning()) {
        /*! \todo FCON verificare se bisogna gestire altre cose durante l'uccisione del thread precedente */
        QMutexLocker consumptionLock(&consumptionMtx);
        consumptionStopped = true;

        while (!exitedDataConsumingLoop) {
            exitedDataConsumingLoopCv.wait(&consumptionMtx);
        }
        consumptionLock.unlock();
    }
}

void DataWriterConsumer::onRecordingSettingsSet(RecordSettingsDialog::RecordSettings_t settings) {
    this->settings = settings;
    this->computeSamples();
}

void DataWriterConsumer::onSamplingRateChanged(Measurement_t samplingRate) {
    pushedSamplingRateHz = samplingRate.getNoPrefixValue();
}

void DataWriterConsumer::onVoltageRangeChanged(RangedMeasurement_t range) {
    pushedVoltageRange = range;
}

void DataWriterConsumer::onCurrentRangeChanged(RangedMeasurement_t range) {
    pushedCurrentRange = range;
}

void DataWriterConsumer::manageConsumptionBegin() {
    recordStartTime = QDateTime::currentDateTime();
    savedValues = 0;
    if (settings.chunkDurationS > 0.0) {
        if (totalValuesToBeSaved > valuesPerChunk) {
            /*! this means "we have to split a definite amount of data... " */
            splitFlag = true;
            valuesToBeSaved = valuesPerChunk;
            totalValuesToBeSaved -= valuesPerChunk;

        } else if (settings.recordDurationS == 0.0) {
            /*! this means "we have to split an indefinite amount of data... " */
            splitFlag = true;
            valuesToBeSaved = valuesPerChunk;

        } else {
            /*! this means we've reached the last chunk */
            splitFlag = false;
            valuesToBeSaved = totalValuesToBeSaved;
        }

    } else {
        splitFlag = false;
        valuesToBeSaved = totalValuesToBeSaved;
    }

    /*! "... but not yet" */
    splittingFlag = false;

    bufferLen = 0;
}

void DataWriterConsumer::manageConsumptionEnd() {
    if (!splittingFlag) {
        if (hook != nullptr) {
            delete hook;
            hook = nullptr;
        }
    }
}

//bool DataWriterConsumer::createHeaderFile() {
//    string headerFormat;
//    er4cl::getEdhFormat(headerFormat);
//    QString headerString = QString::fromStdString(headerFormat);
//    QString activeChannelsString = "";

//    for (int currentChannelIdx = 0; currentChannelIdx < currentChannelsNum; currentChannelIdx++) {
//        if (pushedActiveChannels[currentChannelIdx]) {
//            activeChannelsString += QString(" %1").arg(currentChannelIdx+1);
//        }
//    }

//    RangedMeasurement_t currentRange;
//    Measurement_t samplingRate;
//    uint16_t oversamplingRatio;

//    /*! For now recording with different current ranges is not supported, so all data is saved in the same range as channel 1 */
//    er4cl::getCurrentRange(currentRange, 0);
//    er4cl::getSamplingRate(samplingRate);

//    headerString.replace("%currentRange%", QString::fromStdString(currentRange.label()));
//    headerString.replace("%samplingRate%", QString::fromStdString(samplingRate.label()));
//    headerString.replace("%dateHour%", QDateTime::currentDateTime().toString("dd/MM/yy hh:mm:ss.zzz"));
//    headerString.replace("%activeChannels%", activeChannelsString);
//    if (er4cl::getOversamplingRatio(oversamplingRatio) == er4cl::Success) {
//        headerString.replace("%oversamplingRatio%", QString("%1").arg(oversamplingRatio));
//        headerString.replace("%oversamplingFlag%", "enabled"); /*! Always enabled, simply if the coefficient is 1 then the multiplication has no effect */
//    }

//    QString filePath = settingsDlg->getValidFilePath();
//    QString baseFileName = settingsDlg->getValidBaseFileName();

//    if (!QDir(filePath).exists()) {
//        QDir().mkdir(filePath);
//    }

//    headerFid.setFileName(filePath + baseFileName + headerExtension);
//    headerFid.open(QFile::WriteOnly | QFile::Truncate);
//    if (!headerFid.isOpen()) {
//        return false;
//    }

//    QTextStream stream(&headerFid);
//    stream << headerString;
//    headerFid.close();

//    return true;
//}

void DataWriterConsumer::computeSamples() {
    if (settings.recordDurationS == 0.0) {
        samplesToBeSaved = -1;
        totalValuesToBeSaved = -1;

    } else {
        samplesToBeSaved = (long long)qRound(settings.recordDurationS*sweepSamplingRateHz);
        totalValuesToBeSaved = samplesToBeSaved*(long long)totalChannelsNum;
    }

    if (settings.chunkDurationS > 0.0) {
        samplesPerChunk = (long long)qRound(settings.chunkDurationS*sweepSamplingRateHz);
        valuesPerChunk = samplesPerChunk*(long long)totalChannelsNum;

    } else {
        samplesPerChunk = -1;
        valuesPerChunk = -1;
    }
}

void DataWriterConsumer::findValidPathName() {
    QString newFullPathName;
    int pathIndex;
    QString newFullFileName;
    QString suffix;
    QString baseFileName = settings.filename;

    if (recordingInitialized) {
        suffix = channelIdxSuffix + QString("_%1").arg(chunkIdx++, 3, 10, QLatin1Char('0'));
        newFullFileName = settings.recordPath + baseFileName + suffix;

    } else {
        QString filePath = settings.recordPath;

        /*! Add date and time if required */
        QString dateTime = "";
        if (settings.appendDate) {
            dateTime = QDateTime::currentDateTime().toString("_yyMMddhhmmss");
        }
        baseFileName += dateTime;

        /*! Create subfolder for the recording */
        filePath += baseFileName;

        suffix = "";
        newFullPathName = filePath + suffix;
        pathIndex = 1;

        while (QDir().exists(newFullPathName)) {
            suffix.sprintf("_%d", pathIndex++);
            newFullPathName = filePath + suffix;
        }
        filePath = newFullPathName + "/";
        QString subFolder = baseFileName + suffix + "/";

        /*! Add suffix for chunk index (\todo FCON not really needed to look for available index if we put everything in subfolder) */
        suffix = channelIdxSuffix + "_000";
        newFullFileName = filePath + baseFileName + suffix;
        chunkIdx = 1;

        while (QDir().exists(newFullFileName + fileNameExtension)) {
            suffix = channelIdxSuffix + QString("_%1").arg(chunkIdx++, 3, 10, QLatin1Char('0'));
            newFullFileName = filePath + baseFileName + suffix;
        }
    }

    if (channelIdxSuffix != "") {
        suffix = suffix.arg(1, 3, 10, QLatin1Char('0'));
    }

//    validSubFileName = subFolder + baseFileName + suffix;
    validFullFileName = newFullFileName + fileNameExtension;
}
