#include "abfdatawriterconsumer.h"

#include <qmath.h>

AbfDataWriterConsumer::AbfDataWriterConsumer(MessageDispatcher * msgDisp, DeviceDataProducer * producer) :
    DataWriterConsumer(msgDisp, producer) {

    dataFormat = RecordSettingsDialog::RecordFileAbf;
    fileNameExtension = ".abf";
    channelIdxSuffix = "_CH%1";
    bytesPerChannel = 2;
    channelsPerFile = DWC_ABF_CHANNEL_PER_FILE;

    rawBuffersLen = 1U << (unsigned int)qFloor(log2((double)DWC_ABF_MAX_SAMPLES_FOR_BUFFERS/(double)(DWC_ABF_CHANNEL_PER_FILE*currentChannelsNum)));
    maxMinPacketsPerBatch = rawBuffersLen/2;

    rawBuffers = new unsigned short* [currentChannelsNum];
    rawBuffers[0] = new unsigned short [rawBuffersLen*DWC_ABF_CHANNEL_PER_FILE*currentChannelsNum];
    for (int channelIdx = 1; channelIdx < currentChannelsNum; channelIdx++) {
        rawBuffers[channelIdx] = rawBuffers[channelIdx-1]+rawBuffersLen*DWC_ABF_CHANNEL_PER_FILE;
    }
    abfs.resize(currentChannelsNum);
    abfs.fill(nullptr);

    /*! Allocate buffer max size once and for all, so we avoid real time memory reallocations */
    buffer.reserve(producer->getDataPacketsBufferLen()*totalChannelsNum);
}

AbfDataWriterConsumer::~AbfDataWriterConsumer() {
    this->onStopConsuming();

    if (rawBuffers[0] != nullptr) {
        delete [] rawBuffers[0];
        rawBuffers[0] = nullptr;
    }

    if (rawBuffers != nullptr) {
        delete [] rawBuffers;
        rawBuffers = nullptr;
    }

    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        ABF * abf = abfs[channelIdx];
        if (abf != nullptr) {
            delete abf;
            abf = nullptr;
        }
    }
}

//void AbfDataWriterConsumer::onMarkTagTime() {
//    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
//        ABF * abf = abfs[channelIdx];
//        if (abf != nullptr) {
//            DataWriterConsumer::onMarkTagTime();

//            abf->TagsInfo[tagIdx].lStart = samplesFromTheBeginning*totalChannelsNum;
//            //    abf->TagsInfo[tagIdx].lStart = (int)(msSinceRecordStarted*2000L/abfIntervalUsI64);

//        }
//    }
//}

//void AbfDataWriterConsumer::onSaveTagString(QString tagString) {
//    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
//        ABF * abf = abfs[channelIdx];
//        if (abf != nullptr) {
//            if (tagString != "__INVALIDTAG__") {
//                DataWriterConsumer::onSaveTagString(tagString);

//                int charCopied = (tagString.length() < ABF_TAGCOMMENTLEN ? tagString.length() : ABF_TAGCOMMENTLEN);
//                strncpy(abf->TagsInfo[tagIdx++].tag, tagString.toStdString().c_str(), (unsigned int)charCopied);
//            }
//        }
//    }
//}

