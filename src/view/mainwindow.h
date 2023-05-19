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
#include "channelcontroldockwidget.h"
#include "boardcontroldockwidget.h"
#include "bigplotdockwidget.h"
#include "recordsettingsdialog.h"
#include "protocoldockwidget.h"
#include "e384commlib_errorcodes.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setModelDevice(ModelDevice * modelDevice);
    QPushButton * getConnectButton();
    QString getSelectedSerialNumber();
    Chessboard * getChessaboard();
    BigPlotDockWidget * getBigPlotWidget();
    DeviceControlDockWidget * getDeviceControlsDockWidget();
    ChannelControlDockWidget * getChannelControlsDockWidget();
    BoardControlDockWidget * getBoardControlsDockWidget();
    ProtocolDockWidget * getProtocolDockWidget();
    RecordSettingsDialog * getRecordSettingsDialog();

    QLabel * SRLbl = nullptr;

public slots:
    void onDevicesListChanged(std::vector <std::string> devicesList);
    void onSetConnectedDeviceIdx(int idx);
    void onConnect(bool flag, ErrorCodes_t err);
    void onCalibLoadingMsg(QString msg);
    void onManualCalibDoneMsg(QString msg);
    void onNeedToChangeModelCellMsg(QString msg);
    void onNeedToCheckFirstModelCellMsg(QString msg);

private:
    void createGuiControls();
    void destroyGuiControls();
    void addViewActions();
    void removeViewActions();
    void restoreUISettings();
    void saveUISettings();

    ModelDevice * mDev = nullptr;
    QMenu * menuView = nullptr;
    QMenu * menuRecordings = nullptr;

    bool interfaceCreated = false;

    QAction * actionRecordingSettings = nullptr;

    QDockWidget * deviceDetectorDw = nullptr;
    DeviceControlDockWidget * deviceControlsDw = nullptr;
    ChannelControlDockWidget * channelControlsDw = nullptr;
    BoardControlDockWidget * boardControlsDw = nullptr;
    BigPlotDockWidget * bigPlotDw = nullptr;
    ProtocolDockWidget * protocolDw = nullptr;
    RecordSettingsDialog * recordSettingsDialog = nullptr;

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
    void setDebugBit(int word, int bit, bool flag);
    void setDebugWord(int word, int value);
    void debugInitialization();
    void sigPerformCalibration(std::vector<std::uint16_t> channelsToCalibrateIdxs);
    void sigModelCellChanged(bool modelCellChanged);
    void sigFirstModelMounted(bool modelCellChanged);
};
#endif // MAINWINDOW_H
