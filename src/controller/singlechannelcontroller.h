#ifndef SINGLECHANNELCONTROLLER_H
#define SINGLECHANNELCONTROLLER_H

#include <QObject>
#include <QVector>

#include "singlechannelcontroldockwidget.h"
#include "mainwindow.h"
#include "application_status.h"
#include "messagedispatcher.h"
#include <QMouseEvent>
#include <QApplication>

class SingleChannelController : public QObject {
    Q_OBJECT

public:
    SingleChannelController(ApplicationStatus * appStatus, MainWindow * mainWindow);
    ~SingleChannelController();
    void onBoardMappingLoaded();
private:
    ApplicationStatus * appStatus = nullptr;
    MainWindow * mainWindow = nullptr;
    SingleChannelControlDockWidget * singleChannelControlsDw = nullptr;
    void clickBehaviour(bool newState);
    void setSelectedStatus(std::vector<int>, bool);

public slots:
    // To do on actions done on the chessboard
    void onSingleChannelClicked(uint16_t chIdx, QMouseEvent *event);
    void onOneBoardClicked(uint16_t brdIdx, bool newState);
    void onOneRowClicked(uint16_t rowIdx, bool newState);
    void onAllChannelsClicked(bool newState);

    // To do on apply pushed on the Channel Control Dock Widget
    void onApplyTurnStimulusOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void onApplyTurnDocOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void onApplyHoldValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> vHoldValues);
    void onApplyStimHalfValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> halfValues);
    void onLiquidJunctionValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> values);
    void onLiquidJunctionResult();

    // Compensations
    void onCompensationApplied(std::vector<uint16_t> channelIndexes, std::vector<bool> cfastEn, std::vector<bool> cslowRsEn, std::vector<bool> rsCpEn, std::vector<bool> rsPgEn, std::vector<double> cfastValues, std::vector<double> cslowValues, std::vector<double> rsValues, std::vector<double> rsCpValues, std::vector<double> rsPgValues, std::vector<uint16_t> rsBWValueIdxs, std::vector<bool> ccCfastEn, std::vector<double> ccCfastValues);

signals:
    // signals sent to Channel Control Dock Widget (I've already done my stuff)

    // signals sent to Compensation Control Widget
    void sigCompValuesDispatched(std::vector<std::vector<double>> compValueMatrix, std::vector<RangedMeasurement> cfastFeatures, std::vector<RangedMeasurement> cslowFeatures, std::vector<RangedMeasurement> rsFeatures, std::vector<RangedMeasurement> rsCpFeatures, std::vector<RangedMeasurement> rsPgFeatures, std::vector<RangedMeasurement> ccCfastFeatures);
};

#endif // SINGLECHANNELCONTROLLER_H
