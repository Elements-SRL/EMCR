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
    ~MeasurementOverviewController();
public slots:
    void onChannelsUpdated();
    void onExportLiveNoiseEstimates();
    void onLiquidJunctionResult(bool started);
private:
    void getNewActiveChannels(std::vector <int>& newActiveChannels);
    MessageDispatcher * msgDisp = nullptr;
    MeasurementsOverviewDockWidget * modw = nullptr;
    int voltageChannelsNum;
    int currentChannelsNum;
    std::vector<int> activeChannelsIdxs;

signals:
    void sigLiveStatisticsResult(StatisticsResult * result);
};

#endif // MEASUREMENTOVERVIEWCONTROLLER_H
