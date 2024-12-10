#include "maincontroller.h"
#include "statearraycontroller.h"
#include "mainwindow.h"
#include "application_status.h"
#include <thread>
#include <chrono> // for std::chrono::seconds

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
        this->stopAndDestroyProducerConsumers();

        mainWindow->connectDevice(false, Success);
        this->destroyControllers();

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
        msgDisp->enableRxMessageType(MsgTypeIdAcquisitionHeader, false);
        mainWindow->setMessageDispatcher(msgDisp);
    }

    mainWindow->connectDevice(true, ret);
    if (connectionSuccessful) {
        this->onMainWindowCreated();

    } else {
        mainWindow->setConnectionLabel("Connection failed");
        emit startDetecting();
    }
}

void MainController::onMainWindowCreated() {
    consumers.clear();

    /*********\
     * Model *
    \*********/

    appStatus = new ApplicationStatus(msgDisp);
    msgDisp->setChannelSelected(0, true);

    /************\
     * Producer *
    \************/

    deviceDataProducer = new DeviceDataProducer(appStatus);
    auto stampPlotConsumer =  new GapFreePlotConsumer(appStatus, deviceDataProducer);
    consumers.append(stampPlotConsumer);

    /***************\
     * Controllers *
    \***************/

    /*! Plots durations */
    Measurement_t defaultPlotDuration = {2.0, UnitPfxNone, "s"};
    deviceController = new DeviceController(appStatus, mainWindow);
    auto abfDataWriterConsumer = new AbfDataWriterConsumer(appStatus, deviceDataProducer);
    consumers.append(abfDataWriterConsumer);
    bigPlotController = new BigPlotController(appStatus, deviceDataProducer, defaultPlotDuration, mainWindow, abfDataWriterConsumer, deviceController);
    chessboardController = new ChessboardController(appStatus, stampPlotConsumer, defaultPlotDuration, mainWindow);
//    COMPENSATION CONTROLLER MUST BE INITIALIZED BEFORE CONTROLLER CHANNEL
    compensationController = new CompensationController(msgDisp, mainWindow);
    multipleChannelController = new MultipleChannelController(appStatus, mainWindow);
    singleChannelController = new SingleChannelController(appStatus, mainWindow);
    boardController = new BoardController(msgDisp, mainWindow);
    measurementOverviewController = new MeasurementOverviewController(appStatus, deviceDataProducer, mainWindow);
    plotPreferencesController = new PlotPreferencesController(msgDisp, mainWindow);
    if (msgDisp->hasProtocols() == Success) {
        voltageProtocolManager = new ProtocolManager(msgDisp);
        currentProtocolManager = new ProtocolManager(msgDisp);
    }
//    voltageProtocolManager = new ProtocolManager(mDev, e384CommLib::VOLTAGE_CLAMP);
//    currentProtocolManager = new ProtocolManager(mDev, e384CommLib::CURRENT_CLAMP);

//    mainWindow->setProtocolDw(voltageProtocolManager->getProtocolDockWidget());
//    mainWindow->setProtocolDw(currentProtocolManager->getProtocolDockWidget());

    stateArrayController = new StateArrayController(msgDisp, mainWindow);

    /*************\
     * Consumers *
    \*************/

    consumers.append(chessboardController->getPlotConsumer());
    for(auto c: bigPlotController->getControllers()){
        centralWidgetControllers.push_back(c);
    }
    
    consumers.append(measurementOverviewController->getLiveStatisticsConsumer());

    mainWindow->addViewActions();

    /***********\
     * Connect *
    \***********/

    connect(chessboardController, &ChessboardController::sigAllChannelsClicked, this, [=](bool newChannelState) {
        chessboardController->onAllChannelsClicked(newChannelState);
        singleChannelController->onChannelsSelected();
        multipleChannelController->onChannelsSelected();
    });
    connect(chessboardController, &ChessboardController::sigOneBoardClicked, this, [=](uint16_t changedBoardIndex, bool newChannelState) {
        chessboardController->onOneBoardClicked(changedBoardIndex, newChannelState);
        singleChannelController->onChannelsSelected();
        multipleChannelController->onChannelsSelected();
    });
    connect(chessboardController, &ChessboardController::sigOneRowClicked, this, [=](uint16_t changedRowIndex, bool newChannelState) {
        chessboardController->onOneRowClicked(changedRowIndex, newChannelState);
        singleChannelController->onChannelsSelected();
        multipleChannelController->onChannelsSelected();
    });
    connect(chessboardController, &ChessboardController::sigSingleChannelClicked, this, [=](uint16_t changedChannelIndex, QMouseEvent * event) {
        chessboardController->onSingleChannelClicked(changedChannelIndex, event);
        singleChannelController->onChannelsSelected();
        multipleChannelController->onChannelsSelected();
    });

    connect(chessboardController, &ChessboardController::sigAllChannelsClicked,     measurementOverviewController, &MeasurementOverviewController::onChannelsUpdated);
    connect(chessboardController, &ChessboardController::sigOneBoardClicked,        measurementOverviewController, &MeasurementOverviewController::onChannelsUpdated);
    connect(chessboardController, &ChessboardController::sigOneRowClicked,          measurementOverviewController, &MeasurementOverviewController::onChannelsUpdated);
    connect(chessboardController, &ChessboardController::sigSingleChannelClicked,   measurementOverviewController, &MeasurementOverviewController::onChannelsUpdated);

    connect(deviceController, &DeviceController::sigVcCurrentRangeSelected,     this, &MainController::onVcCurrentRangeSelected);
    connect(deviceController, &DeviceController::sigVcVoltageRangeSelected,     this, &MainController::onVcVoltageRangeSelected);
    connect(deviceController, &DeviceController::sigCcCurrentRangeSelected,     this, &MainController::onCcCurrentRangeSelected);
    connect(deviceController, &DeviceController::sigCcVoltageRangeSelected,     this, &MainController::onCcVoltageRangeSelected);
    connect(deviceController, &DeviceController::sigVcVoltageFilterSelected,    this, &MainController::onVcVoltageFilterSelected);
    connect(deviceController, &DeviceController::sigCcCurrentFilterSelected,    this, &MainController::onCcCurrentFilterSelected);
    connect(deviceController, &DeviceController::sigSamplingRateSelected,       this, &MainController::onSamplingRateSelected);
    connect(deviceController, &DeviceController::sigDownsamplingRatioSelected,  this, &MainController::onDownsamplingRatioSelected);
    connect(deviceController, &DeviceController::sigClampingModalitySelected,   this, &MainController::onClampingModalitySelected);
    connect(multipleChannelController, &MultipleChannelController::sigAddRemoveFromBigPlot,             bigPlotController,              &BigPlotController::onExpandTrace);
    connect(multipleChannelController, &MultipleChannelController::sigAddRemoveFromBigPlotEx,           bigPlotController,              &BigPlotController::onExpandTrace);
    connect(multipleChannelController, &MultipleChannelController::sigAddRemoveFromBigPlot,             chessboardController,           &ChessboardController::onTracesExpandedOnOff);
    connect(multipleChannelController, &MultipleChannelController::sigAddRemoveFromBigPlotEx,           chessboardController,           &ChessboardController::onTracesExpandedOnOffEx);
    connect(multipleChannelController, &MultipleChannelController::sigChannelsTurnedOnOff,              chessboardController,           &ChessboardController::onChannelsTurnedOnOff);
    connect(multipleChannelController, &MultipleChannelController::sigChannelsTurnedOnOffEx,            chessboardController,           &ChessboardController::onChannelsTurnedOnOffEx);
    connect(multipleChannelController, &MultipleChannelController::sigCalibrationResistorsTurnedOnOff,  chessboardController,           &ChessboardController::onCalibrationResistorsTurnedOnOff);
    connect(multipleChannelController, &MultipleChannelController::sigStimuliTurnedOnOff,               chessboardController,           &ChessboardController::onStimuliTurnedOnOff);
    connect(multipleChannelController, &MultipleChannelController::sigStimuliTurnedOnOffEx,             chessboardController,           &ChessboardController::onStimuliTurnedOnOffEx);
    connect(multipleChannelController, &MultipleChannelController::sigOffsetRecalibrationTurnedOnOff,   chessboardController,           &ChessboardController::onOffsetRecalibrationTurnedOnOff);
    connect(multipleChannelController, &MultipleChannelController::sigOffsetRecalibrationTurnedOnOff,   measurementOverviewController,  &MeasurementOverviewController::onOffsetRecalibrationResult);
    connect(multipleChannelController, &MultipleChannelController::sigOffsetRecalibrationTurnedOnOff,   singleChannelController,        &SingleChannelController::onOffsetRecalibrationResult);
    connect(multipleChannelController, &MultipleChannelController::sigOffsetRecalibrationResetted,      this, [=] () {
        measurementOverviewController->onOffsetRecalibrationResult(false);
        singleChannelController->onOffsetRecalibrationResult();
    });
    connect(multipleChannelController, &MultipleChannelController::sigLjcTurnedOnOff,                   chessboardController,           &ChessboardController::onLjcTurnedOnOff);
    connect(multipleChannelController, &MultipleChannelController::sigLjcTurnedOnOff,                   measurementOverviewController,  &MeasurementOverviewController::onLiquidJunctionResult);
    connect(multipleChannelController, &MultipleChannelController::sigLjcTurnedOnOff,                   singleChannelController,        &SingleChannelController::onLiquidJunctionResult);
    connect(multipleChannelController, &MultipleChannelController::sigLjResetted,                       this, [=] () {
        measurementOverviewController->onLiquidJunctionResult(false);
        singleChannelController->onLiquidJunctionResult();
    });

    connect(plotPreferencesController, &PlotPreferencesController::sigCurrentColorsChanged, bigPlotController, &BigPlotController::onCurrentColorsChanged);
    connect(plotPreferencesController, &PlotPreferencesController::sigCurrentColorChanged,  bigPlotController, &BigPlotController::onCurrentColorChanged);
    connect(plotPreferencesController, &PlotPreferencesController::sigBackgroundChanged,    bigPlotController, &BigPlotController::onBackgroundColorChanged);

    connect(plotPreferencesController, &PlotPreferencesController::sigCurrentColorsChanged, chessboardController, &ChessboardController::onCurrentColorsChanged);
    connect(plotPreferencesController, &PlotPreferencesController::sigCurrentColorChanged,  chessboardController, &ChessboardController::onCurrentColorChanged);
//    connect(plotPreferencesController, &PlotPreferencesController::sigBackgroundChanged,    chessboardController, &ChessboardController::onBackgroundColorChanged);

    if (msgDisp->hasProtocols() == Success) {
        auto protocolDw = static_cast <ProtocolDockWidget *> (mainWindow->getDockWidget(MainWindow::DWProtocol));
        connect(voltageProtocolManager, &ProtocolManager::protocolStarted,          protocolDw->getVoltageProtocolList(), &ProtocolList::protocolStarted);
        connect(voltageProtocolManager, &ProtocolManager::protocolRequestOutcome,   protocolDw->getVoltageProtocolList(), &ProtocolList::onProtocolRequestOutcome);
        connect(voltageProtocolManager, &ProtocolManager::currentApplied,           protocolDw->getVoltageProtocolList(), &ProtocolList::currentApplied);

        connect(currentProtocolManager, &ProtocolManager::protocolStarted,          protocolDw->getCurrentProtocolList(), &ProtocolList::protocolStarted);
        connect(currentProtocolManager, &ProtocolManager::protocolRequestOutcome,   protocolDw->getCurrentProtocolList(), &ProtocolList::onProtocolRequestOutcome);
        connect(currentProtocolManager, &ProtocolManager::currentApplied,           protocolDw->getCurrentProtocolList(), &ProtocolList::currentApplied);

        connect(protocolDw, &ProtocolDockWidget::startProtocol,    this, [=] () {
            protocolDw->getVoltageProtocolList()->onStartProtocol();
            deviceController->handleProtocolStatusChanged(true);
        });
        connect(protocolDw, &ProtocolDockWidget::restartProtocol,    this, [=] () {
            voltageProtocolManager->onRestartProtocolRequest();
        });
        connect(protocolDw, &ProtocolDockWidget::stopProtocol,     this, [=] () {
            protocolDw->getVoltageProtocolList()->onStopProtocol();
            deviceController->handleProtocolStatusChanged(false);
        });
        connect(protocolDw, &ProtocolDockWidget::startProtocol,    this, [=] () {
            protocolDw->getCurrentProtocolList()->onStartProtocol();
            deviceController->handleProtocolStatusChanged(true);
        });
        connect(protocolDw, &ProtocolDockWidget::stopProtocol,    this, [=] () {
            protocolDw->getCurrentProtocolList()->onStopProtocol();
            deviceController->handleProtocolStatusChanged(false);
        });
        connect(protocolDw->getVoltageProtocolList(), &ProtocolList::startProtocolRequest, voltageProtocolManager, &ProtocolManager::onStartProtocolRequest);
        connect(protocolDw->getVoltageProtocolList(), &ProtocolList::increaseProtocolId,   currentProtocolManager, &ProtocolManager::onIncreaseProtocolId);
        connect(protocolDw->getCurrentProtocolList(), &ProtocolList::startProtocolRequest, currentProtocolManager, &ProtocolManager::onStartProtocolRequest);
        connect(protocolDw->getCurrentProtocolList(), &ProtocolList::increaseProtocolId,   voltageProtocolManager, &ProtocolManager::onIncreaseProtocolId);
    }
    connect(mainWindow, &MainWindow::setDebugBit, this, [=] (int word, int bit, bool flag) {
        msgDisp->setDebugBit(word, bit, flag);
    });
    connect(mainWindow, &MainWindow::setDebugWord, this, [=] (int word, int value) {
        msgDisp->setDebugWord(word, value);
    });
    connect(mainWindow, &MainWindow::sigBoardMappingFileChoosen, this, [=](QString filepath) {
        appStatus->loadChannelMappingFromYaml(filepath.toStdString());
        chessboardController->onBoardMappingLoaded();
        singleChannelController->onBoardMappingLoaded();
        measurementOverviewController->boardMappingsLoaded();
    });

    connect(deviceDataProducer, &DeviceDataProducer::bitRateComputed, this, [=] (double value) {
        if (value > 1.0e6) {
            mainWindow->SRLbl->setText(QString("%1 Msps").arg(value/1.0e6));

        } else {
            mainWindow->SRLbl->setText(QString("%1 ksps").arg(value/1.0e3));
        }
    });

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
    this->startProducerConsumers();

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
}

