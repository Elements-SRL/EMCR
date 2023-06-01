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

MainWindow::MainWindow(QWidget * parent) :
    QMainWindow(parent) {

    /*! This line ensures that showMaximized() in main.cpp works */
    this->setGeometry(0, 0, 800, 600);
    this->setObjectName("mainWindow");

    this->setWindowTitle(QString(GLB_SOFTWARE_NAME) + " " + GLB_SOFTWARE_VERSION_NUMBER);

    this->setCentralWidget(new ElementsLogoWidget);

    this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);

    /************\
     * menu bar *
    \************/

    QMenuBar * menuBar = this->menuBar();

    /*! View menu */
    menuView = new QMenu("View");
    menuBar->addMenu(menuView);

    menuRecordings = new QMenu("Recordings");
    menuBar->addMenu(menuRecordings);

    actionRecordingSettings = new QAction("Settings");
    menuRecordings->addAction(actionRecordingSettings);
    actionRecordingSettings->setEnabled(false);

    /************\
     * settings *
    \************/

    recordSettingsDialog = new RecordSettingsDialog;

    connect(actionRecordingSettings, &QAction::triggered, recordSettingsDialog, &RecordSettingsDialog::exec);

    /************************\
     * device detector dock *
    \************************/

    deviceDetectorDw = new QDockWidget;
    deviceDetectorDw->setWindowTitle("Connection");
    deviceDetectorDw->setObjectName("deviceDetectorDw");
    menuView->addAction(deviceDetectorDw->toggleViewAction());
    deviceDetectorDw->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    this->addDockWidget(Qt::TopDockWidgetArea, deviceDetectorDw);

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
    devicesComboBox->setFixedWidth(fm.width("device device device"));
    deviceDetectorHl->addWidget(devicesComboBox);

    connectBtn = new QPushButton("Connect");
    connectBtn->setEnabled(false);
    connectBtn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connectBtn->setCheckable(true);
    deviceDetectorHl->addWidget(connectBtn);

    SRLbl = new QLabel;
    deviceDetectorHl->addWidget(SRLbl);
}

MainWindow::~MainWindow() {
    this->destroyGuiControls();
}

void MainWindow::setModelDevice(ModelDevice * modelDevice) {
    mDev = modelDevice;
}

QPushButton * MainWindow::getConnectButton() {
    return connectBtn;
}

QString MainWindow::getSelectedSerialNumber() {
    return devicesComboBox->itemText(devicesComboBox->currentIndex());
}

Chessboard * MainWindow::getChessaboard() {
    return chessboard;
}

BigPlotDockWidget * MainWindow::getBigPlotWidget() {
    return bigPlotDw;
}

DeviceControlDockWidget * MainWindow::getDeviceControlsDockWidget() {
    return deviceControlsDw;
}

ChannelControlDockWidget * MainWindow::getChannelControlsDockWidget() {
    return channelControlsDw;
}

BoardControlDockWidget * MainWindow::getBoardControlsDockWidget() {
    return boardControlsDw;
}

ProtocolDockWidget * MainWindow::getProtocolDockWidget() {
    return protocolDw;
}

RecordSettingsDialog * MainWindow::getRecordSettingsDialog() {
    return recordSettingsDialog;
}

CompensationControlDockWidget * MainWindow::getCompensationControlsDockWidget() {
    return compensationControlsDw;
}

