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
 *  \brief Header containing all structures of ABF
 */

#ifndef AXON_STRUCTS_H_
#define AXON_STRUCTS_H_

#pragma once
#pragma pack(push, 1)

#ifdef X86_64
    #define ABFLONG int
#else
    #define ABFLONG int
#endif

#include "axon_defs.h"

// GUID is normally defined in the Windows Platform SDK
typedef struct _MYGUID {
    unsigned ABFLONG Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} MYGUID;


// All these structs are persisted to file -> their sizes must NOT be changed without careful
// attention to versioning issues in order to maintain compatibility.

struct ABF_Section {
   unsigned int     uBlockIndex;            // ABF block number of the first entry
   unsigned int     uBytes;                 // size in bytes of of each entry
   long long llNumEntries;           // number of entries in this section
};

struct ABF_ScopeChannel {
    char sName[12];
    ABFLONG lIndex;
    unsigned int uUnknown003;// = 0x000100ff;
    short nDisplayType;
    float fPlotHeightFactor;
    float fUnknown004[2];// = {1.0, 0.0};
};

#define ABF_FILESIGNATURE   0x32464241      // PC="ABF2", MAC="2FBA"

struct ABF_FileInfo {
   unsigned int  uFileSignature;
   unsigned int  uFileVersionNumber;

   // After this point there is no need to be the same as the ABF 1 equivalent.
   unsigned int  uFileInfoSize;

   unsigned int  uActualEpisodes;
   unsigned int  uFileStartDate;
   unsigned int  uFileStartTimeMS;
   unsigned int  uStopwatchTime;
   short nFileType;
   short nDataFormat;
   short nSimultaneousScan;
   short nCRCEnable;
   unsigned int  uFileCRC;
   MYGUID  FileGUID;
   unsigned int  uCreatorVersion;
   unsigned int  uCreatorNameIndex;
   unsigned int  uModifierVersion;
   unsigned int  uModifierNameIndex;
   unsigned int  uProtocolPathIndex;

   // New sections in ABF 2 - protocol stuff ...
   ABF_Section ProtocolSection;           // the protocol92
   ABF_Section ADCSection;                // one for each ADC channel108
   ABF_Section DACSection;                // one for each DAC channel124
   ABF_Section EpochSection;              // one for each epoch140
   ABF_Section ADCPerDACSection;          // one for each ADC for each DAC156
   ABF_Section EpochPerDACSection;        // one for each epoch for each DAC172
   ABF_Section UserListSection;           // one for each user list188
   ABF_Section StatsRegionSection;        // one for each stats region204
   ABF_Section MathSection; //220
   ABF_Section StringsSection; //236

   // ABF 1 sections ...
   ABF_Section DataSection;            // Data
   ABF_Section TagSection;             // Tags
   ABF_Section ScopeSection;           // Scope config
   ABF_Section DeltaSection;           // Deltas
   ABF_Section VoiceTagSection;        // Voice Tags
   ABF_Section SynchArraySection;      // Synch Array
   ABF_Section AnnotationSection;      // Annotations
   ABF_Section StatsSection;           // Stats config

   char  sUnused[148];     // size = 512 bytes
};

struct ABF_ProtocolInfo {
   short nOperationMode;//2
   float fADCSequenceInterval;//6
   bool  bEnableFileCompression;//7
   char  sUnused1[3];//10
   unsigned int  uFileCompressionRatio;//14

   float fSynchTimeUnit;//18
   float fSecondsPerRun;//22
   ABFLONG  lNumSamplesPerEpisode;//26
   ABFLONG  lPreTriggerSamples;//30
   ABFLONG  lEpisodesPerRun;//34
   ABFLONG  lRunsPerTrial;//38
   ABFLONG  lNumberOfTrials;//42
   short nAveragingMode;//44
   short nUndoRunCount;//46
   short nFirstEpisodeInRun;//48
   float fTriggerThreshold;//52
   short nTriggerSource;//54
   short nTriggerAction;//56
   short nTriggerPolarity;//58
   float fScopeOutputInterval;//62
   float fEpisodeStartToStart;//66
   float fRunStartToStart;//70
   ABFLONG  lAverageCount;//74
   float fTrialStartToStart;//78
   short nAutoTriggerStrategy;//80
   float fFirstRunDelayS;//84

