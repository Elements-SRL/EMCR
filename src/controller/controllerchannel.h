#ifndef CONTROLLERCHANNEL_H
#define CONTROLLERCHANNEL_H

#include <QObject>
#include <QVector>
#include <QDebug>

#include "modeldevice.h"


class ControllerChannel : public QObject {
    Q_OBJECT

public:
    ControllerChannel(ModelDevice * mDev);

    void setModelDevice(ModelDevice * mDev);

private:
    ModelDevice * mDev = nullptr;

public slots:
    // To do on actions done on the chessboard
    void onSingleChannelClicked(uint16_t changedChannelIndexes, bool newChannelState);
    void onOneBoardClicked(uint16_t changedBoardIndex, bool newChannelState);
    void onOneRowClicked(uint16_t changedRowIndexes, bool newChannelState);
    void onAllChannelsClicked(bool newChannelState);

    // To do on apply pushed on the Channel Control Dock Widget
    void onApplyTurnChannelOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues);
    void onApplyTurnStimulusOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues);
    void onApplyTurnDocOnOff(vector<uint16_t> channelIndexes, vector<bool> onValues);
    void onApplyVoltageHoldValues(vector<uint16_t> channelIndexes, vector<Measurement_t> vHoldValues);

    /*! \todo void onApplyKawaiiPlotSettings(Boh);*/

    // Compensations
    void onCompensationApplied(vector<uint16_t> channelIndexes, vector<bool> cfastEn, vector<bool> cslowRsEn, vector<bool> rsCpEn, vector<bool> rsPgEn, vector<double> cfastValues, vector<double> cslowValues, vector<double> rsValues, vector<double> rsCpValues, vector<double> rsPgValues, vector<uint16_t> rsBWValueIdxs, vector<bool> ccCfastEn, vector<double> ccCfastValues);


    signals:
    // signals sent to Channel Control Dock Widget (I've already done my stuff)
    void sigSelectedChannelsUpdated();

    // signals sent to Compensation Control Widget
    void sigCompValuesDispatched(vector<vector<double>> compValueMatrix, vector<RangedMeasurement> cfastFeatures, vector<RangedMeasurement> cslowFeatures, vector<RangedMeasurement> rsFeatures, vector<RangedMeasurement> rsCpFeatures, vector<RangedMeasurement> rsPgFeatures, vector<RangedMeasurement> ccCfastFeatures);
};

#endif // CONTROLLERCHANNEL_H
