#include "mainwindow.h"

#include <QBoxLayout>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QPushButton>

#include "messagedispatcher.h"
#include "elementslogowidget.h"
#include "errormanager.h"
#include "globaldefines.h"
#include "qactiongroup.h"
#include "qwindow.h"
#include "resethwhelpdialog.h"
#include "aboutdialog.h"
#include "supportdialog.h"
#include "deviceinfodialog.h"
#include "releasenotesdialog.h"
#include "themecontroller.h"

void MainWindow::setupDeviceConnectionGui(QFrame * container){

    QGridLayout *mainGrid = new QGridLayout(container);
    mainGrid->setContentsMargins(40, 40, 40, 40);

    // Logo on top right
    QLabel *logoLabel = new QLabel();
    logoLabel->setPixmap(QPixmap(":/imgs/logo_with_name_white.png").scaled(200, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    mainGrid->addWidget(logoLabel, 0, 1, Qt::AlignTop | Qt::AlignRight);

    // Left column
    QVBoxLayout *widgetsColumn = new QVBoxLayout();
    widgetsColumn->setSpacing(10);
    widgetsColumn->setContentsMargins(0, 0, 0, 0);

    QLabel *titleLbl = new QLabel("DEVICES");
    titleLbl->setObjectName("selectionTitle");
    QLabel *hintLbl = new QLabel("Available hardware");
    hintLbl->setObjectName("splashSubtitle");

    widgetsColumn->addWidget(titleLbl);
    widgetsColumn->addWidget(hintLbl);
    widgetsColumn->addSpacing(10);

    devicesComboBox = new QComboBox();
    devicesComboBox->setObjectName("deviceCombo");
    devicesComboBox->setFixedWidth(220);

    connectBtn = new QPushButton("CONNECT");
    connectBtn->setObjectName("connectBtn");
    connectBtn->setFixedWidth(220);
    connectBtn->setCursor(Qt::PointingHandCursor);

    widgetsColumn->addWidget(devicesComboBox);
    widgetsColumn->addWidget(connectBtn);

    // Left column is also vertically aligned
    mainGrid->addLayout(widgetsColumn, 0, 0, Qt::AlignVCenter | Qt::AlignLeft);
    mainGrid->setColumnStretch(0, 1);
    mainGrid->setColumnStretch(1, 0);
    mainGrid->setRowStretch(0, 1);
}

MainWindow::MainWindow(QWidget * parent) :
    QMainWindow(parent) {

    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    setFixedSize(1000, 450);

    this->setWindowTitle(QString(GLB_SOFTWARE_NAME) + " " + GLB_SOFTWARE_VERSION_NUMBER);

    //this->setCentralWidget(new ElementsLogoWidget);

    //this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);

    dockWidgets.resize(DockWidgetsNum);
    dockWidgets.fill(nullptr);

    auto deviceDetectorDw = new QDockWidget;
    deviceDetectorDw->setWindowTitle("Connection");
    deviceDetectorDw->setObjectName("deviceDetectorDw");
    deviceDetectorDw->setFloating(false);
    deviceDetectorDw->setTitleBarWidget(new QWidget()); // Hide connection widget titlebar

    this->setDockWidget(DWDeviceDetector, deviceDetectorDw, false, Qt::LeftDockWidgetArea);

    QFrame * deviceDetectorWid = new QFrame();
    deviceDetectorWid->setObjectName("SplashContainer");
    deviceDetectorWid->setFixedSize(1000, 450);
    setupDeviceConnectionGui(deviceDetectorWid);
    deviceDetectorDw->setWidget(deviceDetectorWid);

    /************\
     * menu bar *
    \************/
    // QSettings settings;
    // QMenuBar * menuBar = this->menuBar();

    // /*! View menu */
    // menuView = new QMenu("View");
    // menuBar->addMenu(menuView);

    // actionRearrangeView = new QAction("Rearrange floating widgets");
    // actionRearrangeView->setEnabled(false);
    // connect(actionRearrangeView, &QAction::triggered, this, &MainWindow::onRearrangeView);
    // menuView->addAction(actionRearrangeView);

    // menuView->addSeparator();

    // menuRecordings = new QMenu("Recordings");
    // menuBar->addMenu(menuRecordings);

    // actionRecordingSettings = new QAction("Gap free settings");
    // menuRecordings->addAction(actionRecordingSettings);
    // actionRecordingSettings->setEnabled(false);

    // menuPreferences = new QMenu("Preferences");
    // menuBar->addMenu(menuPreferences);

    // actionPlotPreferences = new QAction("Plots");
    // menuPreferences->addAction(actionPlotPreferences);
    // actionPlotPreferences->setEnabled(false);

    // actionBoardMapping = new QAction("Board mappings");
    // menuPreferences->addAction(actionBoardMapping);
    // actionBoardMapping->setEnabled(false);
    // connect(actionBoardMapping, &QAction::triggered, this, &MainWindow::onBoardMappingPressed);

    // // Theme menu within Preferencies
    // menuTheme = new QMenu("Theme", this);
    // menuPreferences->addMenu(menuTheme);

    // actionDarkTheme = new QAction("Dark", this);
    // actionDarkTheme->setCheckable(true);
    // actionDarkTheme->setData(ThemeController::Dark);

    // actionLightTheme = new QAction("Light", this);
    // actionLightTheme->setCheckable(true);
    // actionLightTheme->setData(ThemeController::Light);

    // themeActionGroup = new QActionGroup(this);
    // themeActionGroup->addAction(actionDarkTheme);
    // themeActionGroup->addAction(actionLightTheme);
    // themeActionGroup->setExclusive(true);

    // menuTheme->addAction(actionDarkTheme);
    // menuTheme->addAction(actionLightTheme);

    // // Restore user selected Theme
    // // Dark Mode is enabled in case there is no saved value
    // int savedTheme = settings.value("Preferences/UI/theme").toInt();
    // if (savedTheme == ThemeController::Light) {
    //     actionLightTheme->setChecked(true);
    // } else {
    //     actionDarkTheme->setChecked(true);
    // }
    // connect(themeActionGroup, &QActionGroup::triggered, this, &MainWindow::onThemeSelected);

    // menuAdvanced = new QMenu("Advanced");
    // menuBar->addMenu(menuAdvanced);

    // actionUpgradeFw = new QAction("Upgrade FW");
    // menuAdvanced->addAction(actionUpgradeFw);

    // menuHwReset = new QMenu("HW reset");
    // menuAdvanced->addMenu(menuHwReset);

    // actionHwReset = new QAction("Apply");
    // connect(actionHwReset, &QAction::triggered, this, &MainWindow::sigResetHw);
    // menuHwReset->addAction(actionHwReset);

    // actionHwResetHelp = new QAction("Help");
    // connect(actionHwResetHelp, &QAction::triggered, this, [=]() {
    //     this->onOpenDialog(ResetHwHelpDlg);
    // });
    // menuHwReset->addAction(actionHwResetHelp);

    // /*! ? menu */
    // menuQuestionMark = new QMenu("?");
    // menuBar->addMenu(menuQuestionMark);

    // actionAbout = new QAction("About");
    // connect(actionAbout, &QAction::triggered, this, [=]() {
    //     this->onOpenDialog(AboutDlg);
    // });
    // menuQuestionMark->addAction(actionAbout);

    // actionDeviceInfo = new QAction("Device Info");
    // connect(actionDeviceInfo, &QAction::triggered, this, [=]() {
    //     this->onOpenDialog(DeviceInfoDlg);
    // });
    // menuQuestionMark->addAction(actionDeviceInfo);

    // actionSupport = new QAction("Support");
    // connect(actionSupport, &QAction::triggered, this, [=]() {
    //     this->onOpenDialog(SupportDlg);
    // });
    // menuQuestionMark->addAction(actionSupport);

    // actionReleaseNotes = new QAction("Release Notes");
    // connect(actionReleaseNotes, &QAction::triggered, this, [=]() {
    //     this->onOpenDialog(ReleaseNotesDlg);
    // });
    // menuQuestionMark->addAction(actionReleaseNotes);

    // connect(actionUpgradeFw, &QAction::triggered, this, &MainWindow::sigUpgradeFw);

    // /************\
    //  * settings *
    // \************/

    // recordSettingsDialog = new RecordSettingsDialog;

    // connect(actionRecordingSettings, &QAction::triggered, recordSettingsDialog, &RecordSettingsDialog::exec);

    /************************\
     * device detector dock *
    \************************/

    // auto deviceDetectorDw = new QDockWidget;
    // deviceDetectorDw->setWindowTitle("Connection");
    // deviceDetectorDw->setObjectName("deviceDetectorDw");
    // menuView->addAction(deviceDetectorDw->toggleViewAction());
    // deviceDetectorDw->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    // this->setDockWidget(DWDeviceDetector, deviceDetectorDw, false, Qt::TopDockWidgetArea);

    // QWidget * deviceDetectorWid = new QWidget;
    // deviceDetectorWid->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    // deviceDetectorDw->setWidget(deviceDetectorWid);

    // QHBoxLayout * deviceDetectorHl = new QHBoxLayout;
    // deviceDetectorHl->setContentsMargins(1, 1, 1, 1);
    // deviceDetectorHl->setSpacing(3);
    // deviceDetectorWid->setLayout(deviceDetectorHl);

    // devicesComboBox = new QComboBox;
    // devicesComboBox->setEnabled(false);
    // devicesComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    // QFontMetrics fm((QFont()));
    // devicesComboBox->setFixedWidth(fm.horizontalAdvance("device device device"));
    // deviceDetectorHl->addWidget(devicesComboBox);

    // connectBtn = new QPushButton("Connect");
    // connectBtn->setEnabled(false);
    // connectBtn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    // connectBtn->setCheckable(true);
    // deviceDetectorHl->addWidget(connectBtn);

    // connectionInfoLbl = new QLabel("");
    // deviceDetectorHl->addWidget(connectionInfoLbl);

    // SRLbl = new QLabel;
    // deviceDetectorHl->addWidget(SRLbl);

    // QWidget * spacer = new QWidget;
    // spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    // deviceDetectorHl->addWidget(spacer);

    //QTimer::singleShot(2000, this, SLOT(showMaximized()));
    this->move(QGuiApplication::primaryScreen()->geometry().center() - rect().center());
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
            connectBtn->setText("Disconnect");

            devicesComboBox->setEnabled(false);
            this->createGuiControls();
            connectBtn->setChecked(true);

        } else {
            ErrorManager e(err);
            connectBtn->setChecked(false);
        }

    } else {
        connectBtn->setText(QString::fromStdString("Connect"));
        connectBtn->setChecked(false);

        this->destroyGuiControls();
        devicesComboBox->setEnabled(true);
    }
}

