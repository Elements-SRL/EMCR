#ifndef MEASUREMENTOVERVIEWCONTROLLER_H
#define MEASUREMENTOVERVIEWCONTROLLER_H

#include <QObject>

#include "mainwindow.h"
#include "measurementsoverviewdockwidget.h"
#include "messagedispatcher.h"
#include "measurementoverviewmodel.h"
class MeasurementOverviewController : public QObject {
    Q_OBJECT

public:
    MeasurementOverviewController(MessageDispatcher * msgDisp, MainWindow * mainWindow);

public slots:
    void onChannelsUpdated();

    void onExportLiveNoiseEstimates();
    void onLiquidJunctionResult(bool started);
    void onLiveStatisticsResults(StatisticsResult * result);

private:
    void getNewActiveChannels(std::vector <int>& newActiveChannels);

    MessageDispatcher * msgDisp = nullptr;
    MainWindow * mainWindow = nullptr;
    MeasurementsOverviewDockWidget * modw = nullptr;
    MeasurementOverviewModel * modm = nullptr;

    int voltageChannelsNum;
    int currentChannelsNum;
    std::vector<int> activeChannelsIdxs;
};

#endif // MEASUREMENTOVERVIEWCONTROLLER_H
