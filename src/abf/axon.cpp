//
// This file is part of the Axon Library.
//
// Copyright (c) 2008-2009 Jakub Nowacki
//
// The Axon Binary Format is property of Molecular Devices.
// All rights to the Axon Binary Format are reserved to Molecular Devices.
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301, USA
//

/*! \file
 *  \brief The main source file of Axon Library, contains ABF class
 */

#include "axon.h"

#include <stdio.h>

/*! ABF class constructor. */
ABF::ABF() {
    /*! Initializing the internal variables to allow the check if they are empty */
    name = "";
    int i;
    int j;
    for (i = 0; i < ABF_ADCCOUNT; i++) {
        ADCInfo[i].nADCNum = -1;
    }
    for (i = 0; i < ABF_DACCOUNT; i++) {
        DACInfo[i].nDACNum = -1;
        for (j = 0; j < ABF_EPOCHCOUNT; j++) {
            EpochInfoPerDAC[i][j].nDACNum = -1;
            EpochInfoPerDAC[i][j].nEpochNum = -1;
        }
    }
    for (i = 0; i < ABF_EPOCHCOUNT; i++) {
        EpochInfo[i].nEpochNum = -1;
    }

    /*! StringAddress saves room for the first string initialized by InitStrings() */
    StringAddress = 44;
    StringIndex = 0;
}

/*! ABF class destructor. */
ABF::~ABF(void) {
    if (handle.is_open()) {
        handle.close();
    }
}

/*! Opens a file. */
int ABF::Open(std::string fname, int mode) {
    /*! If a name have been specified, save it. */
    char data[8*16*16];
    for (int i = 0; i < 8*16*16; i++) {
        data[i] = 0;
    }
    if (fname.length() > 0) {
        name = fname;
    }

    /*! If no name is specified exit. */
    if (name.length() == 0) {
        return AXON_ERROR_NO_NAME;
    }

    /*! Open the file, in case of failure delete the name. */
    handle.open(name, mode);
    if (!handle.is_open()) {
        name = "";
        return AXON_ERROR_NO_HANDLE;
    }

    /*! File Info section */
    handle.seekp(0);
    handle.write(data, 8*16*16);

    return AXON_INFO_OK;
}

/*! Closes a file. */
int ABF::Close(void) {
    /*! If there's no handle (no file was opened) exit. */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }
    /*! Otherwise close file and reset the handle. */
    handle.close();
    return AXON_INFO_OK;
}

/*! Reads File Info from the ABF file */
int ABF::ReadFileInfo() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! File Info section */
    handle.seekp(0);
    handle.read((char *)&FileInfo, sizeof (FileInfo));
    return AXON_INFO_OK;
}

/*! Writes File Info from the ABF file */
int ABF::WriteFileInfo() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! File Info section */
    handle.seekp(0);
    handle.write((char *)&FileInfo, sizeof (FileInfo));
    return AXON_INFO_OK;
}

