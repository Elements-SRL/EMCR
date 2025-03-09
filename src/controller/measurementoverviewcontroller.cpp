#include "measurementoverviewcontroller.h"

MeasurementOverviewController::MeasurementOverviewController(ApplicationStatus * appStatus, DeviceDataProducer * producer, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    voltageChannelsNum = appStatus->getVoltageChannelsNum();
    currentChannelsNum = appStatus->getCurrentChannelsNum();
    activeChannelsIdxs = appStatus->getSelectedChannelsIndexes();
    modw = new MeasurementsOverviewDockWidget(activeChannelsIdxs, voltageChannelsNum, currentChannelsNum, mainWindow);
    modm = new MeasurementOverviewModel(activeChannelsIdxs, voltageChannelsNum, currentChannelsNum);
    connect(modw, &MeasurementsOverviewDockWidget::extract, this, [=](QString filepath){
        modm->exportToCsv(filepath.toStdString());
    });
    liveStatisticsConsumer = new LiveStatisticsConsumer(appStatus, producer);
    resistanceEstimationConsumer = new ResistanceEstimationConsumer(appStatus, producer);
    connect(liveStatisticsConsumer, &LiveStatisticsConsumer::sigResult, this, &MeasurementOverviewController::onLiveStatisticsResults);
    connect(resistanceEstimationConsumer, &ResistanceEstimationConsumer::sigResult, this, &MeasurementOverviewController::onResistanceEstimationResults);
    connect(modw, &QDockWidget::visibilityChanged, this, &MeasurementOverviewController::onSetLiveStatisticsConsumerStatus);

    mainWindow->setDockWidget(MainWindow::DWMeasurementsOverview, modw, false, Qt::BottomDockWidgetArea);
}

void MeasurementOverviewController::onSetLiveStatisticsConsumerStatus(bool status) {
    if (status) {
        liveStatisticsConsumer->onStartConsuming();
    }
    else {
        liveStatisticsConsumer->onStopConsuming();
    }
}

MeasurementOverviewController::~MeasurementOverviewController(){
    delete modw;
    modw = nullptr;
    mainWindow->setDockWidget(MainWindow::DWMeasurementsOverview, modw);
    if (liveStatisticsConsumer!= nullptr) {
        liveStatisticsConsumer->onStopConsuming();
        delete liveStatisticsConsumer;
        liveStatisticsConsumer = nullptr;
    }
    if (resistanceEstimationConsumer!= nullptr) {
        resistanceEstimationConsumer->onStopConsuming();
        delete resistanceEstimationConsumer;
        resistanceEstimationConsumer = nullptr;
    }
}

void MeasurementOverviewController::onOffsetRecalibrationResult(bool started) {
    if (!started) {
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
            modm->setOffsetRecalibrationResults(params.getValues(CalTypesVcOffsetAdc, samplingRateIdx, rangeIdx));
            modw->setOffsetRecalibrationResult(params.getValues(CalTypesVcOffsetAdc, samplingRateIdx, rangeIdx));
            break;

        case ClampingModality_t::ZERO_CURRENT_CLAMP:
        case ClampingModality_t::CURRENT_CLAMP:
            msgDisp->getCCVoltageRangeIdx(rangeIdx);
            modm->setOffsetRecalibrationResults(params.getValues(CalTypesCcOffsetAdc, samplingRateIdx, rangeIdx));
            modw->setOffsetRecalibrationResult(params.getValues(CalTypesCcOffsetAdc, samplingRateIdx, rangeIdx));
            break;
        }
    }
}

void MeasurementOverviewController::onLiquidJunctionResult(bool started) {
    if (!started) {
        std::vector <uint16_t> channelIdxs(currentChannelsNum);
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            channelIdxs[idx] = idx;
        }

        std::vector <Measurement_t> voltages;
        auto msgDisp = appStatus->getMessageDispatcher();
        msgDisp->getLiquidJunctionVoltages(channelIdxs, voltages);

        modm->setLiquidJunctionResults(voltages);
        modw->setLiquidJunctionResult(voltages);
    }
}

void MeasurementOverviewController::onProtocolStarted(unsigned int protId, ProtocolWidget * protocol) {
    resistanceEstimationConsumer->onStopConsuming();
    YAML::AnalysisType_t type = YAML::AnalysisNum;
    if (!(protocol->getAnalysisType(type))) {
        return;
    }
    switch (type) {
    case YAML::ResistanceEstimation:
        resistanceEstimationConsumer->onStartConsuming();
        return;
    }
}

void MeasurementOverviewController::onChannelsUpdated(){
    activeChannelsIdxs = appStatus->getSelectedChannelsIndexes();
    modm->setActiveChannelsIdxs(activeChannelsIdxs);
    modw->updateActiveChannels(activeChannelsIdxs);
}

void MeasurementOverviewController::getNewActiveChannels(std::vector <int>& newActiveChannels){
    newActiveChannels.clear();
    std::vector <bool> selectedChannels = appStatus->getSelectedChannels();
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        if (selectedChannels[channelIdx]){
            newActiveChannels.push_back(channelIdx);
        }
    }
}

void MeasurementOverviewController::onLiveStatisticsResults(StatisticsResultWrapper result){
    const auto r = result.results;
    modm->setStatisticsResult(r);
    modw->onLiveStatisticsResult(r);
}

void MeasurementOverviewController::onResistanceEstimationResults(SingleMeasResultWrapper_t result){
    const auto r = result.results;
    modm->setResistanceEstimationResult(r);
    modw->onResistanceEstimationResult(r);
}

std::vector <DeviceDataConsumer*> MeasurementOverviewController::getConsumers() {
    return {liveStatisticsConsumer, resistanceEstimationConsumer};
}

void MeasurementOverviewController::boardMappingsLoaded(){
    modw->updateActiveChannels(appStatus->getSelectedChannelsIndexes());
    onChannelsUpdated();
}
