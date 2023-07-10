#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QThread>

#include "mainwindow.h"
#include "devicedetector.h"
#include "bigplotcontroller.h"
#include "chessboardcontroller.h"
#include "singlechannelcontroller.h"
#include "multiplechannelcontroller.h"
#include "boardcontroller.h"
#include "devicecontroller.h"
#include "devicedataproducer.h"
#include "abfdatawriterconsumer.h"
#include "plotconsumer.h"
#include "livenoiseconsumer.h"
#include "calibrationconsumer.h"
#include "protocolmanager.h"
#include "statearraycontroller.h"
#include "compensationcontroller.h"

class MainController : public QObject {
    Q_OBJECT

public:
    MainController();
    ~MainController();

    void setMainWindow(MainWindow * mainWindow);

public slots:
    void onDevicesListChanged(std::vector <std::string> devicesList);
    void onConnect(bool flag);
    void onMainWindowCreated();
    void onMainWindowDestroyed();
    void onVcCurrentRangeSelected(int idx);
    void onVcVoltageRangeSelected(int idx);
    void onCcCurrentRangeSelected(int idx);
    void onCcVoltageRangeSelected(int idx);
    void onVcVoltageFilterSelected(int idx);
    void onCcCurrentFilterSelected(int idx);
    void onSamplingRateSelected(int idx);
    void onDownsamplingRatioSelected(int idx);
    void onClampingModalitySelected(int idx);
    void onStartRecording();
    void onStopRecording();

private:
    void startProducerConsumers();
    void stopAndDestroyProducerConsumers();

    MessageDispatcher * msgDisp = nullptr;

    MainWindow * mainWindow = nullptr;

    DeviceDetector * deviceDetector = nullptr;
    QThread deviceDetectorThread;

    DeviceDataProducer * deviceDataProducer = nullptr;
    GapFreePlotConsumer * stampPlotConsumer = nullptr;
    GapFreePlotConsumer * bigPlotConsumer = nullptr;
    AbfDataWriterConsumer * abfDataWriterConsumer = nullptr;
    LiveNoiseConsumer * liveNoiseConsumer = nullptr;
    CalibrationConsumer * calibratorConsumer = nullptr;

    QVector <DeviceDataConsumer*> consumers;
    QVector <DataWriterConsumer*> dataWriterConsumers;

    BigPlotController * bigPlotController = nullptr;
    ChessboardController * chessboardController = nullptr;
    SingleChannelController * singleChannelController = nullptr;
    MultipleChannelController * multipleChannelController = nullptr;
    BoardController * boardController = nullptr;
    DeviceController * deviceController = nullptr;
    StateArrayController * stateArrayController;
    CompensationController * compensationController;

    ProtocolManager * voltageProtocolManager = nullptr;
    ProtocolManager * currentProtocolManager = nullptr;

    int voltageChannelsNumber;
    int currentChannelsNumber;
    int boardsNumber;

signals:
    void startDetecting();
    void stopDetecting();
    void devicesListChanged(std::vector <std::string> devicesList);
    void setConnectedDeviceIdx(int connectedDeviceIdx);
    void connectDevice(bool flag, ErrorCodes_t err);
};

#endif // MAINCONTROLLER_H