/*! Initialize file info */
int ABF::InitFileInfo() {
    char * arr = (char *)&FileInfo;
    for (int i = 0; i < 512; i++) {
        arr[i] = 0;
    }

    FileInfo.uFileSignature = ABF_FILESIGNATURE; /*!< ABF2 PC format 0x32464241 (0x32='2', 0x46='F', 0x42='B', 0x41='A') */
    FileInfo.uFileVersionNumber = 0x02060000;
    FileInfo.uFileInfoSize = 512;
    FileInfo.uActualEpisodes = 0;
    FileInfo.uFileStartDate = 000101; /*!< 01/01/2000 */
    FileInfo.uFileStartTimeMS = 0; /*!< 00:00:00.00 */
    FileInfo.uStopwatchTime = 0;
    FileInfo.nFileType = ABF_ABFFILE;
    FileInfo.nDataFormat = 0; /*!< 2 byte integer (?) */
    FileInfo.nSimultaneousScan = 1;
    FileInfo.nCRCEnable = 0;
    FileInfo.uFileCRC = 0;
    FileInfo.uCreatorVersion = 0; /*!< 0.0.0.0 */
    FileInfo.uCreatorNameIndex = 0;
    FileInfo.uModifierVersion = 0; /*!< 0.0.0.0 */
    FileInfo.uModifierNameIndex = 0;
    FileInfo.uProtocolPathIndex = 0;

    /*! sections */
    FileInfo.ProtocolSection.uBlockIndex = 0;
    FileInfo.ProtocolSection.uBytes = 0;
    FileInfo.ProtocolSection.llNumEntries = 0;
    FileInfo.ADCSection.uBlockIndex = 0;
    FileInfo.ADCSection.uBytes = 0;
    FileInfo.ADCSection.llNumEntries = 0;
    FileInfo.DACSection.uBlockIndex = 0;
    FileInfo.DACSection.uBytes = 0;
    FileInfo.DACSection.llNumEntries = 0;
    FileInfo.EpochSection.uBlockIndex = 0;
    FileInfo.EpochSection.uBytes = 0;
    FileInfo.EpochSection.llNumEntries = 0;
    FileInfo.ADCPerDACSection.uBlockIndex = 0;
    FileInfo.ADCPerDACSection.uBytes = 0;
    FileInfo.ADCPerDACSection.llNumEntries = 0;
    FileInfo.EpochPerDACSection.uBlockIndex = 0;
    FileInfo.EpochPerDACSection.uBytes = 0;
    FileInfo.EpochPerDACSection.llNumEntries = 0;
    FileInfo.UserListSection.uBlockIndex = 0;
    FileInfo.UserListSection.uBytes = 0;
    FileInfo.UserListSection.llNumEntries = 0;
    FileInfo.StatsRegionSection.uBlockIndex = 0;
    FileInfo.StatsRegionSection.uBytes = 0;
    FileInfo.StatsRegionSection.llNumEntries = 0;
    FileInfo.MathSection.uBlockIndex = 0;
    FileInfo.MathSection.uBytes = 0;
    FileInfo.MathSection.llNumEntries = 0;
    FileInfo.StringsSection.uBlockIndex = 0;
    FileInfo.StringsSection.uBytes = 0;
    FileInfo.StringsSection.llNumEntries = 0;
    FileInfo.DataSection.uBlockIndex = 0;
    FileInfo.DataSection.uBytes = sizeof(short);
    FileInfo.DataSection.llNumEntries = 0;
    FileInfo.TagSection.uBlockIndex = 0;
    FileInfo.TagSection.uBytes = 0;
    FileInfo.TagSection.llNumEntries = 0;
    FileInfo.ScopeSection.uBlockIndex = 0;
    FileInfo.ScopeSection.uBytes = 0;
    FileInfo.ScopeSection.llNumEntries = 0;
    FileInfo.DeltaSection.uBlockIndex = 0;
    FileInfo.DeltaSection.uBytes = 0;
    FileInfo.DeltaSection.llNumEntries = 0;
    FileInfo.VoiceTagSection.uBlockIndex = 0;
    FileInfo.VoiceTagSection.uBytes = 0;
    FileInfo.VoiceTagSection.llNumEntries = 0;
    FileInfo.SynchArraySection.uBlockIndex = 0;
    FileInfo.SynchArraySection.uBytes = 0;
    FileInfo.SynchArraySection.llNumEntries = 0;
    FileInfo.AnnotationSection.uBlockIndex = 0;
    FileInfo.AnnotationSection.uBytes = 0;
    FileInfo.AnnotationSection.llNumEntries = 0;
    FileInfo.StatsSection.uBlockIndex = 0;
    FileInfo.StatsSection.uBytes = 0;
    FileInfo.StatsSection.llNumEntries = 0;

    return AXON_INFO_OK;
}

/*! Reads Protocol Info from the ABF file */
int ABF::ReadProtocolInfo() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.ProtocolSection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    /*! Protocol Info section */
    handle.seekp(FileInfo.ProtocolSection.uBlockIndex*ABF_BLOCKSIZE);
    handle.read((char *)&ProtocolInfo, sizeof(ProtocolInfo));
    return AXON_INFO_OK;
}

/*! Writes Protocol Info to the ABF file */
int ABF::WriteProtocolInfo() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.ProtocolSection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    /*! Protocol Info section */
    handle.seekp(FileInfo.ProtocolSection.uBlockIndex*ABF_BLOCKSIZE);
    handle.write((char *)&ProtocolInfo, sizeof(ProtocolInfo));
    return AXON_INFO_OK;
}

