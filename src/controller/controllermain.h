#ifndef CONTROLLERMAIN_H
#define CONTROLLERMAIN_H

#include <QObject>
#include <QThread>

#include "modeldevice.h"
#include "mainwindow.h"
#include "devicedetector.h"
#include "controllerchannel.h"
#include "controllerboard.h"
#include "controllerdevice.h"
#include "devicedataproducer.h"
#include "abfdatawriterconsumer.h"
#include "plotconsumer.h"
#include "livenoiseconsumer.h"
#include "calibrationconsumer.h"
#include "protocolmanager.h"
#include "controllerstatearray.h"
#include "controllercompensation.h"

class ControllerMain : public QObject {
    Q_OBJECT

public:
    ControllerMain();
    ~ControllerMain();

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
    void onStartRecording(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues);
    void onStopRecording();


private:
    void startProducerConsumers();
    void stopAndDestroyProducerConsumers();

    ModelDevice * mDev = nullptr;

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

    ControllerChannel * controllerChannel = nullptr;
    ControllerBoard * controllerBoard = nullptr;
    ControllerDevice * controllerDevice = nullptr;
    ControllerStateArray * controllerStateArray;
    ControllerCompensation * controllerCompensation;

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

#endif // CONTROLLERMAIN_H