   short nChannelStatsStrategy;//86
   ABFLONG  lSamplesPerTrace;//90
   ABFLONG  lStartDisplayNum;//94
   ABFLONG  lFinishDisplayNum;//98
   short nShowPNRawData;//100
   float fStatisticsPeriod;//104
   ABFLONG  lStatisticsMeasurements;//108
   short nStatisticsSaveStrategy;//110

   float fADCRange;//114
   float fDACRange;//118
   ABFLONG  lADCResolution;//122
   ABFLONG  lDACResolution;//126

   short nExperimentType;//128
   short nManualInfoStrategy;//130
   short nCommentsEnable;//132
   ABFLONG  lFileCommentIndex;//136
   short nAutoAnalyseEnable;//138
   short nSignalType;//140

   short nDigitalEnable;//142
   short nActiveDACChannel;//144
   short nDigitalHolding;//146
   short nDigitalInterEpisode;//148
   short nDigitalDACChannel;//150
   short nDigitalTrainActiveLogic;//152

   short nStatsEnable;//154
   short nStatisticsClearStrategy;//156

   short nLevelHysteresis;//158
   ABFLONG  lTimeHysteresis;//162
   short nAllowExternalTags;//164
   short nAverageAlgorithm;//166
   float fAverageWeighting;//170
   short nUndoPromptStrategy;//172
   short nTrialTriggerSource;//174
   short nStatisticsDisplayStrategy;//176
   short nExternalTagType;//178
   short nScopeTriggerOut;//180

   short nLTPType;//182
   short nAlternateDACOutputState;//184
   short nAlternateDigitalOutputState;//186

   float fCellID[3];//198

   short nDigitizerADCs;//200
   short nDigitizerDACs;//202
   short nDigitizerTotalDigitalOuts;//204
   short nDigitizerSynchDigitalOuts;//206
   short nDigitizerType;//208

   char  sUnused[304];     // size = 512 bytes
};

struct ABF_MathInfo {
   short nMathEnable;
   short nMathExpression;
   unsigned int  uMathOperatorIndex;
   unsigned int  uMathUnitsIndex;
   float fMathUpperLimit;
   float fMathLowerLimit;
   short nMathADCNum[2];
   char  sUnused[16];
   float fMathK[6];

   char  sUnused2[64];     // size = 128 bytes
};

struct ABF_ADCInfo {
   // The ADC this struct is describing.
   short nADCNum;//2

   short nTelegraphEnable;//4
   short nTelegraphInstrument;//6
   float fTelegraphAdditGain;//10
   float fTelegraphFilter;//14
   float fTelegraphMembraneCap;//18
   short nTelegraphMode;//20
   float fTelegraphAccessResistance;//24

   short nADCPtoLChannelMap;//26
   short nADCSamplingSeq;//28

   float fADCProgrammableGain;//32
   float fADCDisplayAmplification;//36
   float fADCDisplayOffset;//40
   float fInstrumentScaleFactor;//44
   float fInstrumentOffset;//48
   float fSignalGain;//52
   float fSignalOffset;//56
   float fSignalLowpassFilter;//60
   float fSignalHighpassFilter;//64

   char  nLowpassFilterType;//65
   char  nHighpassFilterType;//66
   float fPostProcessLowpassFilter;//70
   char  nPostProcessLowpassFilterType;//71
   bool  bEnabledDuringPN;//72

   short nStatsChannelPolarity;//74

   ABFLONG  lADCChannelNameIndex;//78
   ABFLONG  lADCUnitsIndex;//82

   char  sUnused[46];         // size = 128 bytes
};

struct ABF_DACInfo {
   // The DAC this struct is describing.
   short nDACNum;//2

   short nTelegraphDACScaleFactorEnable;//4
   float fInstrumentHoldingLevel;//8

   float fDACScaleFactor;//12
   float fDACHoldingLevel;//16
   float fDACCalibrationFactor;//20
   float fDACCalibrationOffset;//24

   ABFLONG  lDACChannelNameIndex;//28
   ABFLONG  lDACChannelUnitsIndex;//32

   ABFLONG  lDACFilePtr;//36
   ABFLONG  lDACFileNumEpisodes;//40

   short nWaveformEnable;//42
   short nWaveformSource;//44
   short nInterEpisodeLevel;//46