/*! Init protocol Info */
int ABF::InitProtocolSection() {

    char * arr = (char *)&ProtocolInfo;
    for (int i = 0; i < (int)sizeof(ProtocolInfo); i++) {
        arr[i] = 0;
    }

    ProtocolInfo.nOperationMode = ABF_GAPFREEFILE;
    ProtocolInfo.fADCSequenceInterval = 0;
    ProtocolInfo.bEnableFileCompression = false;
    ProtocolInfo.uFileCompressionRatio = 1;
    ProtocolInfo.fSynchTimeUnit = 0; /*! \todo FCON c'era un uno qui */
    ProtocolInfo.fSecondsPerRun = 0;
    ProtocolInfo.lNumSamplesPerEpisode = 512;
    ProtocolInfo.lPreTriggerSamples = 16;
    ProtocolInfo.lEpisodesPerRun = 1;
    ProtocolInfo.lRunsPerTrial = 1;
    ProtocolInfo.lNumberOfTrials = 1;
    ProtocolInfo.nAveragingMode = 0;
    ProtocolInfo.nUndoRunCount = 0;
    ProtocolInfo.nFirstEpisodeInRun = 0;
    ProtocolInfo.fTriggerThreshold = 0;
    ProtocolInfo.nTriggerSource = 0;
    ProtocolInfo.nTriggerAction = 0;
    ProtocolInfo.nTriggerPolarity = 0;
    ProtocolInfo.fScopeOutputInterval = 0;
    ProtocolInfo.fEpisodeStartToStart = 0;
    ProtocolInfo.fRunStartToStart = 0;
    ProtocolInfo.lAverageCount = 0;
    ProtocolInfo.fTrialStartToStart = 0;
    ProtocolInfo.nAutoTriggerStrategy = 1;
    ProtocolInfo.fFirstRunDelayS = 0;
    ProtocolInfo.nChannelStatsStrategy = 0;
    ProtocolInfo.lSamplesPerTrace = 16384;
    ProtocolInfo.lStartDisplayNum = 0;
    ProtocolInfo.lFinishDisplayNum = 0;
    ProtocolInfo.nShowPNRawData = 0;
    ProtocolInfo.fStatisticsPeriod = 1;
    ProtocolInfo.lStatisticsMeasurements = 5;
    ProtocolInfo.nStatisticsSaveStrategy = 0;
    ProtocolInfo.fADCRange = 1;
    ProtocolInfo.fDACRange = 1;
    ProtocolInfo.lADCResolution = 32768;
    ProtocolInfo.lDACResolution = 32768;
    ProtocolInfo.nExperimentType = 2;
    ProtocolInfo.nManualInfoStrategy = 0;
    ProtocolInfo.nCommentsEnable = 0;
    ProtocolInfo.lFileCommentIndex = 0;
    ProtocolInfo.nAutoAnalyseEnable = 1;
    ProtocolInfo.nSignalType = 0;
    ProtocolInfo.nDigitalEnable = 0;
    ProtocolInfo.nActiveDACChannel = 0;
    ProtocolInfo.nDigitalHolding = 0;
    ProtocolInfo.nDigitalInterEpisode = 0;
    ProtocolInfo.nDigitalDACChannel = 0;
    ProtocolInfo.nDigitalTrainActiveLogic = 1;
    ProtocolInfo.nStatsEnable = 0;
    ProtocolInfo.nStatisticsClearStrategy = 0;
    ProtocolInfo.nLevelHysteresis = 64;
    ProtocolInfo.lTimeHysteresis = 1;
    ProtocolInfo.nAllowExternalTags = 0;
    ProtocolInfo.nAverageAlgorithm = 0;
    ProtocolInfo.fAverageWeighting = 0.1F;
    ProtocolInfo.nUndoPromptStrategy = 0;
    ProtocolInfo.nTrialTriggerSource = -1;
    ProtocolInfo.nStatisticsDisplayStrategy = 0;
    ProtocolInfo.nExternalTagType = 2;
    ProtocolInfo.nScopeTriggerOut = 0;
    ProtocolInfo.nLTPType = 0;
    ProtocolInfo.nAlternateDACOutputState = 0;
    ProtocolInfo.nAlternateDigitalOutputState = 0;
    ProtocolInfo.fCellID[0] = 0;
    ProtocolInfo.fCellID[1] = 0;
    ProtocolInfo.fCellID[2] = 0;
    ProtocolInfo.nDigitizerADCs = 16;
    ProtocolInfo.nDigitizerDACs = 4;
    ProtocolInfo.nDigitizerTotalDigitalOuts = 16;
    ProtocolInfo.nDigitizerSynchDigitalOuts = 8;
    ProtocolInfo.nDigitizerType = 1;

    return AXON_INFO_OK;
}