void AbfDataWriterConsumer::run() {
    this->manageConsumptionBegin();

    /*! Initialize consumer parameters */
    long long bufferIdx;
    long long rawBufferLen;
    /*! rawBuffer has size DWC_ABF_RAW_BUFFER_LEN, and since data is written DWC_ABF_CHANNEL_PER_FILE values at a time, the maximum safe size for
     *  data written is less than DWC_ABF_RAW_BUFFER_LEN and divisible by DWC_ABF_CHANNEL_PER_FILE */
    long long maxDataSizeWritten = (rawBuffersLen/DWC_ABF_CHANNEL_PER_FILE)*DWC_ABF_CHANNEL_PER_FILE;
    long long rawBufferIdx;
    long long truncatedValues = 0;
    unsigned int minPacketsPerBatch = qMin(maxMinPacketsPerBatch, (unsigned int)qRound(samplingRateHz*DWC_MIN_BATCH_DURATION));

    QMutexLocker consumptionLock(&consumptionMtx);
    consumptionStopped = false;
    exitedDataConsumingLoop = false;
    consumptionLock.unlock();

    long long activeChannelsRatio = totalChannelsNum/activeChannelsNum;
    while (true) {
        consumptionLock.relock();
        if (consumptionStopped) {
            break;
        }
        consumptionLock.unlock();

        if (hook->getDataChunk(buffer, 1, minPacketsPerBatch)) {
            bufferIdx = 0;

            bufferLen = buffer.size();
            long long valuesLen = bufferLen / activeChannelsRatio;
            long long valuesToEndOfFile = valuesToBeSaved-savedValues;

            if (valuesLen >= valuesToEndOfFile) {
                truncatedValues = valuesLen - valuesToEndOfFile;
                valuesLen = valuesToEndOfFile;
                bufferLen = valuesToEndOfFile * activeChannelsRatio;
                if (splitFlag) {
                    splittingFlag = true;

                } else {
                    consumptionStopped = true;
                }
            }
            savedValues += valuesLen;
//            if (recordSettings.episodicFlag && (savedValues >= valuesPerSweep*(long long)(sweepIdx+1)) && (savedValues < valuesToBeSaved)) {
//                /*! if the episodic feature is active and enough values for a sweep have been saved store the sweep info */
//                /*! moreover, do this only if not all values have been saved, since the last sweep is managed when the file is closed */
//                this->saveSynchInfo();
//                episodeStart += samplesPerSweep;
//                sweepIdx++;
//            }

            while (bufferLen > 0) {
                rawBufferIdx = 0;
                rawBufferLen = qMin((long long)(bufferLen*DWC_ABF_CHANNEL_PER_FILE/totalChannelsNum), maxDataSizeWritten);
                bufferLen -= rawBufferLen*totalChannelsNum/DWC_ABF_CHANNEL_PER_FILE;

                while (rawBufferIdx < rawBufferLen) {
                    for (int channelIdx : activeChannels) {
                        rawBuffers[channelIdx][rawBufferIdx] = buffer[bufferIdx+voltageChannelsNum+channelIdx];
                        rawBuffers[channelIdx][rawBufferIdx+1] = buffer[bufferIdx+channelIdx];
                    }
                    rawBufferIdx += DWC_ABF_CHANNEL_PER_FILE;

                    samplesFromTheBeginning++;
                    bufferIdx += totalChannelsNum;
                }
                for (int channelIdx : activeChannels) {
                    abfs[channelIdx]->WriteRawData(rawBuffers[channelIdx], sizeof(unsigned short), rawBufferLen);
                }
            }

            if (splittingFlag) {
                /*! close the recording and start a new one */

                this->manageConsumptionEnd();
                if (!(this->openFile())) {
                    consumptionStopped = true;
                    emit sigError("Couldn't open the recording file");
                }
                this->manageConsumptionBegin();

                bufferLen = truncatedValues * activeChannelsRatio;
                long long valuesLen = bufferLen / activeChannelsRatio;
                long long valuesToEndOfFile = valuesToBeSaved-savedValues;
                if (valuesLen >= valuesToEndOfFile) {
                    /*! Assume a single buffer cannot span more than 1 file chunk
                     *  So we're not checking for splitting condition here */
                    truncatedValues = valuesLen - valuesToEndOfFile;
                    valuesLen = valuesToEndOfFile;
                    bufferLen = valuesToEndOfFile * activeChannelsRatio;
                }
                savedValues += valuesLen;
//                if (recordSettings.episodicFlag && (savedValues >= valuesPerSweep*(long long)(sweepIdx+1)) && (savedValues < valuesToBeSaved)) {
//                    /*! if the episodic feature is active and enough values for a sweep have been saved store the sweep info */
//                    /*! moreover, do this only if not all values have been saved, since the last sweep is managed when the file is closed */
//                    this->saveSynchInfo();
//                    episodeStart += samplesPerSweep;
//                    sweepIdx++;
//                }

                /*! manage the recording of data left in the buffer */
                while (bufferLen > 0) {
                    rawBufferIdx = 0;
                    rawBufferLen = qMin((long long)(bufferLen*DWC_ABF_CHANNEL_PER_FILE/totalChannelsNum), maxDataSizeWritten);
                    bufferLen -= rawBufferLen*totalChannelsNum/DWC_ABF_CHANNEL_PER_FILE;

                    while (rawBufferIdx < rawBufferLen) {
                        for (int channelIdx : activeChannels) {
                            rawBuffers[channelIdx][rawBufferIdx] = buffer[bufferIdx+voltageChannelsNum+channelIdx];
                            rawBuffers[channelIdx][rawBufferIdx+1] = buffer[bufferIdx+channelIdx];
                        }
                        rawBufferIdx += DWC_ABF_CHANNEL_PER_FILE;

                        samplesFromTheBeginning++;
                        bufferIdx += totalChannelsNum;
                    }
                    for (int channelIdx : activeChannels) {
                        abfs[channelIdx]->WriteRawData(rawBuffers[channelIdx], sizeof(unsigned short), rawBufferLen);
                    }
                }
            }
        }
    }

    this->manageConsumptionEnd();

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}

