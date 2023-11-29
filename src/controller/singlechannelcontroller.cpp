#include "singlechannelcontroller.h"
#include "singlechannelcontroldockwidget.h"
#include "mainwindow.h"

SingleChannelController::SingleChannelController(ApplicationStatus * appStatus, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    singleChannelControlsDw = new SingleChannelControlDockWidget(appStatus, mainWindow);
    connect(singleChannelControlsDw, &SingleChannelControlDockWidget::sigAppliedHoldValues, this, &SingleChannelController::onApplyHoldValues);
    connect(singleChannelControlsDw, &SingleChannelControlDockWidget::sigAppliedStimHalfValues, this, &SingleChannelController::onApplyStimHalfValues);
    connect(singleChannelControlsDw, &SingleChannelControlDockWidget::sigLiquidJunctionValues, this, &SingleChannelController::onLiquidJunctionValues);

    if (mainWindow->getCompensationControlsDockWidget() != nullptr) {
        connect(mainWindow->getCompensationControlsDockWidget(), &CompensationControlDockWidget::sigCompensationsApplied, this, [=](std::vector<uint16_t> channelIndexes, std::vector<bool> cfastEn, std::vector<bool> cslowRsEn, std::vector<bool> rsCpEn, std::vector<bool> rsPgEn, std::vector<double> cfastValues, std::vector<double> cslowValues, std::vector<double> rsValues, std::vector<double> rsCpValues, std::vector<double> rsPgValues, std::vector<uint16_t> rsBWValueIdxs, std::vector<bool> ccCfastEn, std::vector<double> ccCfastValues){
            onCompensationApplied(channelIndexes, cfastEn, cslowRsEn, rsCpEn, rsPgEn, cfastValues, cslowValues, rsValues, rsCpValues, rsPgValues, rsBWValueIdxs, ccCfastEn, ccCfastValues);
        });
    }
    mainWindow->setSingleChannelControlsDw(singleChannelControlsDw);
}

SingleChannelController::~SingleChannelController(){
    delete singleChannelControlsDw;
    singleChannelControlsDw = nullptr;
    mainWindow->setSingleChannelControlsDw(singleChannelControlsDw);
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

void SingleChannelController::onLiquidJunctionValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> values){
    ClampingModality_t mode;
    auto msgDisp = appStatus->getMessageDispatcher();
    msgDisp->getClampingModality(mode);

    if (mode == ClampingModality_t::VOLTAGE_CLAMP) {
        msgDisp->setLiquidJunctionVoltage(channelIndexes, values, true);
    }
}