/*! Reads ADC Info from the ABF file */
int ABF::ReadADCInfo() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.ADCSection.uBlockIndex == 0)
        return AXON_INFO_NO_SECTION;

    /*! ADC Info section */
    handle.seekp(FileInfo.ADCSection.uBlockIndex*ABF_BLOCKSIZE);
    handle.read((char *)ADCInfo, FileInfo.ADCSection.uBytes*FileInfo.ADCSection.llNumEntries);

    return AXON_INFO_OK;
}

/*! Writes ADC Info to the ABF file */
int ABF::WriteADCInfo() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.ADCSection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    /*! ADC Info section */
    handle.seekp(FileInfo.ADCSection.uBlockIndex*ABF_BLOCKSIZE);
    handle.write((char *)ADCInfo, FileInfo.ADCSection.uBytes*FileInfo.ADCSection.llNumEntries);

    return AXON_INFO_OK;
}

int ABF::InitADCSection(int index) {

    char * arr = (char *)&(ADCInfo[index]);
    for (int i = 0; i < (int)sizeof(ADCInfo[index]); i++) {
        arr[i] = 0;
    }

    ADCInfo[index].nADCNum = 0;
    ADCInfo[index].nTelegraphEnable = 0;
    ADCInfo[index].nTelegraphInstrument = 0;
    ADCInfo[index].fTelegraphAdditGain = 1;
    ADCInfo[index].fTelegraphFilter = 100000;
    ADCInfo[index].fTelegraphMembraneCap = 0;
    ADCInfo[index].nTelegraphMode = 0;
    ADCInfo[index].fTelegraphAccessResistance = 0;
    ADCInfo[index].nADCPtoLChannelMap = 0;
    ADCInfo[index].nADCSamplingSeq = 0;
    ADCInfo[index].fADCProgrammableGain = 1;
    ADCInfo[index].fADCDisplayAmplification = 1;
    ADCInfo[index].fADCDisplayOffset = 0;
    ADCInfo[index].fInstrumentScaleFactor = 1.0F;
    ADCInfo[index].fInstrumentOffset = 0;
    ADCInfo[index].fSignalGain = 1;
    ADCInfo[index].fSignalOffset = 0;
    ADCInfo[index].fSignalLowpassFilter = 100000;
    ADCInfo[index].fSignalHighpassFilter = 0;
    ADCInfo[index].nLowpassFilterType = 0;
    ADCInfo[index].nHighpassFilterType = 0;
    ADCInfo[index].fPostProcessLowpassFilter = 100000;
    ADCInfo[index].nPostProcessLowpassFilterType = 0;
    ADCInfo[index].bEnabledDuringPN = false;
    ADCInfo[index].nStatsChannelPolarity = 0;
    ADCInfo[index].lADCChannelNameIndex = 0;
    ADCInfo[index].lADCUnitsIndex = 0;

    return AXON_INFO_OK;
}

/*! Reads DAC Info from the ABF file */
int ABF::ReadDACInfo() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.DACSection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    /*! DAC Info section */
    handle.seekp(FileInfo.DACSection.uBlockIndex*ABF_BLOCKSIZE);
    handle.read((char *)DACInfo, FileInfo.DACSection.uBytes*FileInfo.DACSection.llNumEntries);

    return AXON_INFO_OK;
}

/*! Writes DAC Info to the ABF file */
int ABF::WriteDACInfo() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.DACSection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    /*! DAC Info section */
    handle.seekp(FileInfo.DACSection.uBlockIndex*ABF_BLOCKSIZE);
    handle.write((char *)DACInfo, FileInfo.DACSection.uBytes*FileInfo.DACSection.llNumEntries);

    return AXON_INFO_OK;
}