void AbfDataWriterConsumer::initAbfSections() {
    for (int channelIdx : activeChannels) {
        ABF * abf = abfs[channelIdx];
        abf->InitFileInfo();
        abf->InitStrings();
        blockIdx = 1; /*!< First block available */

        /*! File info section */
        int date = 0;
        date += QDateTime::currentDateTime().date().year()*10000;
        date += QDateTime::currentDateTime().date().month()*100;
        date += QDateTime::currentDateTime().date().day();
        abf->FileInfo.uFileStartDate = (unsigned int)date;

        int time = 0;
        time += QDateTime::currentDateTime().time().hour()*3600000;
        time += QDateTime::currentDateTime().time().minute()*60000;
        time += QDateTime::currentDateTime().time().second()*1000;
        time += QDateTime::currentDateTime().time().msec();
        abf->FileInfo.uFileStartTimeMS = (unsigned int)time;

        qsrand((unsigned int)(QTime::currentTime().msec()));
        QUuid uuid = QUuid::createUuid();
        abf->FileInfo.FileGUID.Data1 = uuid.data1;
        abf->FileInfo.FileGUID.Data2 = uuid.data2;
        abf->FileInfo.FileGUID.Data3 = uuid.data3;
        for (int j = 0; j < 8; j++) {
            abf->FileInfo.FileGUID.Data4[j] = uuid.data4[j];
        }

        QStringList ver = QString(GLB_SOFTWARE_VERSION_NUMBER).split('.');
        unsigned int major = ver[0].toUInt();
        unsigned int minor = ver[1].toUInt();
        unsigned int bugfix = ver[2].toUInt();
        unsigned int build = 0;
        unsigned int version = (major << 24) + (minor << 16) + (bugfix << 8) + build;
        abf->FileInfo.uCreatorVersion = version;
        abf->PutString(QString(GLB_SOFTWARE_NAME).remove(" ").toStdString().c_str());
        abf->FileInfo.uCreatorNameIndex = (unsigned int)(abf->GetStringNumber());

        /*! Protocol section */
        abf->InitProtocolSection();
        abf->FileInfo.ProtocolSection.uBlockIndex = blockIdx;
        abf->FileInfo.ProtocolSection.uBytes = 512;
        abf->FileInfo.ProtocolSection.llNumEntries = 1;
        blockIdx += this->blocksUsedBySection(abf->FileInfo.ProtocolSection);

        //            if (recordSettings.episodicFlag == true) {
        //                abf->ProtocolInfo.nOperationMode = ABF_WAVEFORMFILE;
        //                abf->ProtocolInfo.fSynchTimeUnit = abfIntervalUsF32;
        //                abf->ProtocolInfo.lNumberOfTrials = 1;

        //            } else {
        abf->ProtocolInfo.nOperationMode = ABF_GAPFREEFILE;
        abf->ProtocolInfo.fSynchTimeUnit = 0.0;
        abf->ProtocolInfo.lNumberOfTrials = 0;
        //            }
        abf->ProtocolInfo.fADCSequenceInterval = abfIntervalUsF32;
        abf->ProtocolInfo.fADCRange = 1.0;

        /*! Strings */
        abf->PutString(QString("(untitled)").toStdString().c_str());

        abf->PutString(QString("I%1").arg(channelIdx).toStdString().c_str());
        int iStringIdx = abf->GetStringNumber();

        abf->PutString(currentRange.getFullUnit().c_str());
        int iStringUnitIdx = abf->GetStringNumber();

        abf->PutString(QString("V%1").arg(channelIdx).toStdString().c_str());
        int vStringIdx = abf->GetStringNumber();

        abf->PutString(voltageRange.getFullUnit().c_str());
        int vStringUnitIdx = abf->GetStringNumber();

        /*! ADC section */
        short adcNum = 2; /*! We only save one current and the voltage in each abf file */
        abf->FileInfo.ADCSection.uBlockIndex = blockIdx;
        abf->FileInfo.ADCSection.uBytes = 128;
        abf->FileInfo.ADCSection.llNumEntries = adcNum;
        blockIdx += this->blocksUsedBySection(abf->FileInfo.ADCSection);

        short adcIdx = 0;
        abf->InitADCSection(adcIdx);
        abf->ADCInfo[adcIdx].nADCNum = adcIdx;
        abf->ADCInfo[adcIdx].nADCPtoLChannelMap = adcIdx;
        abf->ADCInfo[adcIdx].nADCSamplingSeq = 0;
        abf->ADCInfo[adcIdx].fInstrumentScaleFactor = 1.0F/(float)(currentRange.step*32768.0);
        abf->ADCInfo[adcIdx].fInstrumentOffset = 0.0F;
        abf->ADCInfo[adcIdx].fADCProgrammableGain = 1.0F;

        abf->ADCInfo[adcIdx].lADCChannelNameIndex = iStringIdx;
        abf->ADCInfo[adcIdx].lADCUnitsIndex = iStringUnitIdx;

        adcIdx = 1;
        abf->InitADCSection(adcIdx);
        abf->ADCInfo[adcIdx].nADCNum = adcIdx;
        abf->ADCInfo[adcIdx].nADCPtoLChannelMap = adcIdx;
        abf->ADCInfo[adcIdx].nADCSamplingSeq = 0;
        abf->ADCInfo[adcIdx].fInstrumentScaleFactor = 1.0F/(float)(voltageRange.step*32768.0);
        abf->ADCInfo[adcIdx].fInstrumentOffset = 0.0F;
        abf->ADCInfo[adcIdx].fADCProgrammableGain = 1.0F;

        abf->ADCInfo[adcIdx].lADCChannelNameIndex = vStringIdx;
        abf->ADCInfo[adcIdx].lADCUnitsIndex = vStringUnitIdx;

        /*! DAC section */
        abf->FileInfo.DACSection.uBlockIndex = blockIdx;
        abf->FileInfo.DACSection.uBytes = 256;
        abf->FileInfo.DACSection.llNumEntries = 8;
        blockIdx += this->blocksUsedBySection(abf->FileInfo.DACSection);

        for (short dacIdx = 0; dacIdx < ABF_DACCOUNT; dacIdx++) {
            abf->InitDACSection(dacIdx);
            abf->DACInfo[dacIdx].nDACNum = dacIdx;
            abf->DACInfo[dacIdx].lDACChannelNameIndex = vStringIdx;
            abf->DACInfo[dacIdx].lDACChannelUnitsIndex = vStringUnitIdx;
        }

        /*! Stats region section */
        abf->InitStatsRegionSection();
        abf->FileInfo.StatsRegionSection.uBlockIndex = blockIdx;
        abf->FileInfo.StatsRegionSection.uBytes = 128;
        abf->FileInfo.StatsRegionSection.llNumEntries = 1;
        blockIdx += this->blocksUsedBySection(abf->FileInfo.StatsRegionSection);

        /*! Strings section */
        abf->FileInfo.StringsSection.uBlockIndex = blockIdx;
        abf->FileInfo.StringsSection.uBytes = (unsigned int)(abf->GetStringsTotLen());
        abf->FileInfo.StringsSection.llNumEntries = abf->GetStringNumber();
        blockIdx += this->blocksUsedBySection(abf->GetStringsTotLen());

        /*! Scope section */
        abf->InitScopeSection();
        abf->FileInfo.ScopeSection.uBlockIndex = blockIdx;
        abf->FileInfo.ScopeSection.uBytes = 769;
        abf->FileInfo.ScopeSection.llNumEntries = 1;
        blockIdx += this->blocksUsedBySection(abf->FileInfo.ScopeSection);

        abf->ScopeInfo.fSamplingRateHz = (float)samplingRateHz;
        sprintf(abf->ScopeInfo.channels[0].sName, "I%d", channelIdx);
        sprintf(abf->ScopeInfo.channels[1].sName, "V%d", channelIdx);

        for (short adcIdx = 0; adcIdx < adcNum; adcIdx++) {
            abf->ScopeInfo.channels[adcIdx].lIndex = adcIdx;
            abf->ScopeInfo.channels[adcIdx].nDisplayType = ABF_DISPLAY_SIGNAL;
            abf->ScopeInfo.channels[adcIdx].fPlotHeightFactor = 0.2F;
            abf->ScopeInfo.channels[adcIdx].uUnknown003 = 0x000100ff;
            abf->ScopeInfo.channels[adcIdx].fUnknown004[0] = 1.0;
            abf->ScopeInfo.channels[adcIdx].fUnknown004[1] = 0.0;
        }
        abf->ScopeInfo.nADCNum = adcNum;

        /*! Data section */
        abf->FileInfo.DataSection.uBlockIndex = blockIdx;
        abf->FileInfo.DataSection.uBytes = 2;
        abf->FileInfo.DataSection.llNumEntries = 0; /*!< this is an initialization, this number will increase everytime new data is added */

        /*! Tags section */
        abf->FileInfo.TagSection.uBlockIndex = 0; /*!< we don't know yet were tags will be, they will be placed after the data */
        abf->FileInfo.TagSection.uBytes = 64;
        abf->FileInfo.TagSection.llNumEntries = 0; /*!< this is an initialization, this number will increase everytime a new tag is added */

        /*! Write sections to file */
        abf->WriteADCInfo();
        abf->WriteDACInfo();
        abf->WriteStatsRegion();
        abf->WriteStrings();
        abf->WriteScopeInfo();
    }
}

