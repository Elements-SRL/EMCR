#include "compensationcontroller.h"

CompensationController::CompensationController(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp),
    mainWindow(mainWindow) {

    if (msgDisp->hasCompFeature(MessageDispatcher::U_CpVc) != Success &&
            msgDisp->hasCompFeature(MessageDispatcher::U_Cm) != Success &&
            msgDisp->hasCompFeature(MessageDispatcher::U_Rs) != Success &&
            msgDisp->hasCompFeature(MessageDispatcher::U_RsCp) != Success &&
            msgDisp->hasCompFeature(MessageDispatcher::U_RsPg) != Success &&
            msgDisp->hasCompFeature(MessageDispatcher::U_CpCc) != Success) {
        return;
    }

    compensationControlDockWidget = new CompensationControlDockWidget(msgDisp);
    mainWindow->setDockWidget(MainWindow::DWCompensationControl, compensationControlDockWidget, true, Qt::LeftDockWidgetArea);

    connect(compensationControlDockWidget, &CompensationControlDockWidget::sigCompensationsApplied, this, [=](
            std::vector<uint16_t> channelIndexes,
            std::vector<bool> cfastEn,
            std::vector<bool> cslowRsEn,
            std::vector<bool> rsCpEn,
            std::vector<bool> rsPgEn,
            std::vector<double> cfastValues,
            std::vector<double> cslowValues,
            std::vector<double> rsValues,
            std::vector<double> rsCpValues,
            std::vector<double> rsPgValues,
            std::vector<uint16_t> rsBWValueIdxs,
            std::vector<bool> ccCfastEn,
            std::vector<double> ccCfastValues){
        onCompensationApplied(channelIndexes, cfastEn, cslowRsEn, rsCpEn, rsPgEn, cfastValues, cslowValues, rsValues, rsCpValues, rsPgValues, rsBWValueIdxs, ccCfastEn, ccCfastValues);
    });
}

CompensationController::~CompensationController() {
    delete compensationControlDockWidget;
    compensationControlDockWidget = nullptr;
    mainWindow->setDockWidget(MainWindow::DWCompensationControl, compensationControlDockWidget);
}

void CompensationController::onCompensationApplied(std::vector<uint16_t> channelIndexes, std::vector<bool> cfastEn, std::vector<bool> cslowRsEn, std::vector<bool> rsCpEn, std::vector<bool> rsPgEn, std::vector<double> cfastValues, std::vector<double> cslowValues, std::vector<double> rsValues, std::vector<double> rsCpValues, std::vector<double> rsPgValues, std::vector<uint16_t> rsBWValueIdxs, std::vector<bool> ccCfastEn, std::vector<double> ccCfastValues){
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

    if (!cfastEn.empty()) {
        msgDisp->enableCompensation(channelIndexes, MessageDispatcher::CompCfast, cfastEn, false);
    }
    if (!cslowRsEn.empty()) {
        msgDisp->enableCompensation(channelIndexes, MessageDispatcher::CompCslow, cslowRsEn, false);
    }
    if (!rsCpEn.empty()) {
        msgDisp->enableCompensation(channelIndexes, MessageDispatcher::CompRsCorr, rsCpEn, false);
    }
    if (!rsPgEn.empty()) {
        msgDisp->enableCompensation(channelIndexes, MessageDispatcher::CompRsPred, rsPgEn, false);
    }
    if (!ccCfastEn.empty()) {
        msgDisp->enableCompensation(channelIndexes, MessageDispatcher::CompCcCfast, ccCfastEn, false);
    }

    if (mode == ClampingModality_t::VOLTAGE_CLAMP || mode == ClampingModality_t::VOLTAGE_CLAMP_VOLTAGE_READ) {
        if (!cfastValues.empty()) {
            msgDisp->setCompValues(channelIndexes, MessageDispatcher::U_CpVc, cfastValues, false);
        }

    } else if(mode == ClampingModality_t::ZERO_CURRENT_CLAMP || mode == ClampingModality_t::CURRENT_CLAMP) {
        if (!ccCfastValues.empty()) {
            msgDisp->setCompValues(channelIndexes, MessageDispatcher::U_CpCc, ccCfastValues, false);
        }

    } else {
        /*! \todo MPAC ancora da fare*/
    }
    if (!cslowValues.empty()) {
        msgDisp->setCompValues(channelIndexes, MessageDispatcher::U_Cm, cslowValues, false);
    }
    if (!rsValues.empty()) {
        msgDisp->setCompValues(channelIndexes, MessageDispatcher::U_Rs, rsValues, false);
    }
    if (!rsCpValues.empty()) {
        msgDisp->setCompValues(channelIndexes, MessageDispatcher::U_RsCp, rsCpValues, false);
    }
    if (!rsPgValues.empty()) {
        msgDisp->setCompValues(channelIndexes, MessageDispatcher::U_RsPg, rsPgValues, false);
    }
    if (!rsBWValueIdxs.empty()) {
        msgDisp->setCompOptions(channelIndexes, MessageDispatcher::CompRsCorr, rsBWValueIdxs, false);
    }
    msgDisp->sendCommands();

    msgDisp->getCompValueMatrix(compValueMatrix);

    double defaultParamValue;
    if (mode == ClampingModality_t::VOLTAGE_CLAMP || mode == ClampingModality_t::VOLTAGE_CLAMP_VOLTAGE_READ) {
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

    compensationControlDockWidget->onCompValuesDispatched(compValueMatrix, cfastFeatures, cslowFeatures, rsFeatures, rsCpFeatures, rsPgFeatures, ccCfastFeatures);
}