int ABF::InitDACSection(int index) {

    char * arr = (char *)&(DACInfo[index]);
    for (int i = 0; i < (int)sizeof(DACInfo[index]); i++) {
        arr[i] = 0;
    }

    DACInfo[index].nDACNum = 0;
    DACInfo[index].nTelegraphDACScaleFactorEnable = 0;
    DACInfo[index].fInstrumentHoldingLevel = 0;
    DACInfo[index].fDACScaleFactor = 20;
    DACInfo[index].fDACHoldingLevel = 0;
    DACInfo[index].fDACCalibrationFactor = 1;
    DACInfo[index].fDACCalibrationOffset = 0;
    DACInfo[index].lDACChannelNameIndex = 0;
    DACInfo[index].lDACChannelUnitsIndex = 0;
    DACInfo[index].lDACFilePtr = 0;
    DACInfo[index].lDACFileNumEpisodes = 0;
    DACInfo[index].nWaveformEnable = 0;
    DACInfo[index].nWaveformSource = 0;
    DACInfo[index].nInterEpisodeLevel = 0;
    DACInfo[index].fDACFileScale = 1;
    DACInfo[index].fDACFileOffset = 0;
    DACInfo[index].lDACFileEpisodeNum = 0;
    DACInfo[index].nDACFileADCNum = 0;
    DACInfo[index].nConditEnable = 0;
    DACInfo[index].lConditNumPulses = 0;
    DACInfo[index].fBaselineDuration = 0;
    DACInfo[index].fBaselineLevel = 0;
    DACInfo[index].fStepDuration = 0;
    DACInfo[index].fStepLevel = 0;
    DACInfo[index].fPostTrainPeriod = 10;
    DACInfo[index].fPostTrainLevel = 0;
    DACInfo[index].nMembTestEnable = 0;
    DACInfo[index].nLeakSubtractType = 0;
    DACInfo[index].nPNPolarity = ABF_PN_SAME_POLARITY;
    DACInfo[index].fPNHoldingLevel = 0;
    DACInfo[index].nPNNumADCChannels = 0;
    DACInfo[index].nPNPosition = ABF_PN_BEFORE_EPISODE;
    DACInfo[index].nPNNumPulses = 4;
    DACInfo[index].fPNSettlingTime = 100;
    DACInfo[index].fPNInterpulse = 0;
    DACInfo[index].nLTPUsageOfDAC = 0;
    DACInfo[index].nLTPPresynapticPulses = 0;
    DACInfo[index].lDACFilePathIndex = 0;
    DACInfo[index].fMembTestPreSettlingTimeMS = 100;
    DACInfo[index].fMembTestPostSettlingTimeMS = 100;
    DACInfo[index].nLeakSubtractADCIndex = 0;

    return AXON_INFO_OK;
}

/*! Reads Epoch Info from the ABF file */
int ABF::ReadEpochInfo() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.EpochSection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    /*! Epoch Info section */
    handle.seekp(FileInfo.EpochSection.uBlockIndex*ABF_BLOCKSIZE);
    handle.read((char *)EpochInfo, FileInfo.EpochSection.uBytes*FileInfo.EpochSection.llNumEntries);

    return AXON_INFO_OK;
}

/*! Reads Stats Region Info to the ABF file */
int ABF::ReadStatsRegion() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.StatsRegionSection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    /*! StatsRegion Info section */
    handle.seekp(FileInfo.StatsRegionSection.uBlockIndex*ABF_BLOCKSIZE);
    handle.read((char *)&StatsRegionSection, FileInfo.StatsRegionSection.uBytes*FileInfo.StatsRegionSection.llNumEntries);

    return AXON_INFO_OK;
}

/*! Writes Stats Region Info to the ABF file */
int ABF::WriteStatsRegion() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.StatsRegionSection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    /*! StatsRegion Info section */
    handle.seekp(FileInfo.StatsRegionSection.uBlockIndex*ABF_BLOCKSIZE);
    handle.write((char *)&StatsRegionSection, FileInfo.StatsRegionSection.uBytes*FileInfo.StatsRegionSection.llNumEntries);

    return AXON_INFO_OK;
}