bool AbfDataWriterConsumer::openFile() {
//    QString validSubFileName;
    this->findValidPathName();

    /*! Create header file */
//    if (!(this->createHeaderFile())) {
//        return false;
//    }

//    emit newRecordFile(validSubFileName);
    recordingInitialized = true;

    for (int channelIdx : activeChannels) {
        ABF * abf = new ABF();

        if (!QDir(validFilePath).exists()) {
            QDir().mkdir(validFilePath);
        }

        int openOk = abf->Open(const_cast <char *> (validFullFileName.arg(channelIdx+1, 3, 10, QLatin1Char('0')).toStdString().c_str()), QFile::WriteOnly | QFile::Truncate);

        if (openOk != AXON_INFO_OK) {
            delete abf;
            abf = nullptr;
            return false;
        }
        abfs[channelIdx] = abf;
    }

    return true;
}

void AbfDataWriterConsumer::manageConsumptionBegin() {
    DataWriterConsumer::manageConsumptionBegin();

    /*! Initialize abf sections */
    abfIntervalUsF32 = (float)(1000000.0/samplingRateHz);

    this->initAbfSections();

    samplesFromTheBeginning = 0;
//    tagIdx = 0;
//    sweepIdx = 0;
//    episodeStart = 0;
//    episodeLength = samplesPerSweep*(long long)DWC_ABF_CHANNEL_PER_FILE;
}

