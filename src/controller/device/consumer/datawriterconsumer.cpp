#include "datawriterconsumer.h"

#include "globaldefines.h"

DataWriterConsumer::DataWriterConsumer(ApplicationStatus * appStatus, DeviceDataProducer * producer) :
    DeviceDataConsumer(appStatus, producer) {

    activeChannels.resize(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        activeChannels[idx] = idx;
    }
    activeChannelsNum = totalChannelsNum; /*! This has to take into account also the voltage channels */

    pushedActiveChannels.resize(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        pushedActiveChannels[idx] = idx;
    }
    pushedActiveChannelsNum = totalChannelsNum; /*! This has to take into account also the voltage channels */

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
    activeChannels = pushedActiveChannels;
    activeChannelsNum = pushedActiveChannelsNum;
    if (dataFormat == settings.fileFormat) {
        recordingInitialized = false;
        if (!(this->openFile())) {
            emit sigError("Couldn't open the recording file");
            return;
        }

        hook = this->getDataHook();
        if (hook != nullptr) {
            voltageRange = pushedVoltageRange;
            currentRange = pushedCurrentRange;
            samplingRateHz = pushedSamplingRateHz/(double)pushedDownsamplingRatio;
            downsamplingRatio = pushedDownsamplingRatio;

            this->computeSamples();

            QMutexLocker consumptionLock(&consumptionMtx);
            consumptionStopped = false;
            exitedDataConsumingLoop = false;

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
            exitedDataConsumingLoopCv.wait(&consumptionMtx, 100);
        }
    }
}

void DataWriterConsumer::onRecordingSettingsSet(RecordSettingsDialog::RecordSettings_t settings) {
    this->settings = settings;
    this->computeSamples();
}

void DataWriterConsumer::onRecordSelectedChannels(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues) {
    this->onStopConsuming();
    pushedActiveChannels.clear();
    pushedActiveChannelsNum = 0;

    for (int idx = 0; idx < channelIndexes.size(); idx++) {
        if (onValues[idx]) {
            pushedActiveChannels.push_back(channelIndexes[idx]);
            pushedActiveChannelsNum+=channelsPerFile;
        }
    }
    this->setReadyForRecording(true);
}

void DataWriterConsumer::onSamplingRateChanged(Measurement_t samplingRate) {
    pushedSamplingRateHz = samplingRate.getNoPrefixValue();
    pushedSamplingRateFlag = true;
    if (!this->isRunning()){
        samplingRateHz = pushedSamplingRateHz/(double)downsamplingRatio;
    }
}

void DataWriterConsumer::onDownsamplingRatioChanged(unsigned int ratio) {
    pushedDownsamplingRatio = ratio;
    pushedDownsamplingRatioFlag = true;
    pushedSamplingRateFlag = true;
    if (!this->isRunning()){
        samplingRateHz = pushedSamplingRateHz/(double)ratio;
    }
}

void DataWriterConsumer::onVoltageRangeChanged() {
    pushedVoltageRange = this->getAppStatus()->getVoltageRanges();
    pushedVoltageRangeFlag = true;
}

void DataWriterConsumer::onCurrentRangeChanged() {
    pushedCurrentRange = this->getAppStatus()->getCurrentRanges();
    pushedCurrentRangeFlag = true;
}

void DataWriterConsumer::onClampingModalityChanged(ClampingModality_t mode) {
    pushedClampingModality = mode;
    pushedClampingModalityFlag = true;
}

