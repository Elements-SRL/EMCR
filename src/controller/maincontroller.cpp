#include "maincontroller.h"
#include "statearraycontroller.h"
#include "mainwindow.h"
#include "application_status.h"

MainController::MainController() {
    /*! Set up device detector */
    deviceDetector = new DeviceDetector;
    deviceDetector->moveToThread(&deviceDetectorThread);

    connect(this, &MainController::startDetecting, deviceDetector, &DeviceDetector::onStartDetecting);
    connect(this, &MainController::stopDetecting, deviceDetector, &DeviceDetector::onStopDetecting);

    deviceDetectorThread.start();

    /*! Set up device connector */
    deviceConnector = new DeviceConnector;

    connect(deviceConnector, &DeviceConnector::deviceConnected, this, &MainController::onDeviceConnected);

    upgradeFwController = new UpgradeFwController;

    setMainWindow(new MainWindow());
}

MainController::~MainController() {
    deviceDetectorThread.quit();
    deviceDetectorThread.wait();

    this->onConnect(false);

    delete mainWindow;

    if (deviceDetector != nullptr) {
        delete deviceDetector;
        deviceDetector = nullptr;
    }

    if (deviceConnector != nullptr) {
        delete deviceConnector;
        deviceConnector = nullptr;
        /*! The messageDispatcher is destroyed by the deviceConnector */
        msgDisp = nullptr;
    }

    if (upgradeFwController != nullptr) {
        delete upgradeFwController;
        upgradeFwController = nullptr;
    }
}

void MainController::setMainWindow(MainWindow * mainWindow) {
    this->mainWindow = mainWindow;

    connect(deviceDetector, &DeviceDetector::devicesListChanged, this, &MainController::onDevicesListChanged);
    connect(mainWindow->getConnectButton(), &QPushButton::clicked, this, &MainController::onConnect);
    connect(mainWindow, &MainWindow::sigUpgradeFw, this, &MainController::onUpgradeFw);
    connect(mainWindow, &MainWindow::sigResetHw, this, &MainController::onResetHw);

    mainWindow->show();
    emit startDetecting();
}

void MainController::setSplash(SplashView *s){
    this->splash = s;
}

void MainController::onDevicesListChanged(std::vector <std::string> devicesList) {
    this->mainWindow->setDevicesList(devicesList);
    if (devicesList.size() > 0) {
        if (msgDisp != nullptr) {
            std::string sn;
            msgDisp->getSerialNumber(sn);
            QString connectedDeviceName = QString::fromStdString(sn);

            int connectedDeviceIdx = -1;
            for (unsigned int idx = 0; idx < devicesList.size(); idx++) {
                QString deviceName = QString::fromStdString(devicesList[idx]);
                if (connectedDeviceName == deviceName) {
                    connectedDeviceIdx = (int)idx;
                }
            }

            if (connectedDeviceIdx >= 0) {
                mainWindow->setConnectedDeviceIdx(connectedDeviceIdx);

            } else {
                this->destroyControllers();
                mainWindow->connectDevice(false, Success);
            }
        }
    }
}

void MainController::onConnect(bool flag) {
    emit stopDetecting();
    QString serial = mainWindow->getSelectedSerialNumber();

    if (flag) {
        mainWindow->setConnectionLabel("Connecting, please wait...");
        deviceConnector->setDeviceId(serial);
        deviceConnector->start();

    } else {
        previousVoltageRange.reset();
        previousCurrentRange.reset();

        mainWindow->setConnectionLabel("");
        mainWindow->connectDevice(false, Success);
        this->stopAndDestroyProducerConsumers();

        if (appStatus != nullptr) {
            delete appStatus;
            appStatus = nullptr;
        }

        if (msgDisp != nullptr) {
            msgDisp->disconnectDevice();
            deviceConnector->destroyMessageDispatcher();
            msgDisp = nullptr;
            mainWindow->setMessageDispatcher(msgDisp);
        }

        emit startDetecting();
    }
}

void MainController::onUpgradeFw() {
    upgradeFwController->openView(mainWindow->getSelectedSerialNumber());
}

void MainController::onResetHw() {
    msgDisp->resetAsic(true);
    QThread::msleep(10);
    msgDisp->resetAsic(false);
}

