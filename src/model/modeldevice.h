#ifndef MODELDEVICE_H
#define MODELDEVICE_H

#include <QString>
#include <QVector>
#include <vector>

#include "messagedispatcher.h"
#include "e384commlib_global.h"
#include "e384commlib_global_addendum.h"
#include "modelboard.h"

using namespace std;

class ModelDevice {
public:
    ModelDevice();
    ~ModelDevice();

    MessageDispatcher * getMessageDispatcher();
    QString getSerialNumber();
    bool isConnected();
    vector<ModelBoard*> getBoards();
    vector<ModelChannel*> getChannels();
    int getSamplingRateIdx();
    int getVcCurrentRangeIdx();
    int getVcVoltageRangeIdx();
    int getCcCurrentRangeIdx();
    int getCcVoltageRangeIdx();
    Measurement_t getSamplingRate();
    RangedMeasurement_t getVcCurrentRange();
    RangedMeasurement_t getVcVoltageRange();
    RangedMeasurement_t getCcCurrentRange();
    RangedMeasurement_t getCcVoltageRange();
    int getVcCurrentFilterIdx();
    int getVcVoltageFilterIdx();
    int getCcCurrentFilterIdx();
    int getCcVoltageFilterIdx();
    Measurement_t getVcCurrentFilter();
    Measurement_t getVcVoltageFilter();
    Measurement_t getCcCurrentFilter();
    Measurement_t getCcVoltageFilter();
    QVector <bool> getSelectedChannelsIdxs();
    int getOngoingClampingModality();
    int getOngoingClampingModalityIdx();

    void setMessageDispatcher(MessageDispatcher * messageDispatcher);
    void setSerialNumber(QString serial);
    void setConnected(bool flag);
    void setBoards(vector<ModelBoard*> boards);
    void setChannels(vector<ModelChannel*> channels);
    void setSamplingRate(int idx);
    void setVcCurrentRange(int idx);
    void setVcVoltageRange(int idx);
    void setCcCurrentRange(int idx);
    void setCcVoltageRange(int idx);
    void setSamplingRate(Measurement_t samplingRate);
    void setVcCurrentRange(RangedMeasurement_t vCcurrentRange);
    void setVcVoltageRange(RangedMeasurement_t vCvoltageRange);
    void setCcCurrentRange(RangedMeasurement_t cCcurrentRange);
    void setCcVoltageRange(RangedMeasurement_t cCvoltageRange);
    void setVcCurrentFilter(int idx);
    void setVcVoltageFilter(int idx);
    void setCcCurrentFilter(int idx);
    void setCcVoltageFilter(int idx);
    void setVcCurrentFilter(Measurement_t vCcurrentFilter);
    void setVcVoltageFilter(Measurement_t vCvoltageFilter);
    void setCcCurrentFilter(Measurement_t cCcurrentFilter);
    void setCcVoltageFilter(Measurement_t cCvoltageFilter);
    void setOngoingClampingModality(int mode);
    void setOngoingClampingModalityIdx(int idx);

    void fillBoardList(uint16_t numOfBoards, uint16_t numOfChannelsOnBoard);
    void fillChannelList(uint16_t numOfBoards, uint16_t numOfChannelsOnBoard);

    void flushBoardList();

    // wrappers for MessageDispatcher get features
    ErrorCodes_t getVoltageHoldTunerFeatures(std::vector <RangedMeasurement_t> &voltageHoldTunerFeatures);
    ErrorCodes_t getCurrentHoldTunerFeatures(std::vector <RangedMeasurement_t> &currentHoldTunerFeatures);
    ErrorCodes_t getCalibVcCurrentGainFeatures(RangedMeasurement_t &calibVcCurrentGainFeatures);
    ErrorCodes_t getCalibVcCurrentOffsetFeatures(vector <RangedMeasurement_t> &calibVcCurrentOffsetFeatures);
    ErrorCodes_t getCalibCcVoltageGainFeatures(RangedMeasurement_t &calibCcVoltageGainFeatures);
    ErrorCodes_t getCalibCcVoltageOffsetFeatures(vector <RangedMeasurement_t> &calibCcVoltageOffsetFeatures);
    ErrorCodes_t getGateVoltagesTunerFeatures(RangedMeasurement_t &gateVoltagesTunerFeatures);
    ErrorCodes_t getSourceVoltagesTunerFeatures(RangedMeasurement_t &sourceVoltagesTunerFeatures);
    ErrorCodes_t getChannelsNumberFeatures(int &voltageChannelNum, int &currentChannelNum);
    ErrorCodes_t getBoardsNumberFeatures(int &boardNum);
    ErrorCodes_t getClampingModalitiesFeatures(vector<int> &clampingModalitiesFeatures);