void AbfDataWriterConsumer::manageConsumptionEnd() {
    unsigned int blockIdxOffset = blockIdx;

//    if (recordSettings.episodicFlag) {
//        sweepsNum = sweepIdx+1;
//        episodeLength = ((savedValues-valuesPerSweep*(long long)sweepIdx)/(long long)totalChannelsNum)*(long long)DWC_ABF_CHANNEL_PER_FILE;
//    }

    for (int channelIdx : activeChannels) {
        ABF * abf = abfs[channelIdx];
        if (abf != nullptr) {
            blockIdx = blockIdxOffset+this->blocksUsedBySection(abf->FileInfo.DataSection);

//            if (recordSettings.episodicFlag) {
//                this->saveSynchInfo(abf);
//                this->writeSynchInfo(abf);
//            }

//            if (tagTimeMarked) {
//                /*! If there's a tag request open when the recording ends save an empty tag */
//                this->onMarkTagTime();
//            }

//            this->writeTagsInfo(abf);

            abf->WriteFileInfo();
            abf->WriteProtocolInfo();
            abf->Close();

            delete abf;
            abf = nullptr;
        }
        abfs[channelIdx] = abf;
    }
    DataWriterConsumer::manageConsumptionEnd();
}

unsigned int AbfDataWriterConsumer::blocksUsedBySection(ABF_Section section) {
    long long bytes = section.llNumEntries*(long long)section.uBytes;
    return (unsigned int)((bytes+ABF_BLOCKSIZE-1)/ABF_BLOCKSIZE); /*!< Ceil rounding */
}