void MainController::onDeviceConnected(ErrorCodes_t ret) {
    bool connectionSuccessful = ret == Success;
    if (connectionSuccessful) {
        mainWindow->setConnectionLabel("");
        msgDisp = deviceConnector->getMessageDispatcher();
        msgDisp->getChannelNumberFeatures(voltageChannelsNumber, currentChannelsNumber);
        msgDisp->getBoardsNumberFeatures(boardsNumber);
        mainWindow->setMessageDispatcher(msgDisp);
    }

    mainWindow->connectDevice(true, ret);
    if (connectionSuccessful) {
        this->onMainWindowCreated();
        mainWindow->restoreUISettings();

    } else {
        mainWindow->setConnectionLabel("Connection failed");
        emit startDetecting();
    }
}

void MainController::onMainWindowCreated() {
    /*********\
     * Model *
    \*********/

    appStatus = new ApplicationStatus(msgDisp);
    appStatus->setChannelSelected(0, true);

    /************\
     * Producer *
    \************/

    deviceDataProducer = new DeviceDataProducer(appStatus);

    /*! Plots durations */
    Measurement_t defaultPlotDuration = {2.0, UnitPfxNone, "s"};
    deviceController = new DeviceController(appStatus, mainWindow);
    bigPlotController = new BigPlotController(appStatus, deviceDataProducer, defaultPlotDuration, mainWindow, deviceController);
    chessboardController = new ChessboardController(appStatus, deviceDataProducer, defaultPlotDuration, mainWindow);
    controllersWithConsumer.push_back(chessboardController);
//    COMPENSATION CONTROLLER MUST BE INITIALIZED BEFORE CONTROLLER CHANNEL
    compensationController = new CompensationController(msgDisp, mainWindow);
    multipleChannelController = new MultipleChannelController(appStatus, mainWindow);
    singleChannelController = new SingleChannelController(appStatus, mainWindow);
    boardController = new BoardController(msgDisp, mainWindow);
    measurementOverviewController = new MeasurementOverviewController(appStatus, deviceDataProducer, mainWindow);
    controllersWithConsumer.push_back(measurementOverviewController);
    plotPreferencesController = new PlotPreferencesController(msgDisp, mainWindow);
    if (msgDisp->hasProtocols() == Success) {
        voltageProtocolManager = new ProtocolManager(msgDisp);
        currentProtocolManager = new ProtocolManager(msgDisp);
    }

    autoDecloggerController = new AutoDecloggerController(appStatus, mainWindow, deviceDataProducer);
    controllersWithConsumer.push_back(autoDecloggerController);

    std::vector <std::string> temperatureNames;
    std::vector <e384cl::RangedMeasurement_t> temperatureRanges;
    if (appStatus->getTemperatureChannelsNum() > 0) {
        temperatureController = new TemperatureController(appStatus, mainWindow);
    }

    if (debugControlsEnabled()) {
        debugController = new DebugController(appStatus, mainWindow);
    }

    stateArrayController = new StateArrayController(msgDisp, mainWindow);

    plotDetailController = new PlotDetailController(appStatus, defaultPlotDuration, mainWindow, multipleChannelController, chessboardController, deviceDataProducer);
    /***************\
     * Controllers *
    \***************/

    for (auto &c: bigPlotController->getControllers()) {
        controllersWithConsumer.push_back(c);
    }
    controllersWithConsumer.push_back(plotDetailController);
    mainWindow->addViewActions();

    /***********\
     * Connect *
    \***********/
    chessboardController->connectSingleChannelController(singleChannelController);
    chessboardController->connectMultipleChannelController(multipleChannelController);
    chessboardController->connectMeasurementOverviewController(measurementOverviewController);

    connect(deviceController, &DeviceController::sigVcCurrentRangeSelected,     this, &MainController::onVcCurrentRangeSelected);
    connect(deviceController, &DeviceController::sigVcVoltageRangeSelected,     this, &MainController::onVcVoltageRangeSelected);
    connect(deviceController, &DeviceController::sigCcCurrentRangeSelected,     this, &MainController::onCcCurrentRangeSelected);
    connect(deviceController, &DeviceController::sigCcVoltageRangeSelected,     this, &MainController::onCcVoltageRangeSelected);
    connect(deviceController, &DeviceController::sigVcVoltageFilterSelected,    this, &MainController::onVcVoltageFilterSelected);
    connect(deviceController, &DeviceController::sigCcCurrentFilterSelected,    this, &MainController::onCcCurrentFilterSelected);
    connect(deviceController, &DeviceController::sigSamplingRateSelected,       this, &MainController::onSamplingRateSelected);
    connect(deviceController, &DeviceController::sigDownsamplingRatioSelected,  this, &MainController::onDownsamplingRatioSelected);
    connect(deviceController, &DeviceController::sigClampingModalitySelected,   this, &MainController::onClampingModalitySelected);

    multipleChannelController->connectChessboardController(chessboardController);
    multipleChannelController->connectSingleChannelController(singleChannelController);
    multipleChannelController->connectBigPlotController(bigPlotController);
    multipleChannelController->connectPlotDetailController(plotDetailController);
    multipleChannelController->connectMeasurementOverviewController(measurementOverviewController);

    plotPreferencesController->connectBigPlotController(bigPlotController);
    plotPreferencesController->connectPlotDetailController(plotDetailController);
    plotPreferencesController->connectChessboardController(chessboardController);

    if (msgDisp->hasProtocols() == Success) {
        auto protocolDw = static_cast <ProtocolDockWidget *> (mainWindow->getDockWidget(MainWindow::DWProtocol));
        connect(voltageProtocolManager, &ProtocolManager::protocolRequestOutcome,   protocolDw->getVoltageProtocolList(), &ProtocolList::onProtocolRequestOutcome);
        connect(voltageProtocolManager, &ProtocolManager::currentApplied,           protocolDw->getVoltageProtocolList(), &ProtocolList::currentApplied);

        connect(currentProtocolManager, &ProtocolManager::protocolRequestOutcome,   protocolDw->getCurrentProtocolList(), &ProtocolList::onProtocolRequestOutcome);
        connect(currentProtocolManager, &ProtocolManager::currentApplied,           protocolDw->getCurrentProtocolList(), &ProtocolList::currentApplied);

        connect(voltageProtocolManager, &ProtocolManager::protocolStarted, bigPlotController, &BigPlotController::onProtocolStarted);
        connect(currentProtocolManager, &ProtocolManager::protocolStarted, bigPlotController, &BigPlotController::onProtocolStarted);

        connect(voltageProtocolManager, &ProtocolManager::protocolStarted, measurementOverviewController, &MeasurementOverviewController::onProtocolStarted);
        connect(currentProtocolManager, &ProtocolManager::protocolStarted, measurementOverviewController, &MeasurementOverviewController::onProtocolStarted);

        for (auto controller : bigPlotController->getControllers()) {
            connect(voltageProtocolManager, &ProtocolManager::protocolStarted, controller, &CentralWidgetController::onProtocolStarted);
            connect(currentProtocolManager, &ProtocolManager::protocolStarted, controller, &CentralWidgetController::onProtocolStarted);
        }

        connect(protocolDw, &ProtocolDockWidget::startProtocol,    this, [=] () {
            protocolDw->getVoltageProtocolList()->onStartProtocol();
            protocolDw->getAnalysisVoltageProtocolList()->onStartProtocol();
            protocolDw->getCurrentProtocolList()->onStartProtocol();
            protocolDw->getAnalysisCurrentProtocolList()->onStartProtocol();
            deviceController->handleProtocolStatusChanged(true);
        });
        connect(protocolDw, &ProtocolDockWidget::restartProtocol,    this, [=] () {
            voltageProtocolManager->onRestartProtocolRequest();
            currentProtocolManager->onRestartProtocolRequest();
        });
        connect(protocolDw, &ProtocolDockWidget::stopProtocol,     this, [=] () {
            protocolDw->getVoltageProtocolList()->onStopProtocol();
            protocolDw->getCurrentProtocolList()->onStopProtocol();
            deviceController->handleProtocolStatusChanged(false);
        });
        connect(protocolDw->getVoltageProtocolList(), &ProtocolList::startProtocolRequest, voltageProtocolManager, &ProtocolManager::onStartProtocolRequest);
        connect(protocolDw->getVoltageProtocolList(), &ProtocolList::increaseProtocolId,   currentProtocolManager, &ProtocolManager::onIncreaseProtocolId);
        connect(protocolDw->getAnalysisVoltageProtocolList(), &ProtocolList::startProtocolRequest, voltageProtocolManager, &ProtocolManager::onStartProtocolRequest);
        connect(protocolDw->getAnalysisVoltageProtocolList(), &ProtocolList::increaseProtocolId,   currentProtocolManager, &ProtocolManager::onIncreaseProtocolId);
        connect(protocolDw->getCurrentProtocolList(), &ProtocolList::startProtocolRequest, currentProtocolManager, &ProtocolManager::onStartProtocolRequest);
        connect(protocolDw->getCurrentProtocolList(), &ProtocolList::increaseProtocolId,   voltageProtocolManager, &ProtocolManager::onIncreaseProtocolId);
        connect(protocolDw->getAnalysisCurrentProtocolList(), &ProtocolList::startProtocolRequest, currentProtocolManager, &ProtocolManager::onStartProtocolRequest);
        connect(protocolDw->getAnalysisCurrentProtocolList(), &ProtocolList::increaseProtocolId,   voltageProtocolManager, &ProtocolManager::onIncreaseProtocolId);
    }
    connect(mainWindow, &MainWindow::sigBoardMappingFileChoosen, this, [=](QString filepath) {
        appStatus->loadChannelMappingFromYaml(filepath.toStdString());
        chessboardController->onBoardMappingLoaded();
        singleChannelController->onBoardMappingLoaded();
        measurementOverviewController->boardMappingsLoaded();
    });

    if (temperatureController != nullptr) {
        connect(deviceDataProducer, &DeviceDataProducer::sigTemperatureRead, temperatureController, &TemperatureController::onTemperatureRead);
    }
    connect(deviceDataProducer, &DeviceDataProducer::bitRateComputed, mainWindow, &MainWindow::onBitRateComputed);

    chessboardController->onDurationUpdated(defaultPlotDuration);
    RangedMeasurement plotRange = {0, defaultPlotDuration.value, 1, defaultPlotDuration.prefix, defaultPlotDuration.unit};
    bigPlotController->onRangeUpdated(plotRange);

    /*! Forced initialization at start */
    auto deviceControlDw = static_cast <DeviceControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWDeviceControl));
    deviceControlDw->forceEmit();

    /*! \todo FCON questo potrebbe essere parametrizzato */

    std::vector<uint16_t> channelIndexes(currentChannelsNumber);
    for(int i = 0; i < currentChannelsNumber; i++){
        channelIndexes[i] = i;
    }

    plotPreferencesController->initializePlotColors();

    /*! Start threads */
    this->startProducer();

    multipleChannelController->onChannelsSelected();

    //for devices with less then 16 channels the traces are expanded by default
    multipleChannelController->addRemoveFromBigPlot(true);
}