int ABF::InitStatsRegionSection() {

    char * arr = (char *)&(StatsRegionSection);
    for (int i = 0; i < (int)sizeof(StatsRegionSection); i++) {
        arr[i] = 0;
    }

    StatsRegionSection.nRegionNum = 0;
    StatsRegionSection.nADCNum = 0;
    StatsRegionSection.nStatsActiveChannels = 1;
    StatsRegionSection.nStatsSearchRegionFlags = 1;
    StatsRegionSection.nStatsSelectedRegion = 0;
    StatsRegionSection.nStatsSmoothing = 1;
    StatsRegionSection.nStatsSmoothingEnable = 0;
    StatsRegionSection.nStatsBaseline = -3;
    StatsRegionSection.lStatsBaselineStart = 0;
    StatsRegionSection.lStatsBaselineEnd = 0;
    StatsRegionSection.lStatsMeasurements = 3;
    StatsRegionSection.lStatsStart = 0;
    StatsRegionSection.lStatsEnd = 0;
    StatsRegionSection.nRiseBottomPercentile = 10;
    StatsRegionSection.nRiseTopPercentile = 90;
    StatsRegionSection.nDecayBottomPercentile = 10;
    StatsRegionSection.nDecayTopPercentile = 90;
    StatsRegionSection.nStatsSearchMode = -2;
    StatsRegionSection.nStatsSearchDAC = -1;
    StatsRegionSection.nStatsBaselineDAC = -1;

    return AXON_INFO_OK;
}

/*! Writes Scope Info to the ABF file */
int ABF::WriteScopeInfo() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.ScopeSection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    /*! StatsRegion Info section */
    handle.seekp(FileInfo.ScopeSection.uBlockIndex*ABF_BLOCKSIZE);
    handle.write((char *)&ScopeInfo, FileInfo.ScopeSection.uBytes*FileInfo.ScopeSection.llNumEntries);

    return AXON_INFO_OK;
}

int ABF::InitScopeSection() {

    char * arr = (char *)&(ScopeInfo);
    for (int i = 0; i < (int)sizeof(ScopeInfo); i++) {
        arr[i] = 0;
    }

    ScopeInfo.lUnknownArray001[0] = 0x00000008;
    ScopeInfo.lUnknownArray001[1] = 0x00ffffff;
    ScopeInfo.lUnknownArray001[2] = 0x00c0c0c0;
    ScopeInfo.lUnknownArray001[3] = 0x00000008;
    ScopeInfo.lUnknownArray001[4] = 0x000000ff;
    ScopeInfo.lUnknownArray001[5] = 0x00000000;
    ScopeInfo.lUnknownArray001[6] = 0x000000ff;
    ScopeInfo.lUnknownArray001[7] = 0x00c0c0c0;
    ScopeInfo.lUnknownArray001[8] = 0x00000000;
    ScopeInfo.lUnknownArray001[9] = 0x00f0f0f0;
    ScopeInfo.lUnknownArray001[10] = 0x00a0a0a0;
    ScopeInfo.lUnknownArray001[11] = 0x00000000;

    ScopeInfo.nUnknown002[0] = 0x0001;
    ScopeInfo.nUnknown002[1] = 0x0000;
    ScopeInfo.nUnknown002[2] = 0x0000;
    ScopeInfo.nUnknown002[3] = 0x0000;
    ScopeInfo.nUnknown002[4] = 0x0000;
    ScopeInfo.nUnknown002[5] = 0x0000;
    ScopeInfo.nUnknown002[6] = -11;
    ScopeInfo.nUnknown002[7] = 400;
    ScopeInfo.nUnknown002[8] = 32;
    ScopeInfo.nUnknown002[9] = 0x0000;

    sprintf(ScopeInfo.sFont, "Arial");

    ScopeInfo.nUnknown005 = 70;

    ScopeInfo.nUnknown006[0] = 0x0001;
    ScopeInfo.nUnknown006[1] = 0x0000;
    ScopeInfo.nUnknown006[2] = 0x0000;
    ScopeInfo.nUnknown006[3] = 0x0000;
    ScopeInfo.nUnknown006[4] = 0x0000;
    ScopeInfo.nUnknown006[5] = 0x0080;
    ScopeInfo.nUnknown006[6] = 0x0000;
    ScopeInfo.nUnknown006[7] = 0x8000;
    ScopeInfo.nUnknown006[8] = 0x0000;
    ScopeInfo.nUnknown006[9] = 0x8080;
    ScopeInfo.nUnknown006[10] = 0x0000;
    ScopeInfo.nUnknown006[11] = 0x0000;
    ScopeInfo.nUnknown006[12] = 0x0080;
    ScopeInfo.nUnknown006[13] = 0x0080;
    ScopeInfo.nUnknown006[14] = 0x0080;
    ScopeInfo.nUnknown006[15] = 0x8000;
    ScopeInfo.nUnknown006[16] = 0x0080;
    ScopeInfo.nUnknown006[17] = 0x8080;
    ScopeInfo.nUnknown006[18] = 0x0080;
    ScopeInfo.nUnknown006[19] = 0x00ff;
    ScopeInfo.nUnknown006[20] = 0x0000;
    ScopeInfo.nUnknown006[21] = 0x8000;
    ScopeInfo.nUnknown006[22] = 0x0080;

    ScopeInfo.sUnknown007 = 0;

    return AXON_INFO_OK;
}

