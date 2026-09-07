#include "mainwindow.h"

#include <QBoxLayout>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QPushButton>

#include "confirmdialog/confirmationdialog.h"
#include "messagedispatcher.h"
#include "elementslogowidget.h"
#include "errormanager.h"
#include "globaldefines.h"
#include "qactiongroup.h"
#include "qmovie.h"
#include "qtoolbar.h"
#include "qwindow.h"
#include "resethwhelpdialog.h"
#include "aboutdialog.h"
#include "supportdialog.h"
#include "deviceinfodialog.h"
#include "releasenotesdialog.h"
#include "themecontroller.h"
#include <QDebug>

void MainWindow::setupDeviceConnectionGui(QFrame * container){

    QVBoxLayout *mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(40, 40, 40, 40);
    mainLayout->setSpacing(10);

    QLabel *logoLabel = new QLabel();
    logoLabel->setObjectName("elementsLogo");
    logoLabel->setPixmap(QPixmap(ThemeController::imgsPath() + "/logo_with_name.png").scaled(200, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    mainLayout->addWidget(logoLabel, 0, Qt::AlignTop | Qt::AlignRight);

    // --- STACKED WIDGET ---
    connectionDeviceStack = new QStackedWidget();

    // PAGE 0 - CONNECTING PAGE
    QFrame *connectionPage = new QFrame();
    connectionPage->setObjectName("ConnectionPage");
    QVBoxLayout *connectionLayout = new QVBoxLayout(connectionPage);
    connectionLayout->setSpacing(10);
    connectionLayout->setContentsMargins(0, 0, 0, 0);

    QLabel *titleLbl = new QLabel("DEVICES");
    titleLbl->setObjectName("selectionTitle");
    QLabel *hintLbl = new QLabel("Available hardware");
    hintLbl->setObjectName("splashSubtitle");

    devicesComboBox = new QComboBox();
    devicesComboBox->setObjectName("deviceCombo");
    devicesComboBox->setFixedWidth(220);

    connectionLayout->addWidget(titleLbl);
    connectionLayout->addWidget(hintLbl);
    connectionLayout->addWidget(devicesComboBox);

    connectBtn = new QPushButton("CONNECT");
    connectBtn->setObjectName("connectBtn");
    connectBtn->setFixedWidth(220);
    connectBtn->setCheckable(true);
    connectBtn->setCursor(Qt::PointingHandCursor);
    connectionLayout->addWidget(connectBtn, 0, Qt::AlignLeft);
    connectionLayout->addStretch();

    // PAGE 1: CONNECTED PAGE (Minimized widget)
    QFrame *connectedPage = new QFrame();
    connectedPage->setObjectName("connectedPage");
    QVBoxLayout *connectedPageLayout = new QVBoxLayout(connectedPage);
    QHBoxLayout *connectedRowLayout = new QHBoxLayout();
    QHBoxLayout *connectedTitleLayout = new QHBoxLayout();
    connectedRowLayout->setContentsMargins(0, 0, 0, 0);
    connectedPageLayout->setContentsMargins(0, 0, 0, 0);
    connectedPageLayout->addLayout(connectedTitleLayout);
    connectedPageLayout->addLayout(connectedRowLayout);
    connectedPageLayout->setSpacing(12);

    QLabel * deviceIco = new QLabel("");
    deviceIco->setObjectName("deviceIco");
    QLabel *devicesTitle = new QLabel("DEVICE");
    devicesTitle->setObjectName("deviceConnectedTitle");
    connectedTitleLayout->addWidget(deviceIco);
    connectedTitleLayout->addWidget(devicesTitle);
    connectedTitleLayout->addStretch();

    deviceConnectedLbl = new QLabel("");
    deviceConnectedLbl->setObjectName("deviceConnectedLbl");
    SRLbl = new QLabel("");
    SRLbl->setObjectName("connectionSpeed");
    // onTimeLbl = new QLabel("");
    // onTimeLbl->setObjectName("powerOnTime");
    // onTimeLbl->setVisible(false);

    disconnectBtn = new QPushButton("   DISCONNECT");
    disconnectBtn->setObjectName("disconnectBtn");
    disconnectBtn->setCheckable(true);
    disconnectBtn->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
    disconnectBtn->setCursor(Qt::PointingHandCursor);

    connect(disconnectBtn, &QPushButton::clicked, this, [=]() {
        this->onOpenDialog(ConfirmExitDlg);
    });

    connectedRowLayout->addWidget(deviceConnectedLbl);
    connectedRowLayout->addWidget(SRLbl);
    // connectedRowLayout->addWidget(onTimeLbl);
    connectedRowLayout->addStretch();
    connectedPageLayout->addWidget(disconnectBtn);
    connectedPageLayout->addStretch();

    connectionDeviceStack->addWidget(connectionPage);
    connectionDeviceStack->addWidget(connectedPage);
    connectionDeviceStack->setCurrentIndex(0);
    connectionDeviceStack->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mainLayout->addWidget(connectionDeviceStack);
    mainLayout->addStretch();
}

MainWindow::MainWindow(QWidget * parent) :
    QMainWindow(parent) {

    this->setWindowTitle(QString(GLB_SOFTWARE_NAME) + " " + GLB_SOFTWARE_VERSION_NUMBER + " (" + GLB_COMMLIB_NAME + " " + GLB_COMMLIB_VERSION_NUMBER + ")");
    this->setCorner(Qt::TopLeftCorner, Qt::LeftDockWidgetArea);

    dockWidgets.resize(DockWidgetsNum);
    dockWidgets.fill(nullptr);

    /************************\
     * device detector dock *
    \************************/

    auto deviceDetectorDw = new QDockWidget;
    deviceDetectorDw->setWindowTitle("Connection");
    deviceDetectorDw->setObjectName("deviceDetectorDw");
    deviceDetectorDw->setFloating(false);
    deviceDetectorDw->setTitleBarWidget(new QWidget()); // Hide connection widget titlebar
    deviceDetectorDw->setFeatures(QDockWidget::NoDockWidgetFeatures);

    this->setDockWidget(DWDeviceDetector, deviceDetectorDw, false, Qt::LeftDockWidgetArea);
    QFrame * deviceDetectorWid = new QFrame();
    deviceDetectorWid->setObjectName("deviceDetectorWid");
    deviceDetectorWid->setMinimumSize(1000, 380);

    // Device is not connected at this stage
    // Fixed viewport not resizable - centered
    QSize size = QSize(1000, 380);
    this->setMinimumSize(size);
    this->setMaximumSize(size);
    this->setWindowFlag(Qt::WindowMaximizeButtonHint, false);

    // QSS property for the Connecting Page
    deviceDetectorWid->setProperty("page", "connecting");
    setupDeviceConnectionGui(deviceDetectorWid);
    deviceDetectorDw->setWidget(deviceDetectorWid);

    /************\
     * menu bar *
    \************/
    QSettings settings;
    QMenuBar * menuBar = this->menuBar();

    /*! View menu */
    menuView = new QMenu("View");
    menuBar->addMenu(menuView);

    actionRearrangeView = new QAction("Rearrange floating widgets");
    actionRearrangeView->setEnabled(false);
    connect(actionRearrangeView, &QAction::triggered, this, &MainWindow::onRearrangeView);
    menuView->addAction(actionRearrangeView);

    menuView->addSeparator();

    menuRecordings = new QMenu("Recordings");
    menuBar->addMenu(menuRecordings);

    actionRecordingSettings = new QAction("Gap free settings");
    menuRecordings->addAction(actionRecordingSettings);
    actionRecordingSettings->setEnabled(false);

    menuPreferences = new QMenu("Preferences");
    menuBar->addMenu(menuPreferences);

    actionPlotPreferences = new QAction("Plots");
    menuPreferences->addAction(actionPlotPreferences);
    actionPlotPreferences->setEnabled(false);

    actionBoardMapping = new QAction("Board mappings");
    menuPreferences->addAction(actionBoardMapping);
    actionBoardMapping->setEnabled(false);
    connect(actionBoardMapping, &QAction::triggered, this, &MainWindow::onBoardMappingPressed);

    actionExportCopyableTableHeader = new QAction("Copy table header", this);
    actionExportCopyableTableHeader->setToolTip("Includes header when tables are copied to clipboard.");
    menuPreferences->addAction(actionExportCopyableTableHeader);
    actionExportCopyableTableHeader->setCheckable(true);

    connect(actionExportCopyableTableHeader, &QAction::changed, this, [=]() {
        QSettings settings;
        QString settingsRoot = "Preferences/UI/";
        bool exportCopyableTableHeader = this->actionExportCopyableTableHeader->isChecked();
        settings.setValue(settingsRoot + "exportCopyableTableHeader", exportCopyableTableHeader);
    });

    menuBar->addSeparator();

    /*! Theme menu within Preferencies */
    menuTheme = new QMenu("Theme", this);
    menuPreferences->addMenu(menuTheme);

    actionDarkTheme = new QAction("Dark", this);
    actionDarkTheme->setCheckable(true);
    actionDarkTheme->setData(Dark);

    actionLightTheme = new QAction("Light", this);
    actionLightTheme->setCheckable(true);
    actionLightTheme->setData(Light);

    themeActionGroup = new QActionGroup(this);
    themeActionGroup->addAction(actionDarkTheme);
    themeActionGroup->addAction(actionLightTheme);
    themeActionGroup->setExclusive(true);

    menuTheme->addAction(actionDarkTheme);
    menuTheme->addAction(actionLightTheme);

    // Restore user selected Theme
    // Dark Mode is enabled in case there is no saved value
    int savedTheme = settings.value("Preferences/UI/theme").toInt();
    if (savedTheme == Light) {
        actionLightTheme->setChecked(true);
    } else {
        actionDarkTheme->setChecked(true);
    }
    connect(themeActionGroup, &QActionGroup::triggered, this, &MainWindow::onThemeSelected);

    menuAdvanced = new QMenu("Advanced");
    menuBar->addMenu(menuAdvanced);

    actionUpgradeFw = new QAction("Upgrade FW");
    menuAdvanced->addAction(actionUpgradeFw);

    menuHwReset = new QMenu("HW reset");
    menuAdvanced->addMenu(menuHwReset);

    actionHwReset = new QAction("Apply");
    connect(actionHwReset, &QAction::triggered, this, &MainWindow::sigResetHw);
    menuHwReset->addAction(actionHwReset);

    actionHwResetHelp = new QAction("Help");
    connect(actionHwResetHelp, &QAction::triggered, this, [=]() {
        this->onOpenDialog(ResetHwHelpDlg);
    });
    menuHwReset->addAction(actionHwResetHelp);

    /*! ? menu */
    menuQuestionMark = new QMenu("?");
    menuBar->addMenu(menuQuestionMark);

    actionAbout = new QAction("About");
    connect(actionAbout, &QAction::triggered, this, [=]() {
        this->onOpenDialog(AboutDlg);
    });
    menuQuestionMark->addAction(actionAbout);

    actionDeviceInfo = new QAction("Device Info");
    connect(actionDeviceInfo, &QAction::triggered, this, [=]() {
        this->onOpenDialog(DeviceInfoDlg);
    });
    menuQuestionMark->addAction(actionDeviceInfo);

    actionSupport = new QAction("Support");
    connect(actionSupport, &QAction::triggered, this, [=]() {
        this->onOpenDialog(SupportDlg);
    });
    menuQuestionMark->addAction(actionSupport);

    actionReleaseNotes = new QAction("Release Notes");
    connect(actionReleaseNotes, &QAction::triggered, this, [=]() {
        this->onOpenDialog(ReleaseNotesDlg);
    });
    menuQuestionMark->addAction(actionReleaseNotes);

    connect(actionUpgradeFw, &QAction::triggered, this, &MainWindow::sigUpgradeFw);

    /************\
     * settings *
    \************/

    recordSettingsDialog = new RecordSettingsDialog;

    connect(actionRecordingSettings, &QAction::triggered, recordSettingsDialog, &RecordSettingsDialog::exec);

    /***********************\
     * central dock widgets *
    \***********************/

    centralWrapper = new QWidget(this);

    // 10 px bottom-margin for any central graph
    QVBoxLayout* wrapperLayout = new QVBoxLayout(centralWrapper);
    wrapperLayout->setContentsMargins(0, 0, 0, 10);
    wrapperLayout->setSpacing(0);
    centralWrapper->setLayout(wrapperLayout);
    this->setCentralWidget(centralWrapper);

    /* Margin management for TABBED widgets - occurs when there
     * is a switch from a TAB to another.
     */
    connect(this, &QMainWindow::tabifiedDockWidgetActivated, this, [this](QDockWidget*) {
        QTimer::singleShot(0, this, [this]() {
            for (auto* dw : this->findChildren<QDockWidget*>()) {
                if (!dw || !dw->widget() || !dw->widget()->layout()) continue;

                if (dw->isFloating()) {
                    dw->widget()->layout()->setContentsMargins(0, 0, 0, 0);
                } else if (this->tabifiedDockWidgets(dw).isEmpty()) {
                    dw->widget()->layout()->setContentsMargins(10, 0, 10, 10); // Single Widget
                } else {
                    dw->widget()->layout()->setContentsMargins(10, 0, 10, 0);  // Widget in a stacked TAB
                }
            }
        });
    });
}

MainWindow::~MainWindow() {
    this->destroyGuiControls();
}

void MainWindow::setMessageDispatcher(MessageDispatcher * msgDisp) {
    this->msgDisp = msgDisp;
}

QPushButton * MainWindow::getConnectButton() {
    return connectBtn;
}

QPushButton * MainWindow::getDisconnectButton() {
    return disconnectBtn;
}

QString MainWindow::getSelectedSerialNumber() {
    return devicesComboBox->currentText();
}

BigPlotWidget * MainWindow::getBigPlotWidget() {
    return bigPlotW;
}

QDockWidget * MainWindow::getDockWidget(DockWidgets_t type) {
    return dockWidgets[type];
}

RecordSettingsDialog * MainWindow::getRecordSettingsDialog() {
    return recordSettingsDialog;
}

void MainWindow::setDevicesList(std::vector <std::string> devicesList) {
    if (devicesList.size() > 0) {
        devicesComboBox->clear();

        for (unsigned int idx = 0; idx < devicesList.size(); idx++) {
            QString deviceName = QString::fromStdString(devicesList[idx]);
            devicesComboBox->addItem(deviceName);
        }
        devicesComboBox->setEnabled(true);
        connectBtn->setEnabled(true);

    } else {
        devicesComboBox->clear();

        devicesComboBox->setEnabled(false);
        connectBtn->setEnabled(false);
    }
}

PlotPreferencesDialog * MainWindow::getPlotPreferencesDialog() {
    return plotPreferencesDlg;
}

void MainWindow::setConnectedDeviceIdx(int idx) {
    devicesComboBox->setCurrentIndex(idx);
}

void MainWindow::connectDevice(bool flag, ErrorCodes_t err) {
    QString serial = devicesComboBox->itemText(devicesComboBox->currentIndex());

    if (flag) {
        if (err == Success) {
            devicesComboBox->setEnabled(false);
            this->createGuiControls();
            connectBtn->setChecked(true);

        } else {
            ErrorManager e(err);
            connectBtn->setChecked(false);
            connectBtn->setText("CONNECT");
        }

    } else {
        connectBtn->setText("CONNECT");
        connectBtn->setChecked(false);
        this->destroyGuiControls();
        devicesComboBox->setEnabled(true);
    }
}

void MainWindow::setConnectionLabel(QString text, bool errorFlag) {
    //connectionInfoLbl->setText(text);
    //if (errorFlag) {
    //    connectionInfoLbl->setStyleSheet("color: red; background-color: yellow; font-weight: bold");
    //}
    //else {
    //    connectionInfoLbl->setStyleSheet("");
    //}
}

/********************\
 * set dock widgets *
\********************/

void MainWindow::setBigPlotWidget(BigPlotWidget * widget) {
    if (widget == nullptr) return;

    auto layout = this->centralWidget()->layout();

    if (layout) {
        if (bigPlotW) {
            layout->removeWidget(bigPlotW);
            bigPlotW->deleteLater();
        }
        bigPlotW = widget;
        layout->addWidget(bigPlotW);
    }
}

/*
 * Adds dock wigets to DockArea. Manages floating widget status
 * margin to create a card effect.
 */
void MainWindow::setDockWidget(DockWidgets_t type, QDockWidget * widget, bool floatingFlag, Qt::DockWidgetArea area) {
    dockWidgets[type] = widget;
    if (widget != nullptr) {
        addDockWidget(area, dockWidgets[type]);
        dockWidgets[type]->setFloating(floatingFlag);

        QWidget *innerWidget = widget->widget();

        if (innerWidget) {
            // Widget default docked margins
            if (innerWidget->layout()) {
                innerWidget->layout()->setContentsMargins(10, 0, 10, 10);
            } else {
                innerWidget->setContentsMargins(10, 0, 10, 10);
            }

            // Centralized MARGIN manager for all widgets
            auto updateAllMargins = [this]() {

                if (this->connectionDeviceStack && this->connectionDeviceStack->currentIndex() == 0) {
                    return;
                }

                QTimer::singleShot(0, this, [this]() {
                    for (auto* dw : this->findChildren<QDockWidget*>()) {
                        if (!dw || !dw->widget() || !dw->widget()->layout()) continue;

                        if (dw->objectName() == "deviceDetectorDw") continue;

                        if (dw->isFloating()) {
                            dw->widget()->layout()->setContentsMargins(0, 0, 0, 0); // Single Widget - floating
                        } else if (this->tabifiedDockWidgets(dw).isEmpty()) {
                            dw->widget()->layout()->setContentsMargins(10, 0, 10, 10); // Single Widget - no TAB
                        } else {
                            dw->widget()->layout()->setContentsMargins(10, 0, 10, 0);  // Widget is stacked in a TAB
                        }
                        dw->setStyleSheet(""); // Trigger QSS reload
                    }
                });
            };

            connect(widget, &QDockWidget::topLevelChanged, this, updateAllMargins);
            connect(widget, &QDockWidget::dockLocationChanged, this, updateAllMargins);
            connect(widget, &QDockWidget::visibilityChanged, this, updateAllMargins);
        }
    }
}

void MainWindow::setPlotPreferencesDialog(PlotPreferencesDialog * ppd) {
    plotPreferencesDlg = ppd;
    connect(actionPlotPreferences, &QAction::triggered, plotPreferencesDlg, &PlotPreferencesDialog::exec);
}

void MainWindow::addViewActions() {
    actionRearrangeView->setEnabled(true);
    for (auto dw : dockWidgets) {
        if (dw != nullptr) {
            menuView->addAction(dw->toggleViewAction());
        }
    }
}

void MainWindow::removeViewActions() {
    actionRearrangeView->setEnabled(false);
    for (auto dw : dockWidgets) {
        if (dw != nullptr) {
            menuView->removeAction(dw->toggleViewAction());
        }
    }
}

void MainWindow::createGuiControls() {
    QSettings settings;

    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);

    this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    /******************\
     * protocols dock *
    \******************/

    if (msgDisp->hasProtocols() == Success) {
        auto protocolDw = new ProtocolDockWidget(msgDisp, e384CommLib::VOLTAGE_CLAMP, this);
        protocolDw->setObjectName("protocolDw");
        this->setDockWidget(MainWindow::DWProtocol, protocolDw, false, Qt::LeftDockWidgetArea);
    }

    actionRecordingSettings->setEnabled(true);
    actionPlotPreferences->setEnabled(true);
//    TODO maybe activate it only for devices with more then N channels
    actionBoardMapping->setEnabled(true);
    actionUpgradeFw->setEnabled(false);
    this->addViewActions();
    interfaceCreated = true;

}

void MainWindow::destroyGuiControls() {
    if (!interfaceCreated) {
        return;
    }

    QSettings settings;
    this->saveUISettings();

    this->removeViewActions();
    actionRecordingSettings->setEnabled(false);
    actionPlotPreferences->setEnabled(false);
    actionBoardMapping->setEnabled(false);
    actionUpgradeFw->setEnabled(true);

    if (dockWidgets[DWProtocol] != nullptr) {
        delete dockWidgets[DWProtocol];
        dockWidgets[DWProtocol] = nullptr;
    }

    if (dockWidgets[DWTemperatureSensors] != nullptr){
        delete dockWidgets[DWTemperatureSensors];
        dockWidgets[DWTemperatureSensors] = nullptr;
    }

    if (dockWidgets[DWDebug] != nullptr){
        delete dockWidgets[DWDebug];
        dockWidgets[DWDebug] = nullptr;
    }

    if (bigPlotW) {
        if (centralWrapper && centralWrapper->layout()) {
            centralWrapper->layout()->removeWidget(bigPlotW);
        }
        bigPlotW->deleteLater();
        bigPlotW = nullptr;
    }

    SRLbl->setText("");
    // onTimeLbl->setText("");
    // onTimeLbl->setVisible(false);

//    for (int shortcutIdx = 0; shortcutIdx < shortcuts.size(); shortcutIdx++) {
//        if (shortcuts[shortcutIdx] != nullptr) {
//            delete shortcuts[shortcutIdx];
//        }
//    }
//    shortcuts.clear();

    this->removeDockWidget(dockWidgets[DWDeviceDetector]);
    this->setDockWidget(DWDeviceDetector, dockWidgets[DWDeviceDetector], false, Qt::TopDockWidgetArea);
    dockWidgets[DWDeviceDetector]->setVisible(true);
    this->showHideConnectedDevice(false);
    this->setContentsMargins(0,0,0,0);
    interfaceCreated = false;
}

void MainWindow::restoreUISettings() {
    QTimer * timer = new QTimer;
    timer->setInterval(200);
    timer->setSingleShot(true);

    connect(timer, &QTimer::timeout, this, [=] () {
        QSettings settings;
        QString settingsRoot = "Preferences/UI/";
        QString tag;

        for (auto &&dw : dockWidgets) {
            if (dw != nullptr) {
                tag = settingsRoot + dw->objectName() + "/geometry";
                if (settings.contains(tag)) {
                    dw->setGeometry(settings.value(tag).value <QRect>());
                }
            }
        }

        tag = settingsRoot + bigPlotW->objectName() + "/geometry";
        if (settings.contains(tag)) {
            bigPlotW->setGeometry(settings.value(tag).value <QRect> ());
        }

        tag = settingsRoot + this->objectName() + "/state";
        this->restoreState(settings.value(tag).toByteArray());
    }, Qt::QueuedConnection);

    timer->start();
}

void MainWindow::saveUISettings() {
    QSettings settings;
    QString settingsRoot = "Preferences/UI/";
    QString tag;

    tag = settingsRoot + bigPlotW->objectName() + "/geometry";
    settings.setValue(tag, QVariant(bigPlotW->geometry()));

    for (auto dw : dockWidgets) {
        if (dw != nullptr) {
            tag = settingsRoot + dw->objectName() + "/geometry";
            settings.setValue(tag, QVariant(dw->geometry()));
        }
    }

    tag = settingsRoot + this->objectName() + "/state";
    settings.setValue(tag, this->saveState());

    // Save current app theme
    int themeEnum = this->themeActionGroup->checkedAction()->data().toInt();
    settings.setValue(settingsRoot + "theme", themeEnum);
}

void MainWindow::onNeedToChangeModelCellMsg(QString msg){
    QMessageBox msgBox;
    msgBox.setText(msg);
    msgBox.setStandardButtons(QMessageBox::Ok);
    if(msgBox.exec() == QMessageBox::Ok){
        emit sigModelCellChanged(true);
    }
}

void MainWindow::onBoardMappingPressed(){
    QString filePath = QFileDialog::getOpenFileName(this, "Choose Board Mapping File", QDir::homePath(), "YAML Files (*.yaml *.yml)");
    if (!filePath.isEmpty()){
        emit sigBoardMappingFileChoosen(filePath);
    }
//    else {
//        QMessageBox::critical(this, "Invalid Board Mapping", "The file you chose is not a valid board mapping. Please try again.", QMessageBox::Ok);
//    }
}

void MainWindow::onBitRateComputed(double value) {
    if (value > 1.0e6) {
        SRLbl->setText(QString("%1 Msps").arg(value/1.0e6));
    }
    else {
        SRLbl->setText(QString("%1 ksps").arg(value/1.0e3));
    }
}

void MainWindow::onOnTimeRead(Measurement_t onTime) {
    // onTimeLbl->setVisible(true);
    // onTimeLbl->setText(QString::fromStdString(onTime.label(15)));
}

void MainWindow::onOpenDialog(Dialogs_t type) {
    switch (type) {
    case ResetHwHelpDlg: {
        ResetHwHelpDialog a(this);
        a.exec();
        break;
    }
    case AboutDlg: {
        AboutDialog a(this);
        a.exec();
        break;
    }
    case DeviceInfoDlg: {
        DeviceInfoDialog a(msgDisp, devicesComboBox->currentText(), this);
        a.exec();
        break;
    }
    case SupportDlg: {
        SupportDialog a(this);
        a.exec();
        break;
    }
    case ReleaseNotesDlg: {
        ReleaseNotesDialog a(this);
        a.exec();
        break;
    }
    case ConfirmExitDlg: {
        ConfirmationDialog a(this);
        if (a.exec() == QDialog::Accepted) {
            emit confirmDisconnectDevice();
        }
        break;
    }
    }
}

void MainWindow::onRearrangeView() {
    int c = 0;
    for (auto dw : dockWidgets) {
        if (dw != nullptr) {
            if (dw->isVisible() && dw->isFloating()) {
                auto area = this->dockWidgetArea(dw);
                this->removeDockWidget(dw);
                this->addDockWidget(area, dw);
                dw->setFloating(true);
                dw->setVisible(true);
                auto s = dw->sizeHint();
                auto g = this->geometry();
                dw->setGeometry(g.x()+20*(c+1), g.y()+20*(c+2), s.width(), s.height());
                c++;
            }
        }
    }
}

void MainWindow::onThemeSelected() {
    QAction* activeAction = themeActionGroup->checkedAction();
    if (activeAction != nullptr) {
        int themeEnum = activeAction->data().toInt();
        QSettings settings;
        settings.setValue("Preferences/UI/theme", themeEnum);
        ThemeController::getInstance().applyTheme(static_cast<Theme>(themeEnum));
    }
}

void MainWindow::disconnectDevice(){
    emit confirmDisconnectDevice();
}

/*
 * Updates the Connection widget. Can shows the ComboBox
 * in case a device needs to be chosen. When device is
 * chosen shows a minimized.
 * See the setupDeviceConnectionGui.
 *
 * Param: flag | true  -> shows the Minimized Widget (readonly label)
 *             | false -> shows the SplashScreen style selection (ComboBox)
 */
void MainWindow::showHideConnectedDevice(bool flag){

    QFrame* deviceDetectorWid = qobject_cast<QFrame*>(this->getDockWidget(DWDeviceDetector)->widget());
    if (!deviceDetectorWid) return;

    QVBoxLayout* mainGrid = qobject_cast<QVBoxLayout*>(deviceDetectorWid->layout());
    QLabel* logoLabel = deviceDetectorWid->findChild<QLabel*>("elementsLogo");

    if (flag){
        // --- STATUS: DEVICE CONNECTED ---

        // Minimizing the widget
        if (mainGrid) mainGrid->setContentsMargins(15, 15, 15, 15);
        deviceDetectorWid->setMinimumWidth(250);
        deviceDetectorWid->setMinimumHeight(120);
        deviceDetectorWid->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Minimum);
        deviceDetectorWid->setProperty("page", "connected");

        deviceDetectorWid->style()->unpolish(deviceDetectorWid);
        deviceDetectorWid->style()->polish(deviceDetectorWid);
        deviceDetectorWid->update();

        if (logoLabel) logoLabel->setVisible(false);
        this->connectionDeviceStack->setCurrentIndex(1);

        QString connected = QString("%1 <span style='color:#4CAF50;'>●</span>").arg(getSelectedSerialNumber());
        this->deviceConnectedLbl->setText(connected);

        QTimer::singleShot(50, this, [this]() {
            // Making sure the mainframe is now resizable
            this->setMinimumSize(0, 0);
            this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
            setWindowFlag(Qt::WindowMaximizeButtonHint, true);
            this->show();
        });

    } else {
        // --- STATUS: DEVICE NOT CONNECTED ---

        if (mainGrid) mainGrid->setContentsMargins(40, 40, 40, 40);
        if (logoLabel) logoLabel->setVisible(true);

        // Show the CONNECTING page - device selection
        this->connectionDeviceStack->setCurrentIndex(0);
        deviceDetectorWid->setProperty("page", "connecting");

        // Resizing the window later
        QTimer::singleShot(50, this, [this, deviceDetectorWid]() {
            QSize size = QSize(1000, 380);

            this->setMinimumSize(0, 0);
            this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

            if (deviceDetectorWid) {
                deviceDetectorWid->setMinimumSize(size);
                deviceDetectorWid->setMaximumSize(size);
            }
            this->setMinimumSize(size);
            this->setMaximumSize(size);
            this->resize(size);
        });

        // QSS re-apply
        deviceDetectorWid->style()->unpolish(deviceDetectorWid);
        deviceDetectorWid->style()->polish(deviceDetectorWid);
        deviceDetectorWid->update();

    }
}