void MainController::destroyControllers() {
    if (singleChannelController != nullptr) {
        delete singleChannelController;
        singleChannelController = nullptr;
    }

    if (multipleChannelController != nullptr) {
        delete multipleChannelController;
        multipleChannelController = nullptr;
    }

    if (chessboardController != nullptr) {
        delete chessboardController;
        chessboardController = nullptr;
    }

    if (compensationController != nullptr) {
        delete compensationController;
        compensationController = nullptr;
    }

    if (measurementOverviewController != nullptr) {
        delete measurementOverviewController;
        measurementOverviewController = nullptr;
    }

    if (plotPreferencesController != nullptr) {
        delete plotPreferencesController;
        plotPreferencesController = nullptr;
    }

    if (stateArrayController != nullptr) {
        delete stateArrayController;
        stateArrayController = nullptr;
    }

    if (boardController != nullptr) {
        delete boardController;
        boardController = nullptr;
    }

    if (deviceController != nullptr) {
        delete deviceController;
        deviceController = nullptr;
    }

    if (voltageProtocolManager != nullptr) {
        delete voltageProtocolManager;
        voltageProtocolManager = nullptr;
    }

    if (currentProtocolManager != nullptr) {
        delete currentProtocolManager;
        currentProtocolManager = nullptr;
    }

    if (bigPlotController != nullptr) {
        delete bigPlotController;
        bigPlotController = nullptr;
    }

    if (autoDecloggerController != nullptr) {
        delete autoDecloggerController;
        autoDecloggerController = nullptr;
    }

    if (debugController != nullptr) {
        delete debugController;
        debugController = nullptr;
    }

    if (temperatureController != nullptr) {
        delete temperatureController;
        temperatureController = nullptr;
    }

    if (plotDetailController != nullptr) {
        delete plotDetailController;
        plotDetailController = nullptr;
    }
}

