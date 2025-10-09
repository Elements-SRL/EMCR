#include "singlechannelcontroller.h"
#include "singlechannelcontroldockwidget.h"
#include "mainwindow.h"

SingleChannelController::SingleChannelController(ApplicationStatus * appStatus, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    singleChannelControlsDw = new SingleChannelControlDockWidget(appStatus, mainWindow);
    connect(singleChannelControlsDw, &SingleChannelControlDockWidget::sigAppliedHoldValues, this, &SingleChannelController::onApplyHoldValues);
    connect(singleChannelControlsDw, &SingleChannelControlDockWidget::sigAppliedOffsetRecalibration, this, &SingleChannelController::onApplyOffsetRecalibration);
    connect(singleChannelControlsDw, &SingleChannelControlDockWidget::sigAppliedStimHalfValues, this, &SingleChannelController::onApplyStimHalfValues);
    connect(singleChannelControlsDw, &SingleChannelControlDockWidget::sigLiquidJunctionValues, this, &SingleChannelController::onLiquidJunctionValues);
    connect(singleChannelControlsDw, &SingleChannelControlDockWidget::sigAppliedOffsetTracking, this, &SingleChannelController::onApplyOffsetTracking);

    mainWindow->setDockWidget(MainWindow::DWSingleChannelControl, singleChannelControlsDw, false, Qt::RightDockWidgetArea);
}

SingleChannelController::~SingleChannelController(){
    delete singleChannelControlsDw;
    singleChannelControlsDw = nullptr;
    mainWindow->setDockWidget(MainWindow::DWSingleChannelControl, singleChannelControlsDw);
}

void SingleChannelController::onChannelsSelected() {
    singleChannelControlsDw->onUpdate();
}

void SingleChannelController::onApplyTurnStimulusOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
    auto msgDisp = appStatus->getMessageDispatcher();
    msgDisp->enableStimulus(channelIndexes, onValues, true);
}

void SingleChannelController::onApplyTurnDocOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
    auto msgDisp = appStatus->getMessageDispatcher();
    msgDisp->digitalOffsetCompensation(channelIndexes, onValues, true);
}

void SingleChannelController::onApplyHoldValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> holdValues){
    ClampingModality_t mode;
    auto msgDisp = appStatus->getMessageDispatcher();
    msgDisp->getClampingModality(mode);

    if (mode == ClampingModality_t::VOLTAGE_CLAMP) {
        msgDisp->setVoltageHoldTuner(channelIndexes, holdValues, true);

    } else {
        msgDisp->setCurrentHoldTuner(channelIndexes, holdValues, true);
    }
}

void SingleChannelController::onApplyOffsetRecalibration(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> offsetValues){
    ClampingModality_t mode;
    auto msgDisp = appStatus->getMessageDispatcher();
    msgDisp->getClampingModality(mode);

    if (mode == ClampingModality_t::VOLTAGE_CLAMP) {
        msgDisp->setCalibVcCurrentOffset(channelIndexes, offsetValues, true);

    } else {
        msgDisp->setCalibCcVoltageOffset(channelIndexes, offsetValues, true);
    }
}

void SingleChannelController::onApplyStimHalfValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> halfValues){
    ClampingModality_t mode;
    auto msgDisp = appStatus->getMessageDispatcher();
    msgDisp->getClampingModality(mode);

    if (mode == ClampingModality_t::VOLTAGE_CLAMP) {
        msgDisp->setVoltageHalf(channelIndexes, halfValues, true);

    } else {
        msgDisp->setCurrentHalf(channelIndexes, halfValues, true);
    }
}

void SingleChannelController::onApplyOffsetTracking(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> offsetValues){
    ClampingModality_t mode;
    auto msgDisp = appStatus->getMessageDispatcher();
    msgDisp->getClampingModality(mode);

    if (mode == ClampingModality_t::VOLTAGE_CLAMP) {
        msgDisp->setCurrentTracking(channelIndexes, offsetValues, true);

    // } else {
    //     msgDisp->setCalibCcVoltageOffset(channelIndexes, offsetValues, true);
    }
}

void SingleChannelController::onLiquidJunctionValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> values) {
    ClampingModality_t mode;
    auto msgDisp = appStatus->getMessageDispatcher();
    msgDisp->getClampingModality(mode);

    if (mode == ClampingModality_t::VOLTAGE_CLAMP) {
        msgDisp->setLiquidJunctionVoltage(channelIndexes, values, true);
    }
}

void SingleChannelController::onOffsetRecalibrationResult() {
    ClampingModality_t mode;
    CalibrationParams_t params;
    auto msgDisp = appStatus->getMessageDispatcher();
    msgDisp->getClampingModality(mode);
    msgDisp->getCalibParams(params);
    uint32_t samplingRateIdx;
    msgDisp->getSamplingRateIdx(samplingRateIdx);
    uint32_t rangeIdx;

    switch (mode) {
    case ClampingModality_t::VOLTAGE_CLAMP:
        msgDisp->getVCCurrentRangeIdx(rangeIdx);
        singleChannelControlsDw->setOffsetRecalibrationValues(params.getValues(CalTypesVcOffsetAdc, samplingRateIdx, rangeIdx));
        break;

    case ClampingModality_t::ZERO_CURRENT_CLAMP:
    case ClampingModality_t::CURRENT_CLAMP:
        msgDisp->getCCVoltageRangeIdx(rangeIdx);
        singleChannelControlsDw->setOffsetRecalibrationValues(params.getValues(CalTypesCcOffsetAdc, samplingRateIdx, rangeIdx));
        break;
    }
}

void SingleChannelController::onLiquidJunctionResult() {
    uint16_t currentChannelsNum = appStatus->getCurrentChannelsNum();
    std::vector <uint16_t> channelIdxs(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        channelIdxs[idx] = idx;
    }

    std::vector <Measurement_t> voltages;
    auto msgDisp = appStatus->getMessageDispatcher();
    msgDisp->getLiquidJunctionVoltages(channelIdxs, voltages);

    singleChannelControlsDw->setLiquidJunctionVoltages(voltages);
}

void SingleChannelController::onBoardMappingLoaded() {
    singleChannelControlsDw->onBoardMappingsLoaded();
}
