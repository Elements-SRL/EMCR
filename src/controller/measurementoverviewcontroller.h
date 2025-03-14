#ifndef MEASUREMENTOVERVIEWCONTROLLER_H
#define MEASUREMENTOVERVIEWCONTROLLER_H

#include <QObject>

#include "controllerwithconsumer.h"
#include "mainwindow.h"
#include "measurementsoverviewdockwidget.h"
#include "application_status.h"
#include "measurementoverviewmodel.h"
#include "livestatisticsconsumer.h"
#include "resultwrapper.h"
#include "resistanceestimationconsumer.h"
#include "pipettecapacitanceestimationconsumer.h"
#include "membraneestimationconsumer.h"
#include "resultwrapper.h"

class MeasurementOverviewController : public ControllerWithConsumer {
    Q_OBJECT

public:
    MeasurementOverviewController(ApplicationStatus * appStatus, DeviceDataProducer * producer, MainWindow * mainWindow);
    ~MeasurementOverviewController();
    virtual std::vector <DeviceDataConsumer*> getConsumers() override;
    void boardMappingsLoaded();

public slots:
    void onChannelsUpdated();
    void onOffsetRecalibrationResult(bool started);
    void onLiquidJunctionResult(bool started);
    void onProtocolStarted(unsigned int protId, ProtocolWidget * protocol);

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
    ResistanceEstimationConsumer * resistanceEstimationConsumer = nullptr;
    PipetteCapacitanceEstimationConsumer * pipetteCapacitanceEstimationConsumer = nullptr;
    MembraneEstimationConsumer * membraneEstimationConsumer = nullptr;
    void onSetLiveStatisticsConsumerStatus(bool status);

private slots:
    void onLiveStatisticsResults(StatisticsResultWrapper_t);
    void onResistanceEstimationResults(SingleMeasResultWrapper_t);
    void onPipetteCapacitanceEstimationResults(SingleMeasResultWrapper_t);
    void onMembraneEstimationResults(MembraneResultWrapper_t);
};

#endif // MEASUREMENTOVERVIEWCONTROLLER_H
