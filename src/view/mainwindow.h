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
#include "ivgraphwidget.h"
#include "spectrumwidget.h"

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
    void setConnectionLabel(QString text);

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
    void onNeedToChangeModelCellMsg(QString msg);
    void onBoardMappingPressed();
    void onAbout();
    void onDeviceInfo();
    void onSupport();

private:
    void createGuiControls();
    void destroyGuiControls();
    void restoreUISettings();
    void saveUISettings();

    MessageDispatcher * msgDisp = nullptr;
    QMenu * menuView = nullptr;
    QMenu * menuRecordings = nullptr;
    QMenu * menuPreferences = nullptr;
    QMenu * menuAdvanced = nullptr;
    QMenu * menuHwReset = nullptr;
    QMenu * menuQuestionMark = nullptr;

    bool interfaceCreated = false;

    QAction * actionRecordingSettings = nullptr;

    QAction * actionPlotPreferences = nullptr;
    QAction * actionBoardMapping = nullptr;

    QAction * actionUpgradeFw = nullptr;
    QAction * actionHwReset = nullptr;
    QAction * actionHwResetHelp = nullptr;

    QAction * actionAbout = nullptr;
    QAction* actionDeviceInfo = nullptr;
    QAction* actionSupport = nullptr;

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
    IvGraphWidget * ivGraphWidget = nullptr;
    SpectrumWidget * spectrumWidget = nullptr;
//    BoardMappingDialog * boardMappingDialog= nullptr;
    QDockWidget * debugDw = nullptr;

    QComboBox * devicesComboBox = nullptr;
    QPushButton * connectBtn = nullptr;
    QLabel * connectionInfoLbl;

    int voltageChannelsNum = 1;
    int currentChannelsNum = 1;
    int totalChannelsNum = voltageChannelsNum+currentChannelsNum;

    QVector <QDockWidget *> dockWidgets;
    QVector <QDockWidget *> analysisWidgets;

private slots:
    void onResetHwHelp();

signals:
    void setDebugBit(int word, int bit, bool flag);
    void setDebugWord(int word, int value);
    void sigModelCellChanged(bool modelCellChanged);
    void sigBoardMappingFileChoosen(QString filename);
    void sigUpgradeFw();
    void sigResetHw();
};
#endif // MAINWINDOW_H
