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
#include "resethwhelpdialog.h"
#include "aboutdialog.h"
#include "supportdialog.h"
#include "deviceinfodialog.h"
#include "releasenotesdialog.h"

MainWindow::MainWindow(QWidget * parent) :
    QMainWindow(parent) {

    this->setObjectName("mainWindow");

    this->setWindowTitle(QString(GLB_SOFTWARE_NAME) + " " + GLB_SOFTWARE_VERSION_NUMBER);

    this->setCentralWidget(new ElementsLogoWidget);

    this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);

    dockWidgets.resize(DockWidgetsNum);
    dockWidgets.fill(nullptr);

    /************\
     * menu bar *
    \************/

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

    actionRecordingSettings = new QAction("Settings");
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

    /************************\
     * device detector dock *
    \************************/

    auto deviceDetectorDw = new QDockWidget;
    deviceDetectorDw->setWindowTitle("Connection");
    deviceDetectorDw->setObjectName("deviceDetectorDw");
    menuView->addAction(deviceDetectorDw->toggleViewAction());
    deviceDetectorDw->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    this->setDockWidget(DWDeviceDetector, deviceDetectorDw, false, Qt::TopDockWidgetArea);

    QWidget * deviceDetectorWid = new QWidget;
    deviceDetectorWid->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    deviceDetectorDw->setWidget(deviceDetectorWid);

    QHBoxLayout * deviceDetectorHl = new QHBoxLayout;
    deviceDetectorHl->setContentsMargins(1, 1, 1, 1);
    deviceDetectorHl->setSpacing(3);
    deviceDetectorWid->setLayout(deviceDetectorHl);

    devicesComboBox = new QComboBox;
    devicesComboBox->setEnabled(false);
    devicesComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    QFontMetrics fm((QFont()));
    devicesComboBox->setFixedWidth(fm.horizontalAdvance("device device device"));
    deviceDetectorHl->addWidget(devicesComboBox);

    connectBtn = new QPushButton("Connect");
    connectBtn->setEnabled(false);
    connectBtn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connectBtn->setCheckable(true);
    deviceDetectorHl->addWidget(connectBtn);

    connectionInfoLbl = new QLabel("");
    deviceDetectorHl->addWidget(connectionInfoLbl);

    SRLbl = new QLabel;
    deviceDetectorHl->addWidget(SRLbl);

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    deviceDetectorHl->addWidget(spacer);

    QTimer::singleShot(0, this, SLOT(showMaximized()));
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