unsigned int AbfDataWriterConsumer::blocksUsedBySection(long long sectionSize) {
    return (unsigned int)((sectionSize+ABF_BLOCKSIZE-1)/ABF_BLOCKSIZE); /*!< Ceil rounding */
}

//void AbfDataWriterConsumer::saveSynchInfo() {
//    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
//        if (activeChannelsFlag[channelIdx]) {
//            this->saveSynchInfo(abfs[channelIdx]);
//        }
//    }
//}

//void AbfDataWriterConsumer::saveSynchInfo(ABF * abf) {
//    abf->SynchArray[sweepIdx].lStart = episodeStart;
//    abf->SynchArray[sweepIdx].lLength = episodeLength;
//}

//void AbfDataWriterConsumer::writeSynchInfo(ABF * abf) {
//    int maxLength = 0;

//    for (int sweepIndex = 0; sweepIndex < sweepsNum; sweepIndex++) {
//        maxLength = (maxLength > abf->SynchArray[sweepIndex].lLength ? maxLength : abf->SynchArray[sweepIndex].lLength);
//    }

//    abf->FileInfo.uActualEpisodes = (unsigned int)sweepsNum;

//    abf->FileInfo.SynchArraySection.uBlockIndex = blockIdx;
//    abf->FileInfo.SynchArraySection.uBytes = sizeof(ABFLONG)*2;
//    abf->FileInfo.SynchArraySection.llNumEntries = (long long)sweepsNum;
//    blockIdx += this->blocksUsedBySection(abf->FileInfo.SynchArraySection);

//    abf->WriteSynchData();

//    abf->ProtocolInfo.lEpisodesPerRun = (int)sweepsNum;
//    abf->ProtocolInfo.lNumSamplesPerEpisode = maxLength;
//}

//void AbfDataWriterConsumer::writeTagsInfo(ABF * abf) {
//    if (tagIdx > 0) {
//        abf->FileInfo.TagSection.uBlockIndex = blockIdx;
//        abf->FileInfo.TagSection.llNumEntries = (long long)tagIdx;
//        blockIdx += this->blocksUsedBySection(abf->FileInfo.TagSection);

//        abf->WriteTagsInfo();
//    }
//}
