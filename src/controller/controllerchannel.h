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
    void onApplyTurnChannelOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void onApplyTurnStimulusOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void onApplyTurnDocOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void onApplyVoltageHoldValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> vHoldValues);

    /*! \todo void onApplyKawaiiPlotSettings(Boh);*/


    signals:
    // signals sent to Channel Control Dock Widget (I've already done my stuff)
    void sigSelectedChannelsUpdated();
};

#endif // CONTROLLERCHANNEL_H
