#include "measurementoverviewcontroller.h"

MeasurementOverviewController::MeasurementOverviewController(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp),
    mainWindow(mainWindow) {

    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);
    this->getNewActiveChannels(activeChannelsIdxs);
    modw = new MeasurementsOverviewDockWidget(activeChannelsIdxs, voltageChannelsNum, currentChannelsNum);

    connect(this, &MeasurementOverviewController::sigLiveStatisticsResult, modw, &MeasurementsOverviewDockWidget::onLiveStatisticsResult);
    connect(this, &MeasurementOverviewController::sigLiquidJunctionResult, modw, &MeasurementsOverviewDockWidget::onLiquidJunctionResult);

    mainWindow->setMeasurementOverviewDw(modw);
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

void MeasurementOverviewController::onSingleChannelClicked(uint16_t, bool){
    this->getNewActiveChannels(activeChannelsIdxs);
    modw->updateActiveChannels(activeChannelsIdxs);
}

void MeasurementOverviewController::onOneBoardClicked(uint16_t, bool) {
    this->getNewActiveChannels(activeChannelsIdxs);
    modw->updateActiveChannels(activeChannelsIdxs);
}

void MeasurementOverviewController::onOneRowClicked(uint16_t, bool) {
    this->getNewActiveChannels(activeChannelsIdxs);
    modw->updateActiveChannels(activeChannelsIdxs);
}

void MeasurementOverviewController::onAllChannelsClicked(bool) {
    this->getNewActiveChannels(activeChannelsIdxs);
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
