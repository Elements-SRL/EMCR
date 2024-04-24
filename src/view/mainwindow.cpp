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
#include "aboutdialog.h"
#include "supportdialog.h"
#include "deviceinfodialog.h"

MainWindow::MainWindow(QWidget * parent) :
    QMainWindow(parent) {

    /*! This line ensures that showMaximized() in main.cpp works */
    this->setGeometry(0, 0, 800, 600);
    this->setObjectName("mainWindow");

    this->showMaximized();
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

    /*! ? menu */
    menuQuestionMark = new QMenu("?");
    menuBar->addMenu(menuQuestionMark);

    actionAbout = new QAction("About");
    connect(actionAbout, &QAction::triggered, this, &MainWindow::onAbout);
    menuQuestionMark->addAction(actionAbout);

    actionDeviceInfo = new QAction("Device info");
    connect(actionDeviceInfo, &QAction::triggered, this, &MainWindow::onDeviceInfo);
    menuQuestionMark->addAction(actionDeviceInfo);

    actionSupport = new QAction("Support");
    connect(actionSupport, &QAction::triggered, this, &MainWindow::onSupport);
    menuQuestionMark->addAction(actionSupport);

    connect(actionUpgradeFw, &QAction::triggered, this, &MainWindow::sigUpgradeFw);

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

    connectionInfoLbl = new QLabel("");
    deviceDetectorHl->addWidget(connectionInfoLbl);

    SRLbl = new QLabel;
    deviceDetectorHl->addWidget(SRLbl);

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    deviceDetectorHl->addWidget(spacer);
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

ChessboardDockWidget * MainWindow::getChessboardDockWidget() {
    return chessboardDw;
}

DeviceControlDockWidget * MainWindow::getDeviceControlsDockWidget() {
    return deviceControlsDw;
}

SingleChannelControlDockWidget * MainWindow::getSingleChannelControlsDockWidget() {
    return singleChannelControlsDw;
}

MultipleChannelControlDockWidget * MainWindow::getMultipleChannelControlsDockWidget() {
    return multipleChannelControlsDw;
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

StateArrayDockWidget * MainWindow::getStateArrayDockWidget(){
    return stateArrayDockWidget;
}

MeasurementsOverviewDockWidget * MainWindow::getMeasurementOverviewDockWidget() {
    return measurementsOverviewDw;
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

void MainWindow::setIvGraphWidget(IvGraphWidget * widget) {
    ivGraphWidget = widget;
    if (widget != nullptr) {
        addDockWidget(Qt::LeftDockWidgetArea, ivGraphWidget);
        ivGraphWidget->setFloating(true);
        dockWidgets.append(ivGraphWidget);
    }
}

void MainWindow::setChessboardDw(ChessboardDockWidget * widget) {
    chessboardDw = widget;
    if (widget != nullptr) {
        addDockWidget(Qt::BottomDockWidgetArea, chessboardDw);
        chessboardDw->setFloating(true);
        dockWidgets.append(chessboardDw);
    }
}

void MainWindow::setCompensationControlsDw(CompensationControlDockWidget * widget){
    compensationControlsDw = widget;
    if (widget != nullptr) {
        addDockWidget(Qt::RightDockWidgetArea, compensationControlsDw);
        dockWidgets.append(compensationControlsDw);
    }
}

void MainWindow::setSingleChannelControlsDw(SingleChannelControlDockWidget * widget){
    singleChannelControlsDw = widget;
    if (widget != nullptr) {
        addDockWidget(Qt::RightDockWidgetArea, singleChannelControlsDw);
        dockWidgets.append(singleChannelControlsDw);
    }
}

void MainWindow::setMultipleChannelControlsDw(MultipleChannelControlDockWidget * widget){
    multipleChannelControlsDw = widget;
    if (widget != nullptr) {
        addDockWidget(Qt::RightDockWidgetArea, multipleChannelControlsDw);
        dockWidgets.append(multipleChannelControlsDw);
    }
}

void MainWindow::setBoardControlsDw(BoardControlDockWidget * widget){
    boardControlsDw = widget;
    if (widget != nullptr) {
        addDockWidget(Qt::RightDockWidgetArea, boardControlsDw);
        dockWidgets.append(boardControlsDw);
    }
}

void MainWindow::setDeviceControlDw(DeviceControlDockWidget * widget){
    deviceControlsDw = widget;
    if (widget != nullptr) {
        addDockWidget(Qt::RightDockWidgetArea, deviceControlsDw);
        dockWidgets.append(deviceControlsDw);
    }
}

void MainWindow::setStateArrayDw(StateArrayDockWidget * widget){
    stateArrayDockWidget = widget;
    if (widget != nullptr) {
        addDockWidget(Qt::RightDockWidgetArea, stateArrayDockWidget);
        dockWidgets.append(stateArrayDockWidget);
    }
}

void MainWindow::setMeasurementOverviewDw(MeasurementsOverviewDockWidget * widget) {
    measurementsOverviewDw = widget;
    if (widget != nullptr) {
        addDockWidget(Qt::RightDockWidgetArea, measurementsOverviewDw);
        measurementsOverviewDw->setFloating(true);
        dockWidgets.append(measurementsOverviewDw);
    }
}

void MainWindow::setPlotPreferencesDialog(PlotPreferencesDialog * ppd) {
    plotPreferencesDlg = ppd;
    connect(actionPlotPreferences, &QAction::triggered, plotPreferencesDlg, &PlotPreferencesDialog::exec);
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

/******************\
 * protocols dock *
\******************/

void MainWindow::setProtocolDw(ProtocolDockWidget * pdw){
    protocolDw = pdw;
    addDockWidget(Qt::LeftDockWidgetArea, protocolDw);
    dockWidgets.append(protocolDw);
}

void MainWindow::createGuiControls() {
    QSettings settings;

    this->setStyleSheet("QSplitter::handle{image: url(:/imgs/splitter handle.png)}");

    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);

    dockWidgets.clear();

    this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    /******************\
     * protocols dock *
    \******************/

    if (msgDisp->hasProtocols() == Success) {
        protocolDw = new ProtocolDockWidget(msgDisp, e384CommLib::VOLTAGE_CLAMP, this);
        protocolDw->setObjectName("protocolDw");
        this->addDockWidget(Qt::LeftDockWidgetArea, protocolDw);
        dockWidgets.append(protocolDw);
    }

#ifndef GLB_HIDE_DEBUG_CTRLS

    /**************\
     * debug dock *
    \**************/

    debugDw = new QDockWidget();
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

    QCheckBox * debugRangeWordChx = new QCheckBox("Enable word range");
    debugVl->addWidget(debugRangeWordChx);

    QSpinBox * debugLastWordSbx = new QSpinBox;
    debugLastWordSbx->setRange(0, 32767);
    debugLastWordSbx->setValue(0);
    debugLastWordSbx->setEnabled(false);
    debugVl->addWidget(debugLastWordSbx);

    connect(debugRangeWordChx, &QCheckBox::clicked, debugLastWordSbx, &QWidget::setEnabled);

    debugVl->addWidget(new QLabel("Bit"));
    QSpinBox * debugBitSbx = new QSpinBox;
    debugBitSbx->setRange(0, 15);
    debugBitSbx->setValue(0);
    debugVl->addWidget(debugBitSbx);

    QCheckBox * debugRangeBitChx = new QCheckBox("Enable bit range");
    debugVl->addWidget(debugRangeBitChx);

    QSpinBox * debugLastBitSbx = new QSpinBox;
    debugLastBitSbx->setRange(0, 15);
    debugLastBitSbx->setValue(0);
    debugLastBitSbx->setEnabled(false);
    debugVl->addWidget(debugLastBitSbx);

    connect(debugRangeBitChx, &QCheckBox::clicked, debugLastBitSbx, &QWidget::setEnabled);

    QCheckBox * debugStatusChx = new QCheckBox("Status");
    debugVl->addWidget(debugStatusChx);

    QPushButton * debugApplyBtn = new QPushButton("Apply bit");
    debugApplyBtn->setCheckable(false);
    debugVl->addWidget(debugApplyBtn);

    connect(debugApplyBtn, &QPushButton::clicked, this, [=] () {
        if (debugRangeWordChx->isChecked()) {
            for (int wordIdx = debugWordSbx->value(); wordIdx <= debugLastWordSbx->value(); wordIdx++) {
                if (debugRangeBitChx->isChecked()) {
                    for (int bitIdx = debugBitSbx->value(); bitIdx <= debugLastBitSbx->value(); bitIdx++) {
                        emit setDebugBit(wordIdx, bitIdx, debugStatusChx->isChecked());
                    }

                } else {
                    emit setDebugBit(wordIdx, debugBitSbx->value(), debugStatusChx->isChecked());
                }
            }

        } else {
            if (debugRangeBitChx->isChecked()) {
                for (int bitIdx = debugBitSbx->value(); bitIdx <= debugLastBitSbx->value(); bitIdx++) {
                    emit setDebugBit(debugWordSbx->value(), bitIdx, debugStatusChx->isChecked());
                }

            } else {
                emit setDebugBit(debugWordSbx->value(), debugBitSbx->value(), debugStatusChx->isChecked());
            }
        }
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
        if (debugRangeWordChx->isChecked()) {
            for (int wordIdx = debugWordSbx->value(); wordIdx <= debugLastWordSbx->value(); wordIdx++) {
                emit setDebugWord(wordIdx, debugValueSbx->value());
            }

        } else {
            emit setDebugWord(debugWordSbx->value(), debugValueSbx->value());
        }
    });
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

    if (protocolDw != nullptr){
        delete protocolDw;
        protocolDw = nullptr;
    }

    if (debugDw != nullptr){
        delete debugDw;
        protocolDw = nullptr;
    }

    for (int dockIdx = 0; dockIdx < analysisWidgets.size(); dockIdx++) {
        if (analysisWidgets[dockIdx] != nullptr) {
            delete analysisWidgets[dockIdx];
            analysisWidgets[dockIdx] = nullptr;
        }
    }
    analysisWidgets.clear();

    this->takeCentralWidget();

    SRLbl->setText("");

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
}

void MainWindow::restoreUISettings() {
    QTimer * timer = new QTimer;
    timer->setInterval(200);
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

        tag = settingsRoot + bigPlotW->objectName() + "/geometry";
        if (settings.contains(tag)) {
            bigPlotW->setGeometry(settings.value(tag).value <QRect> ());
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

    tag = settingsRoot + bigPlotW->objectName() + "/geometry";
    settings.setValue(tag, QVariant(bigPlotW->geometry()));

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

void MainWindow::onAbout() {
    AboutDialog a(this);
    a.exec();
}

void MainWindow::onDeviceInfo() {
    DeviceInfoDialog a(msgDisp != nullptr, devicesComboBox->currentText(), this);
    a.exec();
}

void MainWindow::onSupport() {
    SupportDialog a(this);
    a.exec();
}
