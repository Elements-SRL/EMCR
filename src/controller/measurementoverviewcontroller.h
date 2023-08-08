#ifndef MEASUREMENTOVERVIEWCONTROLLER_H
#define MEASUREMENTOVERVIEWCONTROLLER_H

#include <QObject>

#include "mainwindow.h"
#include "measurementsoverviewdockwidget.h"
#include "messagedispatcher.h"

class MeasurementOverviewController : public QObject {
    Q_OBJECT

public:
    MeasurementOverviewController(MessageDispatcher * msgDisp, MainWindow * mainWindow);

public slots:
    void onSingleChannelClicked(uint16_t chIdx, bool newState);
    void onOneBoardClicked(uint16_t brdIdx, bool newState);
    void onOneRowClicked(uint16_t rowIdx, bool newState);
    void onAllChannelsClicked(bool newState);

    void onExportLiveNoiseEstimates();
    void onLiquidJunctionResult(bool started);

private:
    void getNewActiveChannels(std::vector <int>& newActiveChannels);

    MessageDispatcher * msgDisp = nullptr;
    MainWindow * mainWindow = nullptr;
    MeasurementsOverviewDockWidget * modw = nullptr;

    int voltageChannelsNum;
    int currentChannelsNum;
    std::vector<int> activeChannelsIdxs;

signals:
    void sigLiveStatisticsResult(StatisticsResult * result);
};

#endif // MEASUREMENTOVERVIEWCONTROLLER_H
