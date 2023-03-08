#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QComboBox>
#include <QPushButton>

#include "modeldevice.h"
#include "chessboard.h"
#include "devicecontroldockwidget.h"
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
    Chessboard * getChessaboard();
    DeviceControlDockWidget * getControlsDockWidget();

public slots:
    void onDevicesListChanged(vector <string> devicesList);
    void onSetConnectedDeviceIdx(int idx);
    void onConnect(bool flag, e384cl::ErrorCodes_t err);

private:
    void createGuiControls();
    void destroyGuiControls();
    void restoreUISettings();
    void saveUISettings();

    ModelDevice * mDev = nullptr;
    QMenu * menuView = nullptr;

    QDockWidget * deviceDetectorDw = nullptr;
    DeviceControlDockWidget * controlsDw = nullptr;

//    DeviceDataProducer * deviceDataProducer = nullptr;

    QComboBox * devicesComboBox = nullptr;
    QPushButton * connectBtn = nullptr;

    int voltageChannelsNum = 1;
    int currentChannelsNum = 1;
    int totalChannelsNum = voltageChannelsNum+currentChannelsNum;

    QVector <QDockWidget *> dockWidgets;
    QVector <QDockWidget *> analysisWidgets;

    Chessboard * chessboard = nullptr;

signals:
    void widgetsCreated();
    void widgetsDestroyed();

};
#endif // MAINWINDOW_H