void MainWindow::onDevicesListChanged(std::vector <std::string> devicesList) {
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

StateArrayDockWidget * MainWindow::getStateArrayDockWidget(){
    return stateArrayDockWidget;
}

void MainWindow::onSetConnectedDeviceIdx(int idx) {
    devicesComboBox->setCurrentIndex(idx);
}

void MainWindow::onConnect(bool flag, ErrorCodes_t err) {
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

void MainWindow::createGuiControls() {
    QSettings settings;

    this->setStyleSheet("QSplitter::handle{image: url(:/imgs/splitter handle.png)}");

    mDev->getChannelsNumberFeatures(voltageChannelsNum, currentChannelsNum);

    dockWidgets.clear();

    this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    /*****************\
     * controls dock *
    \*****************/

    deviceControlsDw = new DeviceControlDockWidget(mDev);
    deviceControlsDw->setObjectName("deviceControlsDw");
    this->addDockWidget(Qt::RightDockWidgetArea, deviceControlsDw);
    dockWidgets.append(deviceControlsDw);

    boardControlsDw = new BoardControlDockWidget(mDev);
    boardControlsDw->setObjectName("boardControlsDw");
    this->addDockWidget(Qt::RightDockWidgetArea, boardControlsDw);
    dockWidgets.append(boardControlsDw);

    channelControlsDw = new ChannelControlDockWidget(mDev);
    channelControlsDw->setObjectName("channelControlsDw");
    this->addDockWidget(Qt::RightDockWidgetArea, channelControlsDw);
    dockWidgets.append(channelControlsDw);

    compensationControlsDw = new CompensationControlDockWidget(mDev);
    compensationControlsDw->setObjectName("compensationControlsDw");
    this->addDockWidget(Qt::RightDockWidgetArea, compensationControlsDw);
    dockWidgets.append(compensationControlsDw);

    /******************\
     * protocols dock *
    \******************/

    protocolDw = new ProtocolDockWidget(mDev, e384CommLib::VOLTAGE_CLAMP);
    protocolDw->setObjectName("protocolDw");
    this->addDockWidget(Qt::LeftDockWidgetArea, protocolDw);
    dockWidgets.append(protocolDw);

    /*********\
     * plots *
    \*********/

    chessboard = new Chessboard(mDev);
    chessboard->setObjectName("chessboard");
    delete this->takeCentralWidget();
    this->setCentralWidget(chessboard);
    this->centralWidget()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    bigPlotDw = new BigPlotDockWidget(mDev);
    bigPlotDw->setObjectName("bigPlotDw");
    this->addDockWidget(Qt::BottomDockWidgetArea, bigPlotDw);
    dockWidgets.append(bigPlotDw);

    //STATE ARRAY WIDGET
    stateArrayDockWidget = new StateArrayDockWidget(this);
    stateArrayDockWidget->setObjectName("stateArrayDockWidget");
    this->addDockWidget(Qt::RightDockWidgetArea, stateArrayDockWidget);
    dockWidgets.append(stateArrayDockWidget);
    stateArrayDockWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    /**************\
     * debug dock *
    \**************/

#ifndef GLB_HIDE_DEBUG_CTRLS
    QDockWidget * debugDw = new QDockWidget();
    debugDw->setObjectName("debugDw");
    debugDw->setWindowTitle("Debug");
    this->addDockWidget(Qt::RightDockWidgetArea, debugDw);
    dockWidgets.append(debugDw);

    debugDw->setFloating(true);

    QWidget * debugWid = new QWidget;
    debugDw->setWidget(debugWid);

    QVBoxLayout * debugVl = new QVBoxLayout;
    debugWid->setLayout(debugVl);

    debugVl->addWidget(new QLabel("Word"));
    QSpinBox * debugWordSbx = new QSpinBox;
    debugWordSbx->setRange(0, 32767);
    debugWordSbx->setValue(0);
    debugVl->addWidget(debugWordSbx);

    debugVl->addWidget(new QLabel("Bit"));
    QSpinBox * debugBitSbx = new QSpinBox;
    debugBitSbx->setRange(0, 15);
    debugBitSbx->setValue(0);
    debugVl->addWidget(debugBitSbx);

    QCheckBox * debugStatusChx = new QCheckBox("Status");
    debugVl->addWidget(debugStatusChx);

    QPushButton * debugApplyBtn = new QPushButton("Apply bit");
    debugApplyBtn->setCheckable(false);
    debugVl->addWidget(debugApplyBtn);

    connect(debugApplyBtn, &QPushButton::clicked, this, [=] () {
        emit setDebugBit(debugWordSbx->value(), debugBitSbx->value(), debugStatusChx->isChecked());
    });

    debugVl->addWidget(new QLabel("Value"));
    QSpinBox * debugValueSbx = new QSpinBox;
    debugValueSbx->setRange(0, 65535);
    debugValueSbx->setValue(0);
    debugVl->addWidget(debugValueSbx);

    QLabel * debugValueHexLbl = new QLabel;
    debugVl->addWidget(debugValueHexLbl);
    connect(debugValueSbx, QOverload <int> ::of (&QSpinBox::valueChanged), this, [=] (int value) {
        debugValueHexLbl->setText(QString("0x%1").arg(value, 4, 16, QLatin1Char('0')));
    });

    QPushButton * debugApplyValueBtn = new QPushButton("Apply value");
    debugApplyValueBtn->setCheckable(false);
    debugVl->addWidget(debugApplyValueBtn);

    connect(debugApplyValueBtn, &QPushButton::clicked, this, [=] () {
        emit setDebugWord(debugWordSbx->value(), debugValueSbx->value());
    });

    QPushButton * degugInitializeBtn = new QPushButton("Initialize");
    debugVl->addWidget(degugInitializeBtn);
    connect(degugInitializeBtn, &QPushButton::clicked, this, &MainWindow::debugInitialization);



    /*! ------------------------------------------------------------ */
    QDockWidget * calibrationDw = new QDockWidget();
    calibrationDw->setObjectName("calibrationDw");
    calibrationDw->setWindowTitle("Calibration");
    this->addDockWidget(Qt::RightDockWidgetArea, calibrationDw);
    dockWidgets.append(calibrationDw);

    calibrationDw->setFloating(true);

    QWidget * calibrationWid = new QWidget;
    calibrationDw->setWidget(calibrationWid);

    QVBoxLayout * calibrationVl = new QVBoxLayout;
    calibrationWid->setLayout(calibrationVl);

    calibrationVl->addWidget(new QLabel("Board (0 all)"));
    QSpinBox * boardCalibSbx = new QSpinBox;
    boardCalibSbx->setRange(0, 24);
//    boardCalibSbx->setValue(0);
    boardCalibSbx->setSpecialValueText(tr("ALL BOARDS"));
    calibrationVl->addWidget(boardCalibSbx);

    QPushButton * calibrationAllApplyBtn = new QPushButton("Calibrate");
    calibrationAllApplyBtn->setCheckable(false);
    calibrationVl->addWidget(calibrationAllApplyBtn);

//    QPushButton * calibrationBoardApplyBtn = new QPushButton("Calibrate board 2");
//    calibrationBoardApplyBtn->setCheckable(false);
//    calibrationVl->addWidget(calibrationBoardApplyBtn);

    connect(calibrationAllApplyBtn, &QPushButton::clicked, this, [=] () {
        std::vector<uint16_t> channelsToCalibrateIdxs;
        //------------------------
        if(boardCalibSbx->value() == 0){
            for(int i = 0; i < currentChannelsNum; i++){
                channelsToCalibrateIdxs.push_back(i);
            }
        } else {
            for(int i = 16*(boardCalibSbx->value()-1); i < 16*(boardCalibSbx->value()-1) + 16; i++){
                channelsToCalibrateIdxs.push_back(i);
            }

        }
        //------------------------
//        for(int i = 0; i < currentChannelsNum; i++){
//            channelsToCalibrateIdxs.push_back(i);
//        }
        emit sigPerformCalibration(channelsToCalibrateIdxs);
    });


//    connect(calibrationBoardApplyBtn, &QPushButton::clicked, this, [=] () {
//        std::vector<uint16_t> channelsToCalibrateIdxs;
//        for(int i = 16; i < 32; i++){
//            channelsToCalibrateIdxs.push_back(i);
//        }
//        emit sigPerformCalibration(channelsToCalibrateIdxs);
//    });

    /*! ------------------------------------------------------------ */

#endif

    actionRecordingSettings->setEnabled(true);
    this->addViewActions();

    this->restoreUISettings();
    interfaceCreated = true;

    emit widgetsCreated();
}

void MainWindow::destroyGuiControls() {
    if (!interfaceCreated) {
        return;
    }

    QSettings settings;
    this->saveUISettings();

    this->removeViewActions();
    actionRecordingSettings->setEnabled(false);

//    if (plotPreferencesDlg != nullptr) {
//        delete plotPreferencesDlg;
//        plotPreferencesDlg = nullptr;
//    }
//    actionPlotPreferences->setEnabled(false);

//    if (menuFrontEndResetDenoiser->isEnabled()) {
//        settings.setValue("Preferences/frontEndResetDenoiser", actionFrontEndResetDenoiserEnable->isChecked());
//        menuFrontEndResetDenoiser->setEnabled(false);
//    }

//    if (actionDigitalOffsetCompensationAutostop->isEnabled()) {
//        settings.setValue("Preferences/digitalOffsetCompensationAutostop", actionDigitalOffsetCompensationAutostop->isChecked());
//        actionDigitalOffsetCompensationAutostop->setEnabled(false);
//    }

//    menuReset->setEnabled(false);
//    this->removeViewActions();

    for (int dockIdx = 0; dockIdx < dockWidgets.size(); dockIdx++) {
        if (dockWidgets[dockIdx] != nullptr) {
            delete dockWidgets[dockIdx];
            dockWidgets[dockIdx] = nullptr;
        }
    }
    dockWidgets.clear();

    for (int dockIdx = 0; dockIdx < analysisWidgets.size(); dockIdx++) {
        if (analysisWidgets[dockIdx] != nullptr) {
            delete analysisWidgets[dockIdx];
            analysisWidgets[dockIdx] = nullptr;
        }
    }
    analysisWidgets.clear();

//    if (deviceDataProducer!= nullptr) {
//        deviceDataProducer->onStopProducing();
//    }

//    for (int consumerIdx = 0; consumerIdx < consumers.size(); consumerIdx++) {
//        if (consumers[consumerIdx] != nullptr) {
//            consumers[consumerIdx]->onStopConsuming();
//            delete consumers[consumerIdx];
//            consumers[consumerIdx] = nullptr;
//        }
//    }

//    if (deviceDataProducer != nullptr) {
//        delete deviceDataProducer; /*! The destructor also stops the producer thread */
//        deviceDataProducer = nullptr;
//    }

    if (chessboard != nullptr) {
        this->takeCentralWidget();
        delete chessboard;
        chessboard = nullptr;
    }

//    for (int shortcutIdx = 0; shortcutIdx < shortcuts.size(); shortcutIdx++) {
//        if (shortcuts[shortcutIdx] != nullptr) {
//            delete shortcuts[shortcutIdx];
//        }
//    }
//    shortcuts.clear();

    this->setCentralWidget(new ElementsLogoWidget);
    this->removeDockWidget(deviceDetectorDw);
    this->addDockWidget(Qt::TopDockWidgetArea, deviceDetectorDw);
    deviceDetectorDw->setVisible(true);
    interfaceCreated = false;

    emit widgetsDestroyed();
}

void MainWindow::addViewActions() {
    for (int dockIdx = 0; dockIdx < dockWidgets.size(); dockIdx++) {
        menuView->addAction(dockWidgets[dockIdx]->toggleViewAction());
    }

//    for (int dockIdx = 0; dockIdx < analysisWidgets.size(); dockIdx++) {
//        analysisMenus[dockIdx]->addAction(analysisWidgets[dockIdx]->toggleViewAction());
//        analysisMenus[dockIdx]->actions().at(analysisMenus[dockIdx]->actions().size()-1)->setText(analysisActionNames[dockIdx]);
//    }
}

void MainWindow::removeViewActions() {
    for (int dockIdx = 0; dockIdx < dockWidgets.size(); dockIdx++) {
        menuView->removeAction(dockWidgets[dockIdx]->toggleViewAction());
    }

//    for (int dockIdx = 0; dockIdx < analysisWidgets.size(); dockIdx++) {
//        menuAnalysis->removeAction(analysisWidgets[dockIdx]->toggleViewAction());
//    }
}

void MainWindow::restoreUISettings() {
    QTimer * timer = new QTimer;
    timer->setInterval(10);
    timer->setSingleShot(true);

    connect(timer, &QTimer::timeout, this, [=] () {
        QSettings settings;
        QString settingsRoot = "Preferences/UI/";
        QString tag;

        for (int dockIdx = 0; dockIdx < dockWidgets.size(); dockIdx++) {
            tag = settingsRoot + dockWidgets[dockIdx]->objectName() + "/geometry";
            if (settings.contains(tag)) {
                dockWidgets[dockIdx]->setGeometry(settings.value(tag).value <QRect> ());
            }
        }

        for (int dockIdx = 0; dockIdx < analysisWidgets.size(); dockIdx++) {
            tag = settingsRoot + analysisWidgets[dockIdx]->objectName() + "/geometry";
            if (settings.contains(tag)) {
                analysisWidgets[dockIdx]->setGeometry(settings.value(tag).value <QRect> ());
            }
        }

        tag = settingsRoot + chessboard->objectName() + "/geometry";
        if (settings.contains(tag)) {
            chessboard->setGeometry(settings.value(tag).value <QRect> ());
        }

        tag = settingsRoot + this->objectName() + "/state";
        this->restoreState(settings.value(tag).toByteArray());
    });

    timer->start();
}

void MainWindow::saveUISettings() {
    QSettings settings;
    QString settingsRoot = "Preferences/UI/";
    QString tag;

    tag = settingsRoot + chessboard->objectName() + "/geometry";
    settings.setValue(tag, QVariant(chessboard->geometry()));

    for (int dockIdx = 0; dockIdx < dockWidgets.size(); dockIdx++) {
        tag = settingsRoot + dockWidgets[dockIdx]->objectName() + "/geometry";
        settings.setValue(tag, QVariant(dockWidgets[dockIdx]->geometry()));
    }

    for (int dockIdx = 0; dockIdx < analysisWidgets.size(); dockIdx++) {
        tag = settingsRoot + analysisWidgets[dockIdx]->objectName() + "/geometry";
        settings.setValue(tag, QVariant(analysisWidgets[dockIdx]->geometry()));
    }

    tag = settingsRoot + this->objectName() + "/state";
    settings.setValue(tag, this->saveState());
}

void MainWindow::onCalibLoadingMsg(QString msg){
    QMessageBox msgBox;
    msgBox.about(this, "Calibration info", msg);
}

void MainWindow::onManualCalibDoneMsg(QString msg){
    QMessageBox msgBox;
    msgBox.about(this, "Calibration info", msg);
}

void MainWindow::onNeedToChangeModelCellMsg(QString msg){
    QMessageBox msgBox;
    msgBox.setText(msg);
    msgBox.setStandardButtons(QMessageBox::Ok);
    if(msgBox.exec() == QMessageBox::Ok){
        emit sigModelCellChanged(true);
    }
}

void MainWindow::onNeedToCheckFirstModelCellMsg(QString msg){
    QMessageBox msgBox;
    msgBox.setText(msg);
    msgBox.setStandardButtons(QMessageBox::Ok);
    if(msgBox.exec() == QMessageBox::Ok){
        emit sigFirstModelMounted(true);
    }
}