    ErrorCodes_t getVcCurrentRangesFeatures(vector <RangedMeasurement_t> &vcCurrentRangesFeatures, uint16_t &defaultVcCurrRangeIdx);
    ErrorCodes_t getVcVoltageRangesFeatures(vector <RangedMeasurement_t> &vcVoltageRangesFeatures);
    ErrorCodes_t getCcCurrentRangesFeatures(vector <RangedMeasurement_t> &ccCurrentRangesFeatures);
    ErrorCodes_t getCcVoltageRangesFeatures(vector <RangedMeasurement_t> &ccVoltageRangesFeatures);

    ErrorCodes_t getSamplingRatesFeatures(vector <Measurement_t> &samplingRatesFeatures);

    ErrorCodes_t getVoltageStimulusLpfsFeatures(vector <Measurement_t> &filterOptions);
    ErrorCodes_t getCurrentStimulusLpfsFeatures(vector <Measurement_t> &filterOptions);

//    ErrorCodes_t getCalibVcVoltStepFeatures(vector <Measurement_t> &calibVcVoltStepsFeatures);
//    ErrorCodes_t getCalibVcResFeatures(vector <Measurement_t> &calibVcResFeatures);
    ErrorCodes_t getCalibDataFeatures(CalibrationData_t &calibData);

    ErrorCodes_t getCompFeatures(uint16_t paramToExtractFeatures, std::vector <RangedMeasurement_t> &compensationFeatures, double &defaultParamValue);
    ErrorCodes_t getCompOptionsFeatures(MessageDispatcher::CompensationTypes type ,std::vector <std::string> &compOptionsArray);
    ErrorCodes_t enableCompensation(std::vector<uint16_t> channelIndexes, uint16_t compTypeToEnable, std::vector<bool> onValues);

    //---------------------------------------------/

private:
    MessageDispatcher * messageDispatcher = nullptr;
    QString serialNumber = "";

    bool connected = false;
    vector<ModelBoard*> myBoards;
    vector<ModelChannel*> myChannels;
    int samplingRateIdx = 0;
    int vcCurrentRangeIdx = 0;
    int vcVoltageRangeIdx = 0;
    int ccCurrentRangeIdx = 0;
    int ccVoltageRangeIdx = 0;
    Measurement_t samplingRate = {0.0, UnitPfxNone, "Hz"};
    RangedMeasurement_t vcCurrentRange = {0.0, 0.0, 0.0, UnitPfxMilli, "V"};
    RangedMeasurement_t vcVoltageRange = {0.0, 0.0, 0.0, UnitPfxMilli, "V"};
    RangedMeasurement_t ccCurrentRange = {0.0, 0.0, 0.0, UnitPfxMilli, "V"};
    RangedMeasurement_t ccVoltageRange = {0.0, 0.0, 0.0, UnitPfxMilli, "V"};
    int vcCurrentFilterIdx = 0;
    int vcVoltageFilterIdx = 0;
    int ccCurrentFilterIdx = 0;
    int ccVoltageFilterIdx = 0;
    Measurement_t vcCurrentFilter = {0.0, UnitPfxNone, "Hz"};
    Measurement_t vcVoltageFilter = {0.0, UnitPfxNone, "Hz"};
    Measurement_t ccCurrentFilter = {0.0, UnitPfxNone, "Hz"};
    Measurement_t ccVoltageFilter = {0.0, UnitPfxNone, "Hz"};
    int ongoingClampingModality = E384CL_VOLTAGE_CLAMP_MODE;
    int ongoingClampingModalityIdx = 0;
};

#endif // MODELDEVICE_H
