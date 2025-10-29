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

public slots:
    void onChannelsSelected();
    void onApplyTurnStimulusOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void onApplyTurnDocOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void onApplyHoldValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> vHoldValues);
    void onApplyOffsetRecalibration(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> offsetValues);
    void onApplyStimHalfValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> halfValues);
    void onApplyOffsetTracking(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> offsetValues);
    void onLiquidJunctionValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> values);
    void onApplyRamp(std::vector <uint16_t> channelIndexes, std::vector <Measurement_t> vInitial, std::vector <Measurement_t> vFinal, std::vector <Measurement_t> duration);
    void onOffsetRecalibrationResult();
    void onLiquidJunctionResult();

signals:
    // signals sent to Channel Control Dock Widget (I've already done my stuff)

    // signals sent to Compensation Control Widget
    void sigCompValuesDispatched(std::vector<std::vector<double>> compValueMatrix, std::vector<RangedMeasurement> cfastFeatures, std::vector<RangedMeasurement> cslowFeatures, std::vector<RangedMeasurement> rsFeatures, std::vector<RangedMeasurement> rsCpFeatures, std::vector<RangedMeasurement> rsPgFeatures, std::vector<RangedMeasurement> ccCfastFeatures);
};

#endif // SINGLECHANNELCONTROLLER_H