   float fDACFileScale;//50
   float fDACFileOffset;//54
   ABFLONG  lDACFileEpisodeNum;//58
   short nDACFileADCNum;//60

   short nConditEnable;//62
   ABFLONG  lConditNumPulses;//66
   float fBaselineDuration;//70
   float fBaselineLevel;//74
   float fStepDuration;//78
   float fStepLevel;//82
   float fPostTrainPeriod;//86
   float fPostTrainLevel;//90
   short nMembTestEnable;//92

   short nLeakSubtractType;//94
   short nPNPolarity;//96
   float fPNHoldingLevel;//100
   short nPNNumADCChannels;//102
   short nPNPosition;//104
   short nPNNumPulses;//106
   float fPNSettlingTime;//110
   float fPNInterpulse;//114

   short nLTPUsageOfDAC;//116
   short nLTPPresynapticPulses;//118

   ABFLONG  lDACFilePathIndex;//122

   float fMembTestPreSettlingTimeMS;//126
   float fMembTestPostSettlingTimeMS;//130

   short nLeakSubtractADCIndex;//132

   char  sUnused[124];     // size = 256 bytes
};

struct ABF_EpochInfoPerDAC {
   // The Epoch / DAC this struct is describing.
   short nEpochNum;
   short nDACNum;

   // One full set of epochs (ABF_EPOCHCOUNT) for each DAC channel ...
   short nEpochType;
   float fEpochInitLevel;
   float fEpochLevelInc;
   ABFLONG  lEpochInitDuration;
   ABFLONG  lEpochDurationInc;
   ABFLONG  lEpochPulsePeriod;
   ABFLONG  lEpochPulseWidth;

   char  sUnused[18];      // size = 48 bytes
};

struct ABF_EpochInfo {
   // The Epoch this struct is describing.
   short nEpochNum;

   // Describes one epoch
   short nDigitalValue;
   short nDigitalTrainValue;
   short nAlternateDigitalValue;
   short nAlternateDigitalTrainValue;
   bool  bEpochCompression;   // Compress the data from this epoch using uFileCompressionRatio

   char  sUnused[21];      // size = 32 bytes
};

struct ABF_StatsRegionInfo {
   // The stats region this struct is describing.
   short nRegionNum;
   short nADCNum;

   short nStatsActiveChannels;
   short nStatsSearchRegionFlags;
   short nStatsSelectedRegion;
   short nStatsSmoothing;
   short nStatsSmoothingEnable;
   short nStatsBaseline;
   ABFLONG  lStatsBaselineStart;
   ABFLONG  lStatsBaselineEnd;

   // Describes one stats region
   ABFLONG  lStatsMeasurements;
   ABFLONG  lStatsStart;
   ABFLONG  lStatsEnd;
   short nRiseBottomPercentile;
   short nRiseTopPercentile;
   short nDecayBottomPercentile;
   short nDecayTopPercentile;
   short nStatsSearchMode;
   short nStatsSearchDAC;
   short nStatsBaselineDAC;

   char  sUnused[78];   // size = 128 bytes
};

struct ABF_ScopeInfo {
    // NOTE: Most of this is guessed/unknown
    ABFLONG lUnknownArray001[12];
    float fSamplingRateHz;
    short nUnknown002[10];
    char sFont[32];

    ABF_ScopeChannel channels[ABF_ADCCOUNT];

    short nUnknown005;
    short nADCNum;
    unsigned short nUnknown006[58];
    char sUnknown007;

    char  sUnused[255];   // size = 255 bytes
};

struct ABF_UserListInfo {
   // The user list this struct is describing.
   short nListNum;

   // Describes one user list
   short nULEnable;
   short nULParamToVary;
   short nULRepeat;
   ABFLONG  lULParamValueListIndex;

   char  sUnused[52];   // size = 64 bytes
};

struct ABF_SynchArray {
    ABFLONG lStart;
    ABFLONG lLength;
};

struct ABF_TagsInfo {
    ABFLONG lStart;
    char tag[ABF_TAGCOMMENTLEN] = {' '};
    ABFLONG lEnd = 1;
};

// Strings section structure not defined by Axon

#pragma pack(pop)                      // return to default packing

#endif   // AXON_STRUCTS_H_
