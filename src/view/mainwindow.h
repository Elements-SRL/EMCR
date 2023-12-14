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
#include "statearraydockwidget.h"
#include "measurementsoverviewdockwidget.h"
#include "plotpreferencesdialog.h"

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
    MeasurementsOverviewDockWidget * getMeasurementOverviewDockWidget();
    PlotPreferencesDialog * getPlotPreferencesDialog();
    void setDevicesList(std::vector <std::string> devicesList);
    void setConnectedDeviceIdx(int idx);
    void connectDevice(bool flag, ErrorCodes_t err);

    void setBigPlotWidget(BigPlotWidget * widget);
    void setChessboardDw(ChessboardDockWidget * widget);
    void setCompensationControlsDw(CompensationControlDockWidget * widget);
    void setSingleChannelControlsDw(SingleChannelControlDockWidget * widget);
    void setMultipleChannelControlsDw(MultipleChannelControlDockWidget * widget);
    void setBoardControlsDw(BoardControlDockWidget * widget);
    void setDeviceControlDw(DeviceControlDockWidget * widget);
    void setProtocolDw(ProtocolDockWidget * widget);
    void setStateArrayDw(StateArrayDockWidget * widget);
    void setMeasurementOverviewDw(MeasurementsOverviewDockWidget * widget);
    void setPlotPreferencesDialog(PlotPreferencesDialog * widget);
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
    QMenu * menuPreferences = nullptr;

    bool interfaceCreated = false;

    QAction * actionRecordingSettings = nullptr;

    QAction * actionPlotPreferences = nullptr;
    QAction * actionBoardMapping = nullptr;


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
    MeasurementsOverviewDockWidget * measurementsOverviewDw = nullptr;
    PlotPreferencesDialog * plotPreferencesDlg = nullptr;
//    BoardMappingDialog * boardMappingDialog= nullptr;
    QDockWidget * calibrationDw = nullptr;
    QDockWidget * debugDw = nullptr;

    QComboBox * devicesComboBox = nullptr;
    QPushButton * connectBtn = nullptr;

    int voltageChannelsNum = 1;
    int currentChannelsNum = 1;
    int totalChannelsNum = voltageChannelsNum+currentChannelsNum;

    QVector <QDockWidget *> dockWidgets;
    QVector <QDockWidget *> analysisWidgets;
    void onBoardMappingPressed();

signals:
    void setDebugBit(int word, int bit, bool flag);
    void setDebugWord(int word, int value);
    void sigPerformCalibration(std::vector<std::uint16_t> channelsToCalibrateIdxs);
    void sigModelCellChanged(bool modelCellChanged);
    void sigBoardMappingFileChoosen(QString filename);
};
#endif // MAINWINDOW_H