void DataWriterConsumer::manageConsumptionBegin() {
    recordStartTime = QDateTime::currentDateTime();
    savedValues = 0;
    if (chunkFlag) {
        if (totalValuesToBeSaved > valuesPerChunk) {
            /*! this means "we have to split a definite amount of data... " */
            splitFlag = true;
            valuesToBeSaved = valuesPerChunk;
            totalValuesToBeSaved -= valuesPerChunk;

        } else if (unlimitedFlag) {
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
//        if (pushedActiveChannelsFlag[currentChannelIdx]) {
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
    std::vector <bool> selectedChannels = appStatus->getSelectedChannels();
    activeChannelsNum = 0;
    for (auto v: selectedChannels) {
        if (v) {
            activeChannelsNum += channelsPerFile;
        }
    }

    /*! First samples computation, might not be the final one because of specific behaviorus when durations are 0 */
    samplesToBeSaved = (long long) (settings.recordDurationS*samplingRateHz);
    samplesPerChunk = (long long) (settings.chunkDurationS*samplingRateHz);

    unlimitedFlag = settings.recordDurationS == 0.0;
    chunkFlag = settings.chunkDurationS > 0.0;

    long long maxSamplesPerChunk = PSD_MAX_MB_PER_FILE*BYTES_PER_MEGA_BYTES/(bytesPerChannel*channelsPerFile);

//    true only for abf
    if (unlimitedFlag && chunkFlag) {
        if (samplesPerChunk > maxSamplesPerChunk) {
            samplesPerChunk = maxSamplesPerChunk;
        }

    } else if (unlimitedFlag && !chunkFlag) {
        chunkFlag = true;
        samplesPerChunk = maxSamplesPerChunk;

    } else if (!unlimitedFlag && chunkFlag) {
        if (samplesPerChunk > maxSamplesPerChunk) {
            samplesPerChunk = maxSamplesPerChunk;
        }
        if (samplesPerChunk >= samplesToBeSaved) {
            chunkFlag = false;
            samplesPerChunk = -1.0;
        }

    } else if (!unlimitedFlag && !chunkFlag) {
        if (samplesToBeSaved > maxSamplesPerChunk){
            chunkFlag = true;
            samplesPerChunk = maxSamplesPerChunk;
        }
    }

    if (unlimitedFlag) {
        samplesToBeSaved = -1;
        totalValuesToBeSaved = -1;

    } else {
        totalValuesToBeSaved = samplesToBeSaved*(long long)activeChannelsNum;
    }

    if (chunkFlag) {
        valuesPerChunk = samplesPerChunk*(long long)activeChannelsNum;

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
    if (recordingInitialized) {
        suffix = channelIdxSuffix + QString("_%1").arg(chunkIdx++, 3, 10, QLatin1Char('0'));
        newFullFileName = validFilePath + baseFileName + suffix;

    } else {
        baseFileName = filename;
        validFilePath = recordPath;
        //    if recordings folder does not exist, create it
        QDir dir(validFilePath);
        if (!dir.exists()) {
            QDir().mkpath(validFilePath);
        }
        /*! Add date and time if required */
        QString dateTime = "";
        if (settings.appendDate) {
            dateTime = QDateTime::currentDateTime().toString("_yyMMddhhmmss");
        }
        baseFileName += dateTime;

        /*! Create subfolder for the recording */
        validFilePath += baseFileName;

        suffix = "";
        newFullPathName = validFilePath + suffix;
        pathIndex = 1;

        while (QDir().exists(newFullPathName)) {
            suffix = QString("_%1").arg(pathIndex++);
            newFullPathName = validFilePath + suffix;
        }
        validFilePath = newFullPathName + "/";
        QString subFolder = baseFileName + suffix + "/";

        /*! Add suffix for chunk index (\todo FCON not really needed to look for available index if we put everything in subfolder) */
        suffix = channelIdxSuffix + "_000";
        newFullFileName = validFilePath + baseFileName + suffix;
        chunkIdx = 1;

        while (QDir().exists(newFullFileName + fileNameExtension)) {
            suffix = channelIdxSuffix + QString("_%1").arg(chunkIdx++, 3, 10, QLatin1Char('0'));
            newFullFileName = validFilePath + baseFileName + suffix;
        }
    }

    if (channelIdxSuffix != "") {
        suffix = suffix.arg(1, 3, 10, QLatin1Char('0'));
    }

//    validSubFileName = subFolder + baseFileName + suffix;
    validFullFileName = newFullFileName + fileNameExtension;
    validVoltageFullFileName = newFullFileName + "V" + fileNameExtension;
}

void DataWriterConsumer::onFilenameSet(QString fname){
    filename = fname;
}
void DataWriterConsumer::onFilePathSet(QString path){
    recordPath = path;
}
