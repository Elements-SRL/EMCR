#ifndef CONTROLLERCHANNEL_H
#define CONTROLLERCHANNEL_H

#include <QObject>
#include <QVector>

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
    void onApplyTurnChannelOnOff(vector<uint16_t> channelIndexes, vector<bool> onvalues);
    void onApplyTurnDocOnOff(vector<uint16_t> channelIndexes, vector<bool> onvalues);
    void onApplyVoltageHoldValues(vector<uint16_t> channelIndexes, vector<Measurement_t> voltages);
    /*! \todo void onApplyKawaiiPlotSettings(Boh);*/


    signals:
    // signals sent to Channel Control Dock Widget (I've already done my stuff)
    void sigUpdateChannelControlDockWidget();

    // signals received from Channel Control Dock Widget (I still have to do my stuff)
    void sigAppliedTurnChannelOnOff(vector<uint16_t> channelIndexes, vector<bool> onvalues);
    void sigAppliedTurnDocOnOff(vector<uint16_t> channelIndexes, vector<bool> onvalues);
    void sigAppliedVoltageHoldValues(vector<uint16_t> channelIndexes, vector<Measurement_t> voltages);
    /*! \todo sigAppliedKawaiiPlotSettings(BOH)*/
};

#endif // CONTROLLERCHANNEL_H
