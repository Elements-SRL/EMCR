#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDockWidget>
#include <QMenu>
#include <QMenuBar>
#include <QComboBox>
#include <QPushButton>

#include "messagedispatcher.h"
#include "chessboarddockwidget.h"
#include "devicecontroldockwidget.h"
#include "singlechannelcontroldockwidget.h"
#include "multiplechannelcontroldockwidget.h"
#include "boardcontroldockwidget.h"
#include "bigplotwidget.h"
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
    BigPlotWidget * getBigPlotWidget();
    ChessboardDockWidget * getChessboardDockWidget();
    DeviceControlDockWidget * getDeviceControlsDockWidget();
    SingleChannelControlDockWidget * getSingleChannelControlsDockWidget();
    MultipleChannelControlDockWidget * getMultipleChannelControlsDockWidget();
    BoardControlDockWidget * getBoardControlsDockWidget();
    ProtocolDockWidget * getProtocolDockWidget();
    RecordSettingsDialog * getRecordSettingsDialog();
    CompensationControlDockWidget * getCompensationControlsDockWidget();
    StateArrayDockWidget * getStateArrayDockWidget();
    void setDevicesList(std::vector <std::string> devicesList);
    void setConnectedDeviceIdx(int idx);
    void connectDevice(bool flag, ErrorCodes_t err);

    void setBigPlotWidget(BigPlotWidget * bpw);
    void setChessboardDw(ChessboardDockWidget * cbdw);
    void setCompensationControlsDw(CompensationControlDockWidget * ccdw);
    void setSingleChannelControlsDw(SingleChannelControlDockWidget * scdw);
    void setMultipleChannelControlsDw(MultipleChannelControlDockWidget * mcdw);
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
    BigPlotWidget * bigPlotW = nullptr;
    ChessboardDockWidget * chessboardDw = nullptr;
    DeviceControlDockWidget * deviceControlsDw = nullptr;
    SingleChannelControlDockWidget * singleChannelControlsDw = nullptr;
    MultipleChannelControlDockWidget * multipleChannelControlsDw = nullptr;
    BoardControlDockWidget * boardControlsDw = nullptr;
    ProtocolDockWidget * protocolDw = nullptr;
    RecordSettingsDialog * recordSettingsDialog = nullptr;
    CompensationControlDockWidget * compensationControlsDw = nullptr;
    StateArrayDockWidget * stateArrayDockWidget = nullptr;

    QComboBox * devicesComboBox = nullptr;
    QPushButton * connectBtn = nullptr;

    int voltageChannelsNum = 1;
    int currentChannelsNum = 1;
    int totalChannelsNum = voltageChannelsNum+currentChannelsNum;

    QVector <QDockWidget *> dockWidgets;
    QVector <QDockWidget *> analysisWidgets;

signals:
    void setDebugBit(int word, int bit, bool flag);
    void setDebugWord(int word, int value);
    void debugInitialization();
    void sigPerformCalibration(std::vector<std::uint16_t> channelsToCalibrateIdxs);
    void sigModelCellChanged(bool modelCellChanged);
};
#endif // MAINWINDOW_H
