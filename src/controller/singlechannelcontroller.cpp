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

    mainWindow->setDockWidget(MainWindow::DWSingleChannelControl, singleChannelControlsDw, false, Qt::RightDockWidgetArea);
}

SingleChannelController::~SingleChannelController(){
    delete singleChannelControlsDw;
    singleChannelControlsDw = nullptr;
    mainWindow->setDockWidget(MainWindow::DWSingleChannelControl, singleChannelControlsDw);
}

void SingleChannelController::onSingleChannelClicked(uint16_t chIdx, QMouseEvent *event){
    bool newState = event->button() == Qt::LeftButton;
    clickBehaviour(newState);
    auto msgDisp = appStatus->getMessageDispatcher();
    if (newState) {
        // slightly inefficient
        auto selectedIndexes = appStatus->getSelectedChannelsIndexes();
        bool isChSelected = false;
        for (auto idx: selectedIndexes){
            if (idx == chIdx){
                isChSelected = true;
                break;
            }
        }
//        if the channel is selected but the user is pressing ctrl toggle it
        msgDisp->setChannelSelected(chIdx, !((QApplication::keyboardModifiers() & Qt::ControlModifier) && isChSelected));
    } else {
        msgDisp->setChannelSelected(chIdx, newState);
    }
    singleChannelControlsDw->onUpdate();
}

void SingleChannelController::onOneBoardClicked(uint16_t brdIdx, bool newState) {
    clickBehaviour(newState);
    setSelectedStatus(appStatus->getVisibleChannelsOnBoard(brdIdx), newState);
    singleChannelControlsDw->onUpdate();
}

void SingleChannelController::onOneRowClicked(uint16_t rowIdx, bool newState) {
    clickBehaviour(newState);
    setSelectedStatus(appStatus->getVisibleChannelsOnRow(rowIdx), newState);
    singleChannelControlsDw->onUpdate();
}

void SingleChannelController::onAllChannelsClicked(bool newState) {
    clickBehaviour(newState);    
    setSelectedStatus(appStatus->getVisibleChannels(), newState);
    singleChannelControlsDw->onUpdate();
}

void SingleChannelController::clickBehaviour(bool newState){
//    if the newState is false or the user is not pressing ctrl, don't make anything
    if (!newState || (QApplication::keyboardModifiers() & Qt::ControlModifier)){
        return;
    }
    // Ctrl key is pressed
    //    TODO deleteme
    auto msgDisp = appStatus->getMessageDispatcher();
    msgDisp->setAllChannelsSelected(false);
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
        singleChannelControlsDw->setOffsetRecalibrationValues(params.vcOffsetAdc[samplingRateIdx][rangeIdx]);
        break;

    case ClampingModality_t::ZERO_CURRENT_CLAMP:
    case ClampingModality_t::CURRENT_CLAMP:
        msgDisp->getCCVoltageRangeIdx(rangeIdx);
        singleChannelControlsDw->setOffsetRecalibrationValues(params.ccOffsetAdc[samplingRateIdx][rangeIdx]);
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

void SingleChannelController::setSelectedStatus(std::vector<int> channelIndexes, bool newStatus) {
    std::map <int, bool> channelsAndStatus;
    for (auto chIdx: channelIndexes) {
        channelsAndStatus[chIdx] = newStatus;
    }
    appStatus->setSelectedChannels(channelsAndStatus);
}

void SingleChannelController::onBoardMappingLoaded() {
    singleChannelControlsDw->onBoardMappingsLoaded();
}
