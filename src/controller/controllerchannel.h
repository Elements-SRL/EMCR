#ifndef CONTROLLERCHANNEL_H
#define CONTROLLERCHANNEL_H

#include <QObject>
#include <QVector>
#include <QDebug>

#include "modeldevice.h"
#include "channelcontroldockwidget.h"
#include "mainwindow.h"

class ControllerChannel : public QObject {
    Q_OBJECT

public:
    ControllerChannel(ModelDevice * mDev, MainWindow * mainWindow);

private:
    ModelDevice * mDev = nullptr;
    MainWindow * mainWindow = nullptr;
    ChannelControlDockWidget * channelControlsDw = nullptr;
    void updateView();
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
    void onApplyHoldValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> vHoldValues);

    /*! \todo void onApplyKawaiiPlotSettings(Boh);*/

    // Compensations
    void onCompensationApplied(std::vector<uint16_t> channelIndexes, std::vector<bool> cfastEn, std::vector<bool> cslowRsEn, std::vector<bool> rsCpEn, std::vector<bool> rsPgEn, std::vector<double> cfastValues, std::vector<double> cslowValues, std::vector<double> rsValues, std::vector<double> rsCpValues, std::vector<double> rsPgValues, std::vector<uint16_t> rsBWValueIdxs, std::vector<bool> ccCfastEn, std::vector<double> ccCfastValues);


    signals:
    // signals sent to Channel Control Dock Widget (I've already done my stuff)

    // signals sent to Compensation Control Widget
    void sigCompValuesDispatched(std::vector<std::vector<double>> compValueMatrix, std::vector<RangedMeasurement> cfastFeatures, std::vector<RangedMeasurement> cslowFeatures, std::vector<RangedMeasurement> rsFeatures, std::vector<RangedMeasurement> rsCpFeatures, std::vector<RangedMeasurement> rsPgFeatures, std::vector<RangedMeasurement> ccCfastFeatures);
};

#endif // CONTROLLERCHANNEL_H