void SingleChannelController::onCompensationApplied(std::vector<uint16_t> channelIndexes, std::vector<bool> cfastEn, std::vector<bool> cslowRsEn, std::vector<bool> rsCpEn, std::vector<bool> rsPgEn, std::vector<double> cfastValues, std::vector<double> cslowValues, std::vector<double> rsValues, std::vector<double> rsCpValues, std::vector<double> rsPgValues, std::vector<uint16_t> rsBWValueIdxs, std::vector<bool> ccCfastEn, std::vector<double> ccCfastValues){
    ClampingModality_t mode;
    auto msgDisp = appStatus->getMessageDispatcher();
    msgDisp->getClampingModality(mode);
    std::vector<std::vector<double>> compValueMatrix;
    std::vector<RangedMeasurement> cfastFeatures;
    std::vector<RangedMeasurement> cslowFeatures;
    std::vector<RangedMeasurement> rsFeatures;
    std::vector<RangedMeasurement> rsCpFeatures;
    std::vector<RangedMeasurement> rsPgFeatures;
    std::vector<RangedMeasurement> ccCfastFeatures;

    compValueMatrix.resize(channelIndexes.size(), std::vector<double>(MessageDispatcher::CompensationUserParamsNum));
    cfastFeatures.resize(channelIndexes.size());
    cslowFeatures.resize(channelIndexes.size());
    rsFeatures.resize(channelIndexes.size());
    rsCpFeatures.resize(channelIndexes.size());
    rsPgFeatures.resize(channelIndexes.size());
    ccCfastFeatures.resize(channelIndexes.size());

    msgDisp->enableCompensation(channelIndexes, MessageDispatcher::CompCfast, cfastEn, true);
    msgDisp->enableCompensation(channelIndexes, MessageDispatcher::CompCslow, cslowRsEn, true);
    msgDisp->enableCompensation(channelIndexes, MessageDispatcher::CompRsCorr, rsCpEn, true);
    msgDisp->enableCompensation(channelIndexes, MessageDispatcher::CompRsPred, rsPgEn, true);
    msgDisp->enableCompensation(channelIndexes, MessageDispatcher::CompCcCfast, ccCfastEn, true);

    /*! \todo MPAC discriminare tra voltage e current clamp*/
    if(mode == ClampingModality_t::VOLTAGE_CLAMP){
        msgDisp->setCompValues(channelIndexes, MessageDispatcher::U_CpVc, cfastValues, true);

    } else if(mode == ClampingModality_t::ZERO_CURRENT_CLAMP || mode == ClampingModality_t::CURRENT_CLAMP) {
        msgDisp->setCompValues(channelIndexes, MessageDispatcher::U_CpCc, ccCfastValues, true);

    } else {
        /*! \todo MPAC ancora da fare*/
    }
    msgDisp->setCompValues(channelIndexes, MessageDispatcher::U_Cm, cslowValues, true);
    msgDisp->setCompValues(channelIndexes, MessageDispatcher::U_Rs, rsValues, true);
    msgDisp->setCompValues(channelIndexes, MessageDispatcher::U_RsCp, rsCpValues, true);
    msgDisp->setCompValues(channelIndexes, MessageDispatcher::U_RsPg, rsPgValues, true);
    msgDisp->setCompOptions(channelIndexes, MessageDispatcher::CompRsCorr, rsBWValueIdxs, true);

    msgDisp->getCompValueMatrix(compValueMatrix);

    /*! \todo MPAC discriminare tra voltage e current clamp*/
    double defaultParamValue;
    if(mode == ClampingModality_t::VOLTAGE_CLAMP){
        msgDisp->getCompFeatures(MessageDispatcher::U_CpVc, cfastFeatures, defaultParamValue);

    } else if(mode == ClampingModality_t::ZERO_CURRENT_CLAMP || mode == ClampingModality_t::CURRENT_CLAMP) {
        msgDisp->getCompFeatures(MessageDispatcher::U_CpCc, ccCfastFeatures, defaultParamValue);

    } else {
        /*! \todo MPAC ancora da fare*/
    }
    msgDisp->getCompFeatures(MessageDispatcher::U_Cm, cslowFeatures, defaultParamValue);
    msgDisp->getCompFeatures(MessageDispatcher::U_Rs, rsFeatures, defaultParamValue);
    msgDisp->getCompFeatures(MessageDispatcher::U_RsCp, rsCpFeatures, defaultParamValue);
    msgDisp->getCompFeatures(MessageDispatcher::U_RsPg, rsPgFeatures, defaultParamValue);

    mainWindow->getCompensationControlsDockWidget()->onCompValuesDispatched(compValueMatrix, cfastFeatures, cslowFeatures, rsFeatures, rsCpFeatures, rsPgFeatures, ccCfastFeatures);
}

void SingleChannelController::onLiquidJunctionResult(){
    uint16_t currentChannelsNum = appStatus->getCurrentChannelsNum();
    std::vector <uint16_t> channelIdxs(currentChannelsNum);
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        channelIdxs[idx] = idx;
    }

    std::vector <Measurement_t> stdVoltages;
    auto msgDisp = appStatus->getMessageDispatcher();
    msgDisp->getLiquidJunctionVoltages(channelIdxs, stdVoltages);

    singleChannelControlsDw->setLiquidJunctionVoltages(stdVoltages);
}

void SingleChannelController::setSelectedStatus(std::vector<int> channelIndexes, bool newStatus){
    std::map<int, bool> channelsAndStatus;
    for(auto chIdx: channelIndexes){
        channelsAndStatus[chIdx] = newStatus;
    }
    appStatus->setSelectedChannels(channelsAndStatus);
}