/*! Message forward from mainController to other consumers */

void MainController::onVcCurrentRangeSelected(int idx) {
    /*! update GUI */
    auto deviceControlDw = static_cast <DeviceControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWDeviceControl));
    deviceControlDw->updateParameters();

    RangedMeasurement_t range;
    msgDisp->getVCCurrentRange(range);
    if (previousCurrentRange.has_value() && previousCurrentRange.value() == range) {
        return;
    }
    previousCurrentRange.emplace(range);

    for (auto consumer : consumers) {
        consumer->onCurrentRangeChanged(range);
    }
    for (auto controller : centralWidgetControllers) {
        controller->onCurrentRangeChanged(range);
    }
    chessboardController->onRangeUpdated(range);
    bigPlotController->onRangeUpdated(range);
    auto singleChannelControlDw = static_cast <SingleChannelControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWSingleChannelControl));
    singleChannelControlDw->onVcCurrentRangeSelected(idx); /*! \todo FCON vedere se questo genere di getXXXDw possono essere sostituite con chiamate ai controller */
}

void MainController::onVcVoltageRangeSelected(int idx) {
    /*! update GUI */
    auto deviceControlDw = static_cast <DeviceControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWDeviceControl));
    deviceControlDw->updateParameters();

    RangedMeasurement_t range;
    msgDisp->getVCVoltageRange(range);
    if (previousVoltageRange.has_value() && previousVoltageRange.value() == range) {
        return;
    }
    previousVoltageRange.emplace(range);

    for (auto consumer : consumers) {
        consumer->onVoltageRangeChanged(range);
    }
    for (auto controller : centralWidgetControllers) {
        controller->onVoltageRangeChanged(range);
    }
    //this should be useless?
    //chessboardController->onRangeUpdated(range, QwtPlot::yRight);
    bigPlotController->onRangeUpdated(range);
    auto singleChannelControlDw = static_cast <SingleChannelControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWSingleChannelControl));
    singleChannelControlDw->onVcVoltageRangeSelected(idx); /*! \todo FCON vedere se questo genere di getXXXDw possono esseresostittuite con chiamate ai controller */
}