void MainController::onVcCurrentRangeSelected() {
    /*! update GUI */
    auto deviceControlDw = static_cast <DeviceControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWDeviceControl));
    deviceControlDw->updateParameters();

    auto range = appStatus->getCurrentRanges();
    if (previousCurrentRange.has_value() && previousCurrentRange.value() == range) {
        return;
    }
    previousCurrentRange.emplace(range);

    for (auto controller : controllersWithConsumer) {
        controller->onCurrentRangeChanged();
    }

    chessboardController->onRangeUpdated(appStatus->getCurrentRanges());
    bigPlotController->onRangeUpdated(appStatus->getMaxCurrentRange());
    auto singleChannelControlDw = static_cast <SingleChannelControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWSingleChannelControl));
    singleChannelControlDw->onVcCurrentRangeSelected(); /*! \todo FCON vedere se questo genere di getXXXDw possono essere sostituite con chiamate ai controller */
}

void MainController::onVcVoltageRangeSelected() {
    /*! update GUI */
    auto deviceControlDw = static_cast <DeviceControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWDeviceControl));
    deviceControlDw->updateParameters();

    auto range = appStatus->getVoltageRanges();
    if (previousVoltageRange.has_value() && previousVoltageRange.value() == range) {
        return;
    }
    previousVoltageRange.emplace(range);

    for (auto controller : controllersWithConsumer) {
        controller->onVoltageRangeChanged();
    }

    bigPlotController->onRangeUpdated(appStatus->getMaxVoltageRange());
    auto singleChannelControlDw = static_cast <SingleChannelControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWSingleChannelControl));
    singleChannelControlDw->onVcVoltageRangeSelected(); /*! \todo FCON vedere se questo genere di getXXXDw possono esseresostittuite con chiamate ai controller */
}

