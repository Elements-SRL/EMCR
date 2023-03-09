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

namespace e384cl = e384CommLib;

class ModelDevice {
public:
    ModelDevice();
    ~ModelDevice();

    MessageDispatcher * getMessageDispatcher();
    QString getSerialNumber();
    bool isConnected();
    vector<ModelBoard*> getBoards();
    vector<ModelChannel*> getChannels();
    Measurement_t getSamplingRate();
    RangedMeasurement_t getVcCurrentRange();
    RangedMeasurement_t getVcVoltageRange();
    RangedMeasurement_t getCcCurrentRange();
    RangedMeasurement_t getCcVoltageRange();
    Measurement_t getVcCurrentFilter();
    Measurement_t getVcVoltageFilter();
    Measurement_t getCcCurrentFilter();
    Measurement_t getCcVoltageFilter();
    QVector <bool> getSelectedChannelsIdxs();

    void setMessageDispatcher(MessageDispatcher * messageDispatcher);
    void setSerialNumber(QString serial);
    void setConnected(bool flag);
    void setBoards(vector<ModelBoard*> boards);
    void setChannels(vector<ModelChannel*> channels);
    void setSamplingRate(Measurement_t samplingRate);
    void setVcCurrentRange(RangedMeasurement_t vCcurrentRange);
    void setVcVoltageRange(RangedMeasurement_t vCvoltageRange);
    void setCcCurrentRange(RangedMeasurement_t cCcurrentRange);
    void setCcVoltageRange(RangedMeasurement_t cCvoltageRange);
    void setVcCurrentFilter(Measurement_t vCcurrentFilter);
    void setVcVoltageFilter(Measurement_t vCvoltageFilter);
    void setCcCurrentFilter(Measurement_t cCcurrentFilter);
    void setCcVoltageFilter(Measurement_t cCvoltageFilter);

    void fillBoardList(uint16_t numOfBoards, uint16_t numOfChannelsOnBoard);
    void fillChannelList(uint16_t numOfBoards, uint16_t numOfChannels);

    void flushBoardList();

    // wrappers for MessageDispatcher get features
    e384cl::ErrorCodes_t getVoltageHoldTunerFeatures(RangedMeasurement_t &voltageHoldTunerFeatures);
    e384cl::ErrorCodes_t getCalibVcCurrentGainFeatures(RangedMeasurement_t &calibVcCurrentGainFeatures);
    e384cl::ErrorCodes_t getCalibVcCurrentOffsetFeatures(vector <RangedMeasurement_t> &calibVcCurrentOffsetFeatures);
    e384cl::ErrorCodes_t getCalibCcVoltageGainFeatures(RangedMeasurement_t &calibCcVoltageGainFeatures);
    e384cl::ErrorCodes_t getCalibCcVoltageOffsetFeatures(vector <RangedMeasurement_t> &calibCcVoltageOffsetFeatures);
    e384cl::ErrorCodes_t getGateVoltagesTunerFeatures(RangedMeasurement_t &gateVoltagesTunerFeatures);
    e384cl::ErrorCodes_t getSourceVoltagesTunerFeatures(RangedMeasurement_t &sourceVoltagesTunerFeatures);
    e384cl::ErrorCodes_t getChannelsNumberFeatures(int &voltageChannelNum, int &currentChannelNum);
    e384cl::ErrorCodes_t getBoardsNumberFeatures(int &boardNum);

    e384cl::ErrorCodes_t getVcCurrentRangesFeatures(vector <e384cl::RangedMeasurement_t> &vcCurrentRangesFeatures);
    e384cl::ErrorCodes_t getVcVoltageRangesFeatures(vector <RangedMeasurement_t> &vcVoltageRangesFeatures);
    e384cl::ErrorCodes_t getCcCurrentRangesFeatures(vector <RangedMeasurement_t> &ccCurrentRangesFeatures);
    e384cl::ErrorCodes_t getCcVoltageRangesFeatures(vector <RangedMeasurement_t> &ccVoltageRangesFeatures);

    e384cl::ErrorCodes_t getVoltageStimulusLpfsFeatures(vector <string> &filterOptions);
    e384cl::ErrorCodes_t getCurrentStimulusLpfsFeatures(vector <string> &filterOptions);
    //---------------------------------------------/

private:
    MessageDispatcher * messageDispatcher = nullptr;
    QString serialNumber = "";

    bool connected = false;
    vector<ModelBoard*> myBoards;
    vector<ModelChannel*> myChannels;
    Measurement_t samplingRate = {0.0, UnitPfxNone, "Hz"};
    RangedMeasurement_t vcCurrentRange = {0.0, 0.0, 0.0, UnitPfxMilli, "V"};
    RangedMeasurement_t vcVoltageRange = {0.0, 0.0, 0.0, UnitPfxMilli, "V"};
    RangedMeasurement_t ccCurrentRange = {0.0, 0.0, 0.0, UnitPfxMilli, "V"};
    RangedMeasurement_t ccVoltageRange = {0.0, 0.0, 0.0, UnitPfxMilli, "V"};
    Measurement_t vcCurrentFilter = {0.0, UnitPfxNone, "Hz"};
    Measurement_t vcVoltageFilter = {0.0, UnitPfxNone, "Hz"};;
    Measurement_t ccCurrentFilter = {0.0, UnitPfxNone, "Hz"};;
    Measurement_t ccVoltageFilter = {0.0, UnitPfxNone, "Hz"};;

};

#endif // MODELDEVICE_H
