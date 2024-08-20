#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QThread>

#include "mainwindow.h"
#include "devicedetector.h"
#include "deviceconnector.h"
#include "upgradefwcontroller.h"
#include "bigplotcontroller.h"
#include "chessboardcontroller.h"
#include "singlechannelcontroller.h"
#include "multiplechannelcontroller.h"
#include "boardcontroller.h"
#include "devicecontroller.h"
#include "measurementoverviewcontroller.h"
#include "devicedataproducer.h"
#include "plotconsumer.h"
#include "livestatisticsconsumer.h"
#include "protocolmanager.h"
#include "statearraycontroller.h"
#include "compensationcontroller.h"
#include "plotpreferencescontroller.h"


class MainController : public QObject {
    Q_OBJECT

public:
    MainController();
    ~MainController();

    void setMainWindow(MainWindow * mainWindow);

public slots:
    void onDevicesListChanged(std::vector <std::string> devicesList);
    void onConnect(bool flag);
    void onUpgradeFw();
    void onResetHw();
    void onDeviceConnected(ErrorCodes_t ret);
    void onMainWindowCreated();
    void onVcCurrentRangeSelected(int idx);
    void onVcVoltageRangeSelected(int idx);
    void onCcCurrentRangeSelected(int idx);
    void onCcVoltageRangeSelected(int idx);
    void onVcVoltageFilterSelected(int idx);
    void onCcCurrentFilterSelected(int idx);
    void onSamplingRateSelected(int idx);
    void onDownsamplingRatioSelected(int idx);
    void onClampingModalitySelected(ClampingModality_t mode);

private:
    void startProducerConsumers();
    void stopAndDestroyProducerConsumers();
    void destroyControllers();

    MessageDispatcher * msgDisp = nullptr;
    ApplicationStatus * appStatus = nullptr;
    MainWindow * mainWindow = nullptr;

    DeviceDetector * deviceDetector = nullptr;
    QThread deviceDetectorThread;

    DeviceConnector * deviceConnector = nullptr;

    UpgradeFwController * upgradeFwController = nullptr;

    DeviceDataProducer * deviceDataProducer = nullptr;
    LiveStatisticsConsumer * liveStatisticsConsumer = nullptr;

    QVector <DeviceDataConsumer*> consumers;
    //TODO the following will become a list of Controllers, a controller will abstract away
    //all the slot and signals a common controller will have to be able to respond to 
    //(onSamplingRateChanged, onDownSamplingRateChanged, ecc.)
    std::vector<CentralWidgetController*> centralWidgetControllers;

    BigPlotController * bigPlotController = nullptr;
    ChessboardController * chessboardController = nullptr;
    SingleChannelController * singleChannelController = nullptr;
    MultipleChannelController * multipleChannelController = nullptr;
    BoardController * boardController = nullptr;
    DeviceController * deviceController = nullptr;
    StateArrayController * stateArrayController = nullptr;
    CompensationController * compensationController = nullptr;
    MeasurementOverviewController * measurementOverviewController = nullptr;
    PlotPreferencesController * plotPreferencesController = nullptr;

    ProtocolManager * voltageProtocolManager = nullptr;
    ProtocolManager * currentProtocolManager = nullptr;

    int voltageChannelsNumber;
    int currentChannelsNumber;
    int boardsNumber;
    //for now we are using only the previousVcVoltageRange
    RangedMeasurement * previousVcVoltageRange = nullptr;
    //RangedMeasurement * previousVcCurrentRange = nullptr;
    //RangedMeasurement * previousCcVoltageRange = nullptr;
    //RangedMeasurement * previousCcCurrentRange = nullptr;

signals:
    void startDetecting();
    void stopDetecting();
    void devicesListChanged(std::vector <std::string> devicesList);
    void setConnectedDeviceIdx(int connectedDeviceIdx);
    void connectDevice(bool flag, ErrorCodes_t err);
};

#endif // MAINCONTROLLER_H