/*! Reads Epoch Info per DAC from the ABF file */
int ABF::ReadEpochInfoPerDAC() {
    ABF_EpochInfoPerDAC tmp;

    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.EpochPerDACSection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    /*! Epoch per DAC Info section */
    handle.seekp(FileInfo.EpochPerDACSection.uBlockIndex*ABF_BLOCKSIZE);
    for (long long i = 0; i < FileInfo.EpochPerDACSection.llNumEntries; i++) {
        handle.read((char *)&tmp, FileInfo.EpochPerDACSection.uBytes);
        EpochInfoPerDAC[tmp.nDACNum][tmp.nEpochNum] = tmp;
    }

    return AXON_INFO_OK;
}

/*! Reads string section from the ABF file */
int ABF::ReadStrings() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.StringsSection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    /*! Strings section */
    handle.seekp(FileInfo.StringsSection.uBlockIndex*ABF_BLOCKSIZE);
    handle.read(Strings, ABF_BLOCKSIZE);

    return AXON_INFO_OK;
}

/*! Writes Strings from to ABF file */
int ABF::WriteStrings() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.StringsSection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    /*! Strings section */
    Strings[8] = (char)StringIndex;
    Strings[12] = (char)stringsMaxLen;
    Strings[16] = (char)StringAddress-44;
    handle.seekp(FileInfo.StringsSection.uBlockIndex*ABF_BLOCKSIZE);
    handle.write(Strings, ABF_BLOCKSIZE);

    return AXON_INFO_OK;
}

/*! Gets string with index i from Strings from the ABF file */
int ABF::GetString(char * str, int index) {
    char tmp[ABF_BLOCKSIZE];
    char * ptmp;
    int i = 1;
    int n;

    /*!  The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.StringsSection.llNumEntries < index) {
        return AXON_ERROR_NO_STRING;
    }

    /*! Copy useful Strings part */
    ptmp = Strings+44;
    while (index - i >= 0) {
        /*! If the index point to this string, read it to given variable */
        if (index == i) {
            strcpy(str, ptmp);
            n = (int)(strlen(str));

        } else {
            strcpy(tmp, ptmp);
            n = (int)(strlen(tmp));
        }
        ptmp += n+1;
        i++;
    }

    return AXON_INFO_OK;
}

/*! The same function as before, just returns string (more useful for wrapping) */
char * ABF::GetString(int i) {
    char * str = new char[255];

    this->GetString(str, i);
    return str;
}

int ABF::InitStrings(void) {
    for (int i = 0; i < 512; i++) {
        Strings[i] = 0;
    }

    Strings[0] = 0x53;
    Strings[1] = 0x53;
    Strings[2] = 0x43;
    Strings[3] = 0x48;
    Strings[4] = 1;
    //    Strings[8] = 12; number of strings
    //    Strings[12] = 10; length of longest string, last null char excluded
    //    Strings[16] = 51; total length of strings, including last null char
    return AXON_INFO_OK;
}

/*! Reads Synch Array from the ABF file */
int ABF::ReadSynchArray() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.SynchArraySection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    /*! Synch Array section */
    handle.seekp(FileInfo.SynchArraySection.uBlockIndex*ABF_BLOCKSIZE);
    handle.read((char *)SynchArray, FileInfo.SynchArraySection.uBytes*FileInfo.SynchArraySection.llNumEntries);

    return AXON_INFO_OK;
}

