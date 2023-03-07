#ifndef MODELDEVICE_H
#define MODELDEVICE_H

#include <QString>
#include <vector>

#include "messagedispatcher.h"
#include "e384commlib_global.h"
#include "e384commlib_global_addendum.h"
#include "modelboard.h"

using namespace e384CommLib;
using namespace std;

namespace e384cl = e384CommLib;

class ModelDevice {
public:
    ModelDevice();

    e384cl::ErrorCodes_t getChannelsNumber(int &voltageChannelNum, int &CurrentChannelNum);
    e384cl::ErrorCodes_t getVcCurrentRanges(vector <e384cl::RangedMeasurement_t> &ranges);

    MessageDispatcher * getMessageDispatcher();
    QString getSerialNumber();
    bool isConnected();
    vector<ModelBoard> getBoards();
    vector<ModelChannel> getChannels();
    Measurement_t getSamplingRate();
    RangedMeasurement_t getVcCurrentRange();
    RangedMeasurement_t getVcVoltageRange();
    RangedMeasurement_t getCcCurrentRange();
    RangedMeasurement_t getCcVoltageRange();
    Measurement_t getVcCurrentFilter();
    Measurement_t getVcVoltageFilter();
    Measurement_t getCcCurrentFilter();
    Measurement_t getCcVoltageFilter();

    void setMessageDispatcher(MessageDispatcher * messageDispatcher);
    void setSerialNumber(QString serial);
    void setConnected(bool flag);
    void setBoards(vector<ModelBoard> boards);
    void setChannels(vector<ModelChannel> channels);
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
    void fillChannelList(uint16_t numOfChannels);

private:
    MessageDispatcher * messageDispatcher = nullptr;
    QString serialNumber = "";

    bool connected = false;
    vector<ModelBoard> myBoards;
    vector<ModelChannel> myChannels;
    Measurement_t samplingRate = {0.0, UnitPfxNone, "Hz"};
    RangedMeasurement_t vCcurrentRange = {0.0, 0.0, 0.0, UnitPfxMilli, "V"};
    RangedMeasurement_t vCvoltageRange = {0.0, 0.0, 0.0, UnitPfxMilli, "V"};
    RangedMeasurement_t cCcurrentRange = {0.0, 0.0, 0.0, UnitPfxMilli, "V"};
    RangedMeasurement_t cCvoltageRange = {0.0, 0.0, 0.0, UnitPfxMilli, "V"};
    Measurement_t vCcurrentFilter = {0.0, UnitPfxNone, "Hz"};
    Measurement_t vCvoltageFilter = {0.0, UnitPfxNone, "Hz"};;
    Measurement_t cCcurrentFilter = {0.0, UnitPfxNone, "Hz"};;
    Measurement_t cCvoltageFilter = {0.0, UnitPfxNone, "Hz"};;

};

#endif // MODELDEVICE_H
