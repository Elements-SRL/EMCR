#ifndef MEASUREMENTOVERVIEWCONTROLLER_H
#define MEASUREMENTOVERVIEWCONTROLLER_H

#include <QObject>

#include "mainwindow.h"
#include "measurementsoverviewdockwidget.h"
#include "application_status.h"
#include "measurementoverviewmodel.h"
#include "livestatisticsconsumer.h"


class MeasurementOverviewController : public QObject {
    Q_OBJECT

public:
    MeasurementOverviewController(ApplicationStatus * appStatus, DeviceDataProducer * producer, MainWindow * mainWindow);
    ~MeasurementOverviewController();
    LiveStatisticsConsumer * getLiveStatisticsConsumer();
    void boardMappingsLoaded();
public slots:
    void onChannelsUpdated();
    void onExportLiveNoiseEstimates();
    void onLiquidJunctionResult(bool started);
    void onLiveStatisticsResults(StatisticsResult * result);
private:
    void getNewActiveChannels(std::vector <int>& newActiveChannels);
    ApplicationStatus * appStatus = nullptr;
    MainWindow * mainWindow = nullptr;
    MeasurementsOverviewDockWidget * modw = nullptr;
    MeasurementOverviewModel * modm = nullptr;
    int voltageChannelsNum;
    int currentChannelsNum;
    std::vector<int> activeChannelsIdxs;
    LiveStatisticsConsumer * liveStatisticsConsumer = nullptr;
    void onSetConsumerStatus(bool status);
};

#endif // MEASUREMENTOVERVIEWCONTROLLER_H