void MainController::onCcCurrentRangeSelected(int idx) {
    /*! update GUI */
    auto deviceControlDw = static_cast <DeviceControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWDeviceControl));
    deviceControlDw->updateParameters();

    RangedMeasurement_t range;
    msgDisp->getCCCurrentRange(range);
    if (previousCurrentRange.has_value() && previousCurrentRange.value() == range) {
        return;
    }
    previousCurrentRange.emplace(range);

    for (auto consumer : consumers) {
        consumer->onCurrentRangeChanged(range);
    }
    for (auto controller : centralWidgetControllers) {
        controller->onCurrentRangeChanged(range);
    }
    chessboardController->onRangeUpdated(range);
    bigPlotController->onRangeUpdated(range);
    auto singleChannelControlDw = static_cast <SingleChannelControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWSingleChannelControl));
    singleChannelControlDw->onCcCurrentRangeSelected(idx);
}

void MainController::onCcVoltageRangeSelected(int idx) {
    /*! update GUI */
    auto deviceControlDw = static_cast <DeviceControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWDeviceControl));
    deviceControlDw->updateParameters();

    RangedMeasurement_t range;
    msgDisp->getCCVoltageRange(range);
    if (previousVoltageRange.has_value() && previousVoltageRange.value() == range) {
        return;
    }
    previousVoltageRange.emplace(range);

    for (auto consumer : consumers) {
        consumer->onVoltageRangeChanged(range);
    }
    for (auto controller : centralWidgetControllers) {
        controller->onVoltageRangeChanged(range);
    }
    //this should be useless?
    //chessboardController->onRangeUpdated(range, QwtPlot::yRight);
    bigPlotController->onRangeUpdated(range);
    auto singleChannelControlDw = static_cast <SingleChannelControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWSingleChannelControl));
    singleChannelControlDw->onCcVoltageRangeSelected(idx); /*! \todo FCON vedere se questo genere di getXXXDw possono esseresostittuite con chiamate ai controller */
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

    for (auto consumer : consumers) {
        consumer->onSamplingRateChanged(meas);
    }
    for (auto controller : centralWidgetControllers) {
        controller->onSamplingRateChanged(meas);
    }
}