void MainWindow::setConnectionLabel(QString text) {
    connectionInfoLbl->setText(text);
}

/********************\
 * set dock widgets *
\********************/

void MainWindow::setBigPlotWidget(BigPlotWidget * widget) {
    bigPlotW = widget;
    if (widget != nullptr) {
        delete this->takeCentralWidget();
        this->setCentralWidget(bigPlotW);
        this->centralWidget()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    }
}

void MainWindow::setDockWidget(DockWidgets_t type, QDockWidget * widget, bool floatingFlag, Qt::DockWidgetArea area) {
    dockWidgets[type] = widget;
    if (widget != nullptr) {
        addDockWidget(area, dockWidgets[type]);
        dockWidgets[type]->setFloating(floatingFlag);
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

    this->setStyleSheet("QSplitter::handle{image: url(:/imgs/splitter handle.png)}");

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

    this->takeCentralWidget();

    SRLbl->setText("");

//    for (int shortcutIdx = 0; shortcutIdx < shortcuts.size(); shortcutIdx++) {
//        if (shortcuts[shortcutIdx] != nullptr) {
//            delete shortcuts[shortcutIdx];
//        }
//    }
//    shortcuts.clear();

    this->setCentralWidget(new ElementsLogoWidget);
    this->removeDockWidget(dockWidgets[DWDeviceDetector]);
    this->setDockWidget(DWDeviceDetector, dockWidgets[DWDeviceDetector], false, Qt::TopDockWidgetArea);
    dockWidgets[DWDeviceDetector]->setVisible(true);
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
        ThemeController::getInstance().applyTheme(static_cast<ThemeController::Theme>(themeEnum));
    }
}
