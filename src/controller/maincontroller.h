#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QThread>

#include "modeldevice.h"
#include "mainwindow.h"
#include "devicedetector.h"
#include "controllerchannel.h"

class MainController : public QObject {
    Q_OBJECT

public:
    MainController();
    ~MainController();

    void setMainWindow(MainWindow * mainWindow);

private:
    ModelDevice * mDev = nullptr;

    MainWindow * mainWindow = nullptr;

    DeviceDetector * deviceDetector = nullptr;
    QThread deviceDetectorThread;

    ControllerChannel * controllerChannel = nullptr;

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
    void connectDevice(bool flag, e384cl::ErrorCodes_t err);
};

#endif // MAINCONTROLLER_H
