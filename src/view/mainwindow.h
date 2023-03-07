#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QComboBox>
#include <QPushButton>

#include "modeldevice.h"
#include "e384commlib_errorcodes.h"

namespace e384cl = e384CommLib;

using namespace std;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setModelDevice(ModelDevice * modelDevice);
    QPushButton * getConnectButton();
    QString getSelectedSerialNumber();

public slots:
    void onDevicesListChanged(vector <string> devicesList);
    void onSetConnectedDeviceIdx(int idx);
    void onConnect(bool flag, e384cl::ErrorCodes_t err);

private:
    void createGuiControls();

    ModelDevice * mDev = nullptr;
    QMenu * menuView = nullptr;

    QDockWidget * deviceDetectorDw = nullptr;

//    DeviceDataProducer * deviceDataProducer = nullptr;

    QComboBox * devicesComboBox = nullptr;
    QPushButton * connectBtn = nullptr;

    uint16_t voltageChannelsNum = 1;
    uint16_t currentChannelsNum = 1;
    uint16_t totalChannelsNum = voltageChannelsNum+currentChannelsNum;

    QVector <QDockWidget *> dockWidgets;

//    PlotTab * plotTab = nullptr;

signals:
    void widgetsCreated();

};
#endif // MAINWINDOW_H