/*! Reads the content of the ABF file */
int ABF::ReadAllSections() {
    int ret;

    ret = this->ReadFileInfo();
    if (ret < 0) {
        return ret;
    }

    ret = this->ReadProtocolInfo();
    if (ret < 0) {
        return ret;
    }

    ret = this->ReadADCInfo();
    if (ret < 0) {
        return ret;
    }

    ret = this->ReadDACInfo();
    if (ret < 0) {
        return ret;
    }

    ret = this->ReadEpochInfo();
    if (ret < 0) {
        return ret;
    }

    ret = this->ReadEpochInfoPerDAC();
    if (ret < 0) {
        return ret;
    }

    ret = this->ReadStrings();
    if (ret < 0) {
        return ret;
    }

    return ret;
}

/*! Reads Data from the ABF file */
int ABF::ReadData(void * data) {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.DataSection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    /*! Data section */
    handle.seekp(FileInfo.DataSection.uBlockIndex*ABF_BLOCKSIZE);
    handle.read((char *)data, FileInfo.DataSection.uBytes*FileInfo.DataSection.llNumEntries);

    return AXON_INFO_OK;
}

/*! Reads float data from the ABF file */
float * ABF::ReadFloatData(void) {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return nullptr;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.DataSection.uBlockIndex == 0) {
        return nullptr;
    }

    float * ret = new float[FileInfo.DataSection.llNumEntries];

    if (this->ReadData(ret) != AXON_INFO_OK) {
        return nullptr;
    }
    return ret;
}

/*! Method reads integer data from the ABF file */
short * ABF::ReadIntData(void) {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return nullptr;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.DataSection.uBlockIndex == 0) {
        return nullptr;
    }

    short * ret = new short[FileInfo.DataSection.llNumEntries];

    if (this->ReadData(ret) != AXON_INFO_OK) {
        return nullptr;
    }
    return ret;
}

int ABF::PutString(const char * text) {
    int len = 0;
    while ((text[len] != 0) && (len < 512-StringAddress)) {
        len++;
    }
    if (len > stringsMaxLen) {
        stringsMaxLen = len;
    }
    memcpy(&Strings[StringAddress], text, len+1);
    StringAddress = StringAddress+len+1;
    StringIndex++;

    return AXON_INFO_OK;
}

int ABF::GetStringNumber(void) {
    return StringIndex;
}

int ABF::GetStringsTotLen(void) {
    return StringAddress;
}

/*! Method append Data to the ABF file */
int ABF::WriteRawData(void * data, int size, int num) {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.DataSection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    /*! Data section */
    long offset = (long)(FileInfo.DataSection.uBlockIndex*ABF_BLOCKSIZE)+
                  ((long)(FileInfo.DataSection.llNumEntries))*(long)size;
    handle.seekp(offset);
    handle.write((char *)data, size*num);
    FileInfo.DataSection.llNumEntries += num;

    return AXON_INFO_OK;
}

/*! Write synch Data to the ABF file */
int ABF::WriteSynchData() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.SynchArraySection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    int size = sizeof(ABFLONG);
    int num = FileInfo.SynchArraySection.llNumEntries*2;

    /*! Synch section */
    handle.seekp(FileInfo.SynchArraySection.uBlockIndex*ABF_BLOCKSIZE);
    handle.write((char *)SynchArray, size*num);

    return AXON_INFO_OK;
}

/*! Writes Tags Info to the ABF file */
int ABF::WriteTagsInfo() {
    /*! The file have to be opened first - use Open() */
    if (!handle.is_open()) {
        return AXON_ERROR_NO_HANDLE;
    }

    /*! Check if the section is present in the file */
    if (FileInfo.TagSection.uBlockIndex == 0) {
        return AXON_INFO_NO_SECTION;
    }

    /*! Tags Info section */
    handle.seekp(FileInfo.TagSection.uBlockIndex*ABF_BLOCKSIZE);
    handle.write((char *)TagsInfo, sizeof(ABF_TagsInfo)*FileInfo.TagSection.llNumEntries);
    return AXON_INFO_OK;
}
