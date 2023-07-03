#include "controllerchannel.h"
#include "channelcontroldockwidget.h"
#include "mainwindow.h"

ControllerChannel::ControllerChannel(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp),
    mainWindow(mainWindow) {

    channelControlsDw = new ChannelControlDockWidget(msgDisp);
    connect(channelControlsDw, &ChannelControlDockWidget::sigAppliedTurnChannelOnOff, this, [=](std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
        onApplyTurnChannelOnOff(channelIndexes, onValues);
    });
    connect(channelControlsDw, &ChannelControlDockWidget::sigAppliedTurnStimulsOnOff, this, [=](std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
        onApplyTurnStimulusOnOff(channelIndexes, onValues);
    });
    connect(channelControlsDw, &ChannelControlDockWidget::sigAppliedTurnDocOnOff, this, [=](std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
        onApplyTurnDocOnOff(channelIndexes, onValues);
    });
    connect(channelControlsDw, &ChannelControlDockWidget::sigAppliedHoldValues, this, [=](std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> holdValues){
        onApplyHoldValues(channelIndexes, holdValues);
    });
    connect(mainWindow->getChessaboard(), &Chessboard::allChannelsClicked, this, [=](bool newChannelState){
        onAllChannelsClicked(newChannelState);
    });
    connect(mainWindow->getChessaboard(), &Chessboard::oneRowClicked, this, [=](uint16_t changedRowIndex, bool newChannelState){
        onOneRowClicked(changedRowIndex, newChannelState);
    });
    connect(mainWindow->getChessaboard(), &Chessboard::oneBoardClicked, this, [=](uint16_t changedBoardIndex, bool newChannelState){
        onOneBoardClicked(changedBoardIndex, newChannelState);
    });
    connect(mainWindow->getChessaboard(), &Chessboard::singleChannelClicked, this, [=](uint16_t changedChannelIndexes, bool newChannelState){
        onSingleChannelClicked(changedChannelIndexes, newChannelState);
    });
    connect(mainWindow->getCompensationControlsDockWidget(), &CompensationControlDockWidget::sigCompensationsApplied, this, [=](std::vector<uint16_t> channelIndexes, std::vector<bool> cfastEn, std::vector<bool> cslowRsEn, std::vector<bool> rsCpEn, std::vector<bool> rsPgEn, std::vector<double> cfastValues, std::vector<double> cslowValues, std::vector<double> rsValues, std::vector<double> rsCpValues, std::vector<double> rsPgValues, std::vector<uint16_t> rsBWValueIdxs, std::vector<bool> ccCfastEn, std::vector<double> ccCfastValues){
        onCompensationApplied(channelIndexes, cfastEn, cslowRsEn, rsCpEn, rsPgEn, cfastValues, cslowValues, rsValues, rsCpValues, rsPgValues, rsBWValueIdxs, ccCfastEn, ccCfastValues);
    });
    mainWindow->setChannelControlsDw(channelControlsDw);
}

void ControllerChannel::onSingleChannelClicked(uint16_t chIdx, bool newState){
    msgDisp->setChannelSelected(chIdx, newState);
    this->updateView();
}

void ControllerChannel::onOneBoardClicked(uint16_t brdIdx, bool newState) {
    msgDisp->setBoardSelected(brdIdx, newState);
    this->updateView();
}

void ControllerChannel::onOneRowClicked(uint16_t rowIdx, bool newState) {
    msgDisp->setRowSelected(rowIdx, newState);
    this->updateView();
}

void ControllerChannel::onAllChannelsClicked(bool newState) {
    msgDisp->setAllChannelsSelected(newState);
    this->updateView();
}

void ControllerChannel::onApplyTurnChannelOnOff(std::vector <uint16_t> channelIndexes, std::vector<bool> onValues){
    msgDisp->turnChannelsOn(channelIndexes, onValues, true);
}

void ControllerChannel::onApplyTurnStimulusOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
    msgDisp->enableStimulus(channelIndexes, onValues, true);
}

void ControllerChannel::onApplyTurnDocOnOff(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues){
    msgDisp->digitalOffsetCompensation(channelIndexes, onValues, true);
}

void ControllerChannel::onApplyHoldValues(std::vector<uint16_t> channelIndexes, std::vector<Measurement_t> holdValues){
    ClampingModality_t mode;
    msgDisp->getClampingModality(mode);

    if (mode == ClampingModality_t::VOLTAGE_CLAMP) {
        msgDisp->setVoltageHoldTuner(channelIndexes, holdValues, true);

    } else {
        msgDisp->setCurrentHoldTuner(channelIndexes, holdValues, true);
    }
}

void ControllerChannel::onCompensationApplied(std::vector<uint16_t> channelIndexes, std::vector<bool> cfastEn, std::vector<bool> cslowRsEn, std::vector<bool> rsCpEn, std::vector<bool> rsPgEn, std::vector<double> cfastValues, std::vector<double> cslowValues, std::vector<double> rsValues, std::vector<double> rsCpValues, std::vector<double> rsPgValues, std::vector<uint16_t> rsBWValueIdxs, std::vector<bool> ccCfastEn, std::vector<double> ccCfastValues){
    ClampingModality_t mode;
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

void ControllerChannel::updateView(){
    channelControlsDw->onUpdate();
    mainWindow->getChessaboard()->onSelectedPlotsUdpated();
}


