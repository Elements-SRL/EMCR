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
#include "plotconsumer.h"

class ControllerMain : public QObject {
    Q_OBJECT

public:
    ControllerMain();
    ~ControllerMain();

    void setMainWindow(MainWindow * mainWindow);

private:
    ModelDevice * mDev = nullptr;

    MainWindow * mainWindow = nullptr;

    DeviceDetector * deviceDetector = nullptr;
    QThread deviceDetectorThread;

    DeviceDataProducer * deviceDataProducer = nullptr;
    GapFreePlotConsumer * stampPlotConsumer = nullptr;

    ControllerChannel * controllerChannel = nullptr;
    ControllerBoard * controllerBoard = nullptr;
    ControllerDevice * controllerDevice = nullptr;


public slots:
    void onDevicesListChanged(vector <string> devicesList);
    void onConnect(bool flag);
    void onMainWindowCreated();
    void onMainWindowDestroyed();

signals:
    void startDetecting();
    void stopDetecting();
    void devicesListChanged(vector <string> devicesList);
    void setConnectedDeviceIdx(int connectedDeviceIdx);
    void connectDevice(bool flag, ErrorCodes_t err);
};

#endif // CONTROLLERMAIN_H
