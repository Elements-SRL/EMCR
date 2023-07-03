#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QComboBox>
#include <QPushButton>


#include "messagedispatcher.h"
#include "chessboard.h"
#include "devicecontroldockwidget.h"
#include "channelcontroldockwidget.h"
#include "boardcontroldockwidget.h"
#include "bigplotdockwidget.h"
#include "recordsettingsdialog.h"
#include "protocoldockwidget.h"
#include "e384commlib_errorcodes.h"
#include "compensationcontroldockwidget.h"
#include "statearray/statearraydockwidget.h"


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setMessageDispatcher(MessageDispatcher * msgDisp);
    QPushButton * getConnectButton();
    QString getSelectedSerialNumber();
    Chessboard * getChessaboard();
    BigPlotDockWidget * getBigPlotWidget();
    DeviceControlDockWidget * getDeviceControlsDockWidget();
    ChannelControlDockWidget * getChannelControlsDockWidget();
    BoardControlDockWidget * getBoardControlsDockWidget();
    ProtocolDockWidget * getProtocolDockWidget();
    RecordSettingsDialog * getRecordSettingsDialog();
    CompensationControlDockWidget * getCompensationControlsDockWidget();
    StateArrayDockWidget * getStateArrayDockWidget();
    void setDevicesList(std::vector <std::string> devicesList);
    void setConnectedDeviceIdx(int idx);
    void connectDevice(bool flag, ErrorCodes_t err);

    void setCompensationControlsDw(CompensationControlDockWidget * ccdw);
    void setChannelControlsDw(ChannelControlDockWidget * ccdw);
    void setBoardControlsDw(BoardControlDockWidget * bcdw);
    void setDeviceControlDw(DeviceControlDockWidget * dcdw);
    void setProtocolDw(ProtocolDockWidget * pdw);
    void setStateArrayDw(StateArrayDockWidget * sadw);
    void addViewActions();
    void removeViewActions();

    QLabel * SRLbl = nullptr;

public slots:
    void onCalibLoadingMsg(QString msg);
    void onCalibLoadingMsg(ErrorCodes_t error);
    void onManualCalibDoneMsg(QString msg);
    void onNeedToChangeModelCellMsg(QString msg);

private:
    void createGuiControls();
    void destroyGuiControls();
    void restoreUISettings();
    void saveUISettings();

    MessageDispatcher * msgDisp = nullptr;
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
    CompensationControlDockWidget * compensationControlsDw = nullptr;
    StateArrayDockWidget * stateArrayDockWidget = nullptr;
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
    void setDebugBit(int word, int bit, bool flag);
    void setDebugWord(int word, int value);
    void debugInitialization();
    void sigPerformCalibration(std::vector<std::uint16_t> channelsToCalibrateIdxs);
    void sigModelCellChanged(bool modelCellChanged);
};
#endif // MAINWINDOW_H