#ifndef GLB_HIDE_DEBUG_CTRLS

    /**************\
     * debug dock *
    \**************/

    auto debugDw = new QDockWidget();
    debugDw->setObjectName("debugDw");
    debugDw->setWindowTitle("Debug");
    this->setDockWidget(MainWindow::DWDebug, debugDw, true, Qt::RightDockWidgetArea);

    QWidget * debugWid = new QWidget;
    debugDw->setWidget(debugWid);

    QVBoxLayout * debugVl = new QVBoxLayout;
    debugWid->setLayout(debugVl);

    debugVl->addWidget(new QLabel("Word"));
    QHBoxLayout * debugWordHl = new QHBoxLayout;
    debugVl->addLayout(debugWordHl);

    QSpinBox * debugWordSbx = new QSpinBox;
    debugWordSbx->setRange(0, 32767);
    debugWordSbx->setValue(0);
    debugWordHl->addWidget(debugWordSbx);

    QCheckBox * debugRangeWordChx = new QCheckBox("Range");
    debugWordHl->addWidget(debugRangeWordChx);

    QSpinBox * debugLastWordSbx = new QSpinBox;
    debugLastWordSbx->setRange(0, 32767);
    debugLastWordSbx->setValue(0);
    debugLastWordSbx->setEnabled(false);
    debugWordHl->addWidget(debugLastWordSbx);

    connect(debugRangeWordChx, &QCheckBox::clicked, debugLastWordSbx, &QWidget::setEnabled);

    debugVl->addWidget(new QLabel("Bit"));
    QHBoxLayout * debugBitHl = new QHBoxLayout;
    debugVl->addLayout(debugBitHl);

    QSpinBox * debugBitSbx = new QSpinBox;
    debugBitSbx->setRange(0, 15);
    debugBitSbx->setValue(0);
    debugBitHl->addWidget(debugBitSbx);

    QCheckBox * debugRangeBitChx = new QCheckBox("Range");
    debugBitHl->addWidget(debugRangeBitChx);

    QSpinBox * debugLastBitSbx = new QSpinBox;
    debugLastBitSbx->setRange(0, 15);
    debugLastBitSbx->setValue(0);
    debugLastBitSbx->setEnabled(false);
    debugBitHl->addWidget(debugLastBitSbx);

    connect(debugRangeBitChx, &QCheckBox::clicked, debugLastBitSbx, &QWidget::setEnabled);

    QHBoxLayout * debugBitSetHl = new QHBoxLayout;
    debugVl->addLayout(debugBitSetHl);

    QPushButton * debugResetBitBtn = new QPushButton("RESET bit");
    debugResetBitBtn->setCheckable(false);
    debugBitSetHl->addWidget(debugResetBitBtn);

    QPushButton * debugSetBitBtn = new QPushButton("SET bit");
    debugSetBitBtn->setCheckable(false);
    debugBitSetHl->addWidget(debugSetBitBtn);

    connect(debugResetBitBtn, &QPushButton::clicked, this, [=] () {
        if (debugRangeWordChx->isChecked()) {
            for (int wordIdx = debugWordSbx->value(); wordIdx <= debugLastWordSbx->value(); wordIdx++) {
                if (debugRangeBitChx->isChecked()) {
                    for (int bitIdx = debugBitSbx->value(); bitIdx <= debugLastBitSbx->value(); bitIdx++) {
                        emit setDebugBit(wordIdx, bitIdx, false);
                    }

                } else {
                    emit setDebugBit(wordIdx, debugBitSbx->value(), false);
                }
            }

        } else {
            if (debugRangeBitChx->isChecked()) {
                for (int bitIdx = debugBitSbx->value(); bitIdx <= debugLastBitSbx->value(); bitIdx++) {
                    emit setDebugBit(debugWordSbx->value(), bitIdx, false);
                }

            } else {
                emit setDebugBit(debugWordSbx->value(), debugBitSbx->value(), false);
            }
        }
    });

    connect(debugSetBitBtn, &QPushButton::clicked, this, [=] () {
        if (debugRangeWordChx->isChecked()) {
            for (int wordIdx = debugWordSbx->value(); wordIdx <= debugLastWordSbx->value(); wordIdx++) {
                if (debugRangeBitChx->isChecked()) {
                    for (int bitIdx = debugBitSbx->value(); bitIdx <= debugLastBitSbx->value(); bitIdx++) {
                        emit setDebugBit(wordIdx, bitIdx, true);
                    }

                } else {
                    emit setDebugBit(wordIdx, debugBitSbx->value(), true);
                }
            }

        } else {
            if (debugRangeBitChx->isChecked()) {
                for (int bitIdx = debugBitSbx->value(); bitIdx <= debugLastBitSbx->value(); bitIdx++) {
                    emit setDebugBit(debugWordSbx->value(), bitIdx, true);
                }

            } else {
                emit setDebugBit(debugWordSbx->value(), debugBitSbx->value(), true);
            }
        }
    });

    debugVl->addWidget(new QLabel("Value"));
    QHBoxLayout * debugValueSetHl = new QHBoxLayout;
    debugVl->addLayout(debugValueSetHl);

    QSpinBox * debugValueSbx = new QSpinBox;
    debugValueSbx->setRange(0, 65535);
    debugValueSbx->setValue(0);
    debugValueSetHl->addWidget(debugValueSbx);

    QLabel * debugValueHexLbl = new QLabel("0x0000");
    debugValueSetHl->addWidget(debugValueHexLbl);
    connect(debugValueSbx, QOverload <int> ::of (&QSpinBox::valueChanged), this, [=] (int value) {
        debugValueHexLbl->setText(QString("0x%1").arg(value, 4, 16, QLatin1Char('0')));
    });

    QPushButton * debugApplyValueBtn = new QPushButton("SET value");
    debugApplyValueBtn->setCheckable(false);
    debugValueSetHl->addWidget(debugApplyValueBtn);

    connect(debugApplyValueBtn, &QPushButton::clicked, this, [=] () {
        if (debugRangeWordChx->isChecked()) {
            for (int wordIdx = debugWordSbx->value(); wordIdx <= debugLastWordSbx->value(); wordIdx++) {
                emit setDebugWord(wordIdx, debugValueSbx->value());
            }

        } else {
            emit setDebugWord(debugWordSbx->value(), debugValueSbx->value());
        }
    });

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    debugVl->addWidget(spacer);

#endif

    actionRecordingSettings->setEnabled(true);
    actionPlotPreferences->setEnabled(true);
//    TODO maybe activate it only for devices with more then N channels
    actionBoardMapping->setEnabled(true);
    actionUpgradeFw->setEnabled(false);
    this->addViewActions();

    this->restoreUISettings();
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

        for (auto dw : dockWidgets) {
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

//        qDebug() << this->geometry();
//        this->doc
    });

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
        DeviceInfoDialog a(msgDisp != nullptr, devicesComboBox->currentText(), this);
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
