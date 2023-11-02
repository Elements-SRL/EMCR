#include "measurementoverviewcontroller.h"
#include <iostream>

MeasurementOverviewController::MeasurementOverviewController(MessageDispatcher * msgDisp, DeviceDataProducer * producer, MainWindow * mainWindow) :
    msgDisp(msgDisp),
    mainWindow(mainWindow) {

    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);
    this->getNewActiveChannels(activeChannelsIdxs);
    modw = new MeasurementsOverviewDockWidget(activeChannelsIdxs, voltageChannelsNum, currentChannelsNum);
    modm = new MeasurementOverviewModel(activeChannelsIdxs, voltageChannelsNum, currentChannelsNum);
    connect(modw, &MeasurementsOverviewDockWidget::extract, this, [=](QString filepath){
        modm->exportToCsv(filepath.toStdString());
    });
    liveStatisticsConsumer = new LiveStatisticsConsumer(msgDisp, producer);
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

void MeasurementOverviewController::onExportLiveNoiseEstimates() {
    QString filename = "noise";
    QString filedir = QDir::currentPath() + "/";
    QString filepath = filedir + filename + ".csv";
    while (QFile::exists(filepath)) {
        filename += "_";
        filepath = filedir + filename + ".csv";
    }

    QFile file(filepath);
    file.open(QIODevice::WriteOnly);

    QTextStream stream(&file);
//    for (auto noise : res->stdCurrent) {
//        stream << noise << "\n";
//    }
    file.close();
}

void MeasurementOverviewController::onLiquidJunctionResult(bool started) {
    if (!started) {
        std::vector <uint16_t> channelIdxs(currentChannelsNum);
        for (int idx = 0; idx < currentChannelsNum; idx++) {
            channelIdxs[idx] = idx;
        }

        std::vector <Measurement_t> stdVoltages;
        msgDisp->getLiquidJunctionVoltages(channelIdxs, stdVoltages);
        QVector <Measurement_t> voltages(currentChannelsNum);

        for (int idx = 0; idx < currentChannelsNum; idx++) {
            voltages[idx] = stdVoltages[idx];
        }

        modw->setLiquidJunctionResult(voltages);
    }
}

void MeasurementOverviewController::onChannelsUpdated(){
    this->getNewActiveChannels(activeChannelsIdxs);
    modm->setActiveChannelsIdxs(activeChannelsIdxs);
    modw->updateActiveChannels(activeChannelsIdxs);
}

void MeasurementOverviewController::getNewActiveChannels(std::vector <int>& newActiveChannels){
    newActiveChannels.clear();
    std::vector <bool> selectedChannels;
    msgDisp->getSelectedChannels(selectedChannels);
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        if (selectedChannels[channelIdx]){
            newActiveChannels.push_back(channelIdx);
        }
    }
}

void MeasurementOverviewController::onLiveStatisticsResults(StatisticsResult * result){
    modm->setStatisticsResult(result);
    modw->onLiveStatisticsResult(result);
}

LiveStatisticsConsumer * MeasurementOverviewController::getLiveStatisticsConsumer(){
    return liveStatisticsConsumer;
}

