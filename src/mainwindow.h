#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QComboBox>
#include <QPushButton>
#include <QThread>

#include "devicedetector.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:

    QMenu * menuView = nullptr;

    QDockWidget * deviceDetectorDw = nullptr;

    DeviceDetector * deviceDetector = nullptr;
//    DeviceDataProducer * deviceDataProducer = nullptr;
    bool deviceConnected = false;

    QThread deviceDetectorThread;

    QComboBox * devicesComboBox = nullptr;
    QPushButton * connectBtn = nullptr;

    MessageDispatcher * messageDispatcher = nullptr;

public slots:
    void onDevicesListChanged(vector <string> devicesList);

signals:
    void startDetecting();
    void stopDetecting();
};
#endif // MAINWINDOW_H
