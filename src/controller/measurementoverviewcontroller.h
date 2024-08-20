#ifndef MEASUREMENTOVERVIEWCONTROLLER_H
#define MEASUREMENTOVERVIEWCONTROLLER_H

#include <QObject>

#include "mainwindow.h"
#include "measurementsoverviewdockwidget.h"
#include "application_status.h"
#include "measurementoverviewmodel.h"
#include "livestatisticsconsumer.h"
#include "statisticsresultwrapper.h"

class MeasurementOverviewController : public QObject {
    Q_OBJECT

public:
    MeasurementOverviewController(ApplicationStatus * appStatus, DeviceDataProducer * producer, MainWindow * mainWindow);
    ~MeasurementOverviewController();
    LiveStatisticsConsumer * getLiveStatisticsConsumer();
    void boardMappingsLoaded();

public slots:
    void onChannelsUpdated();
    void onOffsetRecalibrationResult(bool started);
    void onLiquidJunctionResult(bool started);

private:
    void getNewActiveChannels(std::vector <int>& newActiveChannels);
    ApplicationStatus * appStatus = nullptr;
    MainWindow * mainWindow = nullptr;
    MeasurementsOverviewDockWidget * modw = nullptr;
    MeasurementOverviewModel * modm = nullptr;
    int voltageChannelsNum;
    int currentChannelsNum;
    std::vector<uint16_t> activeChannelsIdxs;
    LiveStatisticsConsumer * liveStatisticsConsumer = nullptr;
    void onSetConsumerStatus(bool status);

private slots:
    void onLiveStatisticsResults(StatisticsResultWrapper);
};

#endif // MEASUREMENTOVERVIEWCONTROLLER_H
