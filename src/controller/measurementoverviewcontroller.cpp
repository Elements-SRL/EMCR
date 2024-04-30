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
    connect(liveStatisticsConsumer, &LiveStatisticsConsumer::sigResult, this, &MeasurementOverviewController::onLiveStatisticsResults);
    connect(modw, &QDockWidget::visibilityChanged, this, &MeasurementOverviewController::onSetConsumerStatus);

    mainWindow->setMeasurementOverviewDw(modw);    
}

void MeasurementOverviewController::onSetConsumerStatus(bool status) {
    if (status) {
        liveStatisticsConsumer->onStartConsuming();
    } else {
        liveStatisticsConsumer->onStopConsuming();
    }
}

MeasurementOverviewController::~MeasurementOverviewController(){
    delete modw;
    modw = nullptr;
    mainWindow->setMeasurementOverviewDw(modw);
    if (liveStatisticsConsumer!= nullptr) {
        liveStatisticsConsumer->onStopConsuming();
        delete liveStatisticsConsumer;
        liveStatisticsConsumer = nullptr;
    }
}

void MeasurementOverviewController::onLiquidJunctionResult(bool started) {
    if (!started) {
        std::vector <uint16_t> channelIdxs(currentChannelsNum);
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            channelIdxs[idx] = idx;
        }

        std::vector <Measurement_t> stdVoltages;
        auto msgDisp = appStatus->getMessageDispatcher();
        msgDisp->getLiquidJunctionVoltages(channelIdxs, stdVoltages);
        std::vector <Measurement_t> voltages(currentChannelsNum);

        for (int idx = 0; idx < currentChannelsNum; idx++) {
            voltages[idx] = stdVoltages[idx];
        }
        modm->setLiquidJunctionResults(voltages);
        modw->setLiquidJunctionResult(voltages);
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

LiveStatisticsConsumer * MeasurementOverviewController::getLiveStatisticsConsumer(){
    return liveStatisticsConsumer;
}

void MeasurementOverviewController::boardMappingsLoaded(){
    modw->updateActiveChannels(appStatus->getSelectedChannelsIndexes());
    onChannelsUpdated();
}
