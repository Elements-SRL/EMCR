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
    MainWindow(QWidget * parent = nullptr);
    ~MainWindow();

    typedef enum DockWidgets {
        DWDeviceDetector,
        DWChessboard,
        DWDeviceControl,
        DWSingleChannelControl,
        DWMultipleChannelControl,
        DWBoardControl,
        DWProtocol,
        DWCompensationControl,
        DWStateArray,
        DWMeasurementsOverview,
        DWAutoDeclogger,
        DWTemperatureSensors,
        DWDebug,
        DockWidgetsNum
    } DockWidgets_t;

    void setMessageDispatcher(MessageDispatcher * msgDisp);
    QPushButton * getConnectButton();
    QString getSelectedSerialNumber();
    BigPlotWidget * getBigPlotWidget();
    QDockWidget * getDockWidget(DockWidgets_t type);
    RecordSettingsDialog * getRecordSettingsDialog();
    PlotPreferencesDialog * getPlotPreferencesDialog();
    void setDevicesList(std::vector <std::string> devicesList);
    void setConnectedDeviceIdx(int idx);
    void connectDevice(bool flag, ErrorCodes_t err);
    void setConnectionLabel(QString text);

    void setBigPlotWidget(BigPlotWidget * widget);
    void setDockWidget(DockWidgets_t type, QDockWidget * widget, bool floatingFlag = true, Qt::DockWidgetArea area = Qt::RightDockWidgetArea);
    void setPlotPreferencesDialog(PlotPreferencesDialog * widget);
    void addViewActions();
    void removeViewActions();
    void restoreUISettings();
    void saveUISettings();

public slots:
    void onNeedToChangeModelCellMsg(QString msg);
    void onBoardMappingPressed();
    void onBitRateComputed(double bitRate);
    void onOnTimeRead(Measurement_t onTime);

private:
    typedef enum {
        ResetHwHelpDlg,
        AboutDlg,
        DeviceInfoDlg,
        SupportDlg,
        ReleaseNotesDlg
    } Dialogs_t;

    void createGuiControls();
    void destroyGuiControls();

    MessageDispatcher * msgDisp = nullptr;
    QMenu * menuView = nullptr;
    QMenu * menuRecordings = nullptr;
    QMenu * menuPreferences = nullptr;
    QMenu * menuAdvanced = nullptr;
    QMenu * menuHwReset = nullptr;
    QMenu * menuQuestionMark = nullptr;

    bool interfaceCreated = false;

    QAction * actionRearrangeView = nullptr;
    QAction * actionRecordingSettings = nullptr;

    QAction * actionPlotPreferences = nullptr;
    QAction * actionBoardMapping = nullptr;

    QAction * actionUpgradeFw = nullptr;
    QAction * actionHwReset = nullptr;
    QAction * actionHwResetHelp = nullptr;

    QAction * actionAbout = nullptr;
    QAction * actionDeviceInfo = nullptr;
    QAction * actionSupport = nullptr;
    QAction * actionReleaseNotes = nullptr;

    QLabel * SRLbl = nullptr;
    QLabel * onTimeLbl = nullptr;

    BigPlotWidget * bigPlotW = nullptr;
    RecordSettingsDialog * recordSettingsDialog = nullptr;
    PlotPreferencesDialog * plotPreferencesDlg = nullptr;
    SpectrumWidget * spectrumWidget = nullptr;

    QComboBox * devicesComboBox = nullptr;
    QPushButton * connectBtn = nullptr;
    QLabel * connectionInfoLbl;

    int voltageChannelsNum = 1;
    int currentChannelsNum = 1;
    int totalChannelsNum = voltageChannelsNum+currentChannelsNum;

    QVector <QDockWidget *> dockWidgets;

private slots:
    void onOpenDialog(Dialogs_t type);
    void onRearrangeView();

signals:
    void sigModelCellChanged(bool modelCellChanged);
    void sigBoardMappingFileChoosen(QString filename);
    void sigUpgradeFw();
    void sigResetHw();
};
#endif // MAINWINDOW_H