void MainController::onCcCurrentRangeSelected() {
    /*! update GUI */
    auto deviceControlDw = static_cast <DeviceControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWDeviceControl));
    deviceControlDw->updateParameters();

    auto range = appStatus->getCurrentRanges();
    if (previousCurrentRange.has_value() && previousCurrentRange.value() == range) {
        return;
    }
    previousCurrentRange.emplace(range);

    for (auto controller : controllersWithConsumer) {
        controller->onCurrentRangeChanged();
    }
    bigPlotController->onRangeUpdated(appStatus->getMaxCurrentRange());
    auto singleChannelControlDw = static_cast <SingleChannelControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWSingleChannelControl));
    singleChannelControlDw->onCcCurrentRangeSelected();
}

void MainController::onCcVoltageRangeSelected() {
    /*! update GUI */
    auto deviceControlDw = static_cast <DeviceControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWDeviceControl));
    deviceControlDw->updateParameters();

    auto range = appStatus->getVoltageRanges();
    if (previousVoltageRange.has_value() && previousVoltageRange.value() == range) {
        return;
    }
    previousVoltageRange.emplace(range);

    for (auto controller : controllersWithConsumer) {
        controller->onVoltageRangeChanged();
    }

    chessboardController->onRangeUpdated(appStatus->getVoltageRanges());
    bigPlotController->onRangeUpdated(appStatus->getMaxVoltageRange());
    auto singleChannelControlDw = static_cast <SingleChannelControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWSingleChannelControl));
    singleChannelControlDw->onCcVoltageRangeSelected(); /*! \todo FCON vedere se questo genere di getXXXDw possono esseresostittuite con chiamate ai controller */
}

void MainController::onVcVoltageFilterSelected(int) {
    /*! update GUI */
    auto deviceControlDw = static_cast <DeviceControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWDeviceControl));
    deviceControlDw->updateParameters();
}

void MainController::onCcCurrentFilterSelected(int) {
    /*! update GUI */
    auto deviceControlDw = static_cast <DeviceControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWDeviceControl));
    deviceControlDw->updateParameters();
}

void MainController::onSamplingRateSelected(int) {
    /*! update GUI */
    auto deviceControlDw = static_cast <DeviceControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWDeviceControl));
    deviceControlDw->updateParameters();

    Measurement_t meas;
    msgDisp->getSamplingRate(meas);

    for (auto controller : controllersWithConsumer) {
        controller->onSamplingRateChanged(meas);
    }
}

void MainController::onDownsamplingRatioSelected(int) {
    uint32_t ratio;
    msgDisp->getDownsamplingRatio(ratio);

    for (auto controller : controllersWithConsumer) {
        controller->onDownsamplingRatioChanged(ratio);
    }
}

void MainController::onClampingModalitySelected(ClampingModality_t mode) {
    /*! update GUI */
    auto deviceControlDw = static_cast <DeviceControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWDeviceControl));
    deviceControlDw->updateParameters();

    auto protocolDw = static_cast <ProtocolDockWidget *> (mainWindow->getDockWidget(MainWindow::DWProtocol));
    if (protocolDw != nullptr) {
        protocolDw->onSetClampingModality(mode);
    }

    auto multipleChannelDw = static_cast <MultipleChannelControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWMultipleChannelControl));
    multipleChannelDw->onSetClampingModality(mode);
}

void MainController::startProducer() {
    deviceDataProducer->start();
}

void MainController::stopAndDestroyProducerConsumers() {
    for (auto& controller : controllersWithConsumer) {
        controller->onStopConsuming();
    }
    this->destroyControllers();
    if (deviceDataProducer != nullptr) {
        deviceDataProducer->onStopProducing();
        delete deviceDataProducer;
        deviceDataProducer = nullptr;
    }
    controllersWithConsumer.clear();
}