void MainController::onDownsamplingRatioSelected(int) {
    uint32_t ratio;
    msgDisp->getDownsamplingRatio(ratio);

    for (auto consumer : consumers) {
        consumer->onDownsamplingRatioChanged(ratio);
    }
    for (auto controller : centralWidgetControllers) {
        controller->onDownsamplingRatioChanged(ratio);
    }
}

void MainController::onClampingModalitySelected(ClampingModality_t mode) {
    /*! update GUI */
    auto deviceControlDw = static_cast <DeviceControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWDeviceControl));
    deviceControlDw->updateParameters();

    auto protocolDw = static_cast <ProtocolDockWidget *> (mainWindow->getDockWidget(MainWindow::DWProtocol));
    protocolDw->onSetClampingModality(mode);

    auto multipleChannelDw = static_cast <MultipleChannelControlDockWidget *> (mainWindow->getDockWidget(MainWindow::DWMultipleChannelControl));
    multipleChannelDw->onSetClampingModality(mode);
}

void MainController::startProducerConsumers() {
    deviceDataProducer->start();
}

void MainController::stopAndDestroyProducerConsumers() {
    for (auto consumer : consumers) {
        consumer->onStopConsuming();
    }
    for (auto controller : centralWidgetControllers) {
        controller->onStopConsuming();
    }
    if (deviceDataProducer!= nullptr) {
        deviceDataProducer->onStopProducing();
        delete deviceDataProducer;
        deviceDataProducer = nullptr;
    }
    consumers.clear();
    centralWidgetControllers.clear();
}
