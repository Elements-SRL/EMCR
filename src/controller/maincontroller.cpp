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
    dataWriterConsumers.clear();

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

    bigPlotController = new BigPlotController(appStatus, deviceDataProducer, defaultPlotDuration, mainWindow);
    chessboardController = new ChessboardController(appStatus, stampPlotConsumer, defaultPlotDuration, mainWindow);
//    COMPENSATION CONTROLLER MUST BE INITIALIZED BEFORE CONTROLLER CHANNEL
    compensationController = new CompensationController(msgDisp, mainWindow);
    multipleChannelController = new MultipleChannelController(appStatus, mainWindow);
    singleChannelController = new SingleChannelController(appStatus, mainWindow);
    boardController = new BoardController(msgDisp, mainWindow);
    deviceController = new DeviceController(appStatus, mainWindow);
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
    abfDataWriterConsumer = new AbfDataWriterConsumer(appStatus, deviceDataProducer);
    consumers.append(abfDataWriterConsumer);
    dataWriterConsumers.append(abfDataWriterConsumer);
    consumers.append(measurementOverviewController->getLiveStatisticsConsumer());

    mainWindow->addViewActions();

    /***********\
     * Connect *
    \***********/

    connect(chessboardController, &ChessboardController::sigAllChannelsClicked,     singleChannelController, &SingleChannelController::onAllChannelsClicked);
    connect(chessboardController, &ChessboardController::sigOneBoardClicked,        singleChannelController, &SingleChannelController::onOneBoardClicked);
    connect(chessboardController, &ChessboardController::sigOneRowClicked,          singleChannelController, &SingleChannelController::onOneRowClicked);
    connect(chessboardController, &ChessboardController::sigSingleChannelClicked,   singleChannelController, &SingleChannelController::onSingleChannelClicked);

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

    connect(multipleChannelController, &MultipleChannelController::sigStartRecording,                   this,                           &MainController::onStartRecording);
    connect(multipleChannelController, &MultipleChannelController::sigStopRecording,                    this,                           &MainController::onStopRecording);
    connect(multipleChannelController, &MultipleChannelController::sigAddRemoveFromBigPlot,             bigPlotController,              &BigPlotController::onExpandTrace);
    connect(multipleChannelController, &MultipleChannelController::sigAddRemoveFromBigPlot,             chessboardController,           &ChessboardController::onTracesExpandedOnOff);
    connect(multipleChannelController, &MultipleChannelController::sigChannelsTurnedOnOff,              chessboardController,           &ChessboardController::onChannelsTurnedOnOff);
    connect(multipleChannelController, &MultipleChannelController::sigCalibrationResistorsTurnedOnOff,  chessboardController,           &ChessboardController::onCalibrationResistorsTurnedOnOff);
    connect(multipleChannelController, &MultipleChannelController::sigStimuliTurnedOnOff,               chessboardController,           &ChessboardController::onStimuliTurnedOnOff);
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
        connect(voltageProtocolManager, &ProtocolManager::protocolStarted,          mainWindow->getProtocolDockWidget()->getVoltageProtocolList(), &ProtocolList::protocolStarted);
        connect(voltageProtocolManager, &ProtocolManager::protocolRequestOutcome,   mainWindow->getProtocolDockWidget()->getVoltageProtocolList(), &ProtocolList::onProtocolRequestOutcome);
        connect(voltageProtocolManager, &ProtocolManager::currentApplied,           mainWindow->getProtocolDockWidget()->getVoltageProtocolList(), &ProtocolList::currentApplied);

        connect(currentProtocolManager, &ProtocolManager::protocolStarted,          mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::protocolStarted);
        connect(currentProtocolManager, &ProtocolManager::protocolRequestOutcome,   mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::onProtocolRequestOutcome);
        connect(currentProtocolManager, &ProtocolManager::currentApplied,           mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::currentApplied);

        connect(mainWindow->getProtocolDockWidget(), &ProtocolDockWidget::startProtocol,    this, [=] () {
            mainWindow->getProtocolDockWidget()->getVoltageProtocolList()->onStartProtocol();
            deviceController->handleProtocolStatusChanged(true);
        });
        connect(mainWindow->getProtocolDockWidget(), &ProtocolDockWidget::restartProtocol,    this, [=] () {
            voltageProtocolManager->onRestartProtocolRequest();
        });
        connect(mainWindow->getProtocolDockWidget(), &ProtocolDockWidget::stopProtocol,     this, [=] () {
            mainWindow->getProtocolDockWidget()->getVoltageProtocolList()->onStopProtocol();
            deviceController->handleProtocolStatusChanged(false);
        });
        connect(mainWindow->getProtocolDockWidget(), &ProtocolDockWidget::startProtocol,    this, [=] () {
            mainWindow->getProtocolDockWidget()->getCurrentProtocolList()->onStartProtocol();
            deviceController->handleProtocolStatusChanged(true);
        });
        connect(mainWindow->getProtocolDockWidget(), &ProtocolDockWidget::stopProtocol,    this, [=] () {
            mainWindow->getProtocolDockWidget()->getCurrentProtocolList()->onStopProtocol();
            deviceController->handleProtocolStatusChanged(false);
        });
        connect(mainWindow->getProtocolDockWidget()->getVoltageProtocolList(), &ProtocolList::startProtocolRequest, voltageProtocolManager, &ProtocolManager::onStartProtocolRequest);
        connect(mainWindow->getProtocolDockWidget()->getVoltageProtocolList(), &ProtocolList::increaseProtocolId,   currentProtocolManager, &ProtocolManager::onIncreaseProtocolId);
        connect(mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::startProtocolRequest, currentProtocolManager, &ProtocolManager::onStartProtocolRequest);
        connect(mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::increaseProtocolId,   voltageProtocolManager, &ProtocolManager::onIncreaseProtocolId);
    }

    connect(mainWindow->getRecordSettingsDialog(), &RecordSettingsDialog::sigSettingsSet,   abfDataWriterConsumer, &DataWriterConsumer::onRecordingSettingsSet);
    connect(mainWindow->getMultipleChannelControlsDockWidget(), &MultipleChannelControlDockWidget::sigFileNameChanged, abfDataWriterConsumer, &DataWriterConsumer::onFilenameSet);
    connect(mainWindow->getMultipleChannelControlsDockWidget(), &MultipleChannelControlDockWidget::sigRecordPathChanged, abfDataWriterConsumer, &DataWriterConsumer::onFilePathSet);
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

    connect(abfDataWriterConsumer, &DataWriterConsumer::sigRecording, [=] (bool flag) {
        multipleChannelController->onRecordingExecution(flag);
        deviceController->handleRecording(flag);
    });

    chessboardController->onDurationUpdated(defaultPlotDuration);
    RangedMeasurement plotRange = {0, defaultPlotDuration.value, 1, defaultPlotDuration.prefix, defaultPlotDuration.unit};
    bigPlotController->onRangeUpdated(plotRange);

    /*! Forced initialization at start */
    mainWindow->getDeviceControlsDockWidget()->forceEmit();
    mainWindow->getRecordSettingsDialog()->forceSettingsEmit();

    /*! \todo FCON questo potrebbe essere parametrizzato */

    std::vector<uint16_t> channelIndexes(currentChannelsNumber);
    for(int i = 0; i < currentChannelsNumber; i++){
        channelIndexes[i] = i;
    }

    plotPreferencesController->initializePlotColors();

    /*! Start threads */
    this->startProducerConsumers();

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
    mainWindow->getDeviceControlsDockWidget()->updateParameters();

    RangedMeasurement_t range;
    msgDisp->getVCCurrentRange(range);

    for (auto consumer : consumers) {
        consumer->onCurrentRangeChanged(range);
    }
    for (auto controller : centralWidgetControllers) {
        controller->onCurrentRangeChanged(range);
    }
    chessboardController->onRangeUpdated(range);
    bigPlotController->onRangeUpdated(range);
    mainWindow->getSingleChannelControlsDockWidget()->onVcCurrentRangeSelected(idx); /*! \todo FCON vedere se questo genere di getXXXDw possono esseresostittuite con chiamate ai controller */
}

void MainController::onVcVoltageRangeSelected(int idx) {
    /*! update GUI */
    mainWindow->getDeviceControlsDockWidget()->updateParameters();

    RangedMeasurement_t range;
    msgDisp->getVCVoltageRange(range);
    // first of all we have to initialize the previousVcVoltageRange otherwise the equality check below will crash
    if (previousVcVoltageRange != nullptr  && *previousVcVoltageRange == range) {
        return;
    }
    previousVcVoltageRange = &range;

    for (auto consumer : consumers) {
        consumer->onVoltageRangeChanged(range);
    }
    for (auto controller : centralWidgetControllers) {
        controller->onVoltageRangeChanged(range);
    }
    //this should be useless?
    //chessboardController->onRangeUpdated(range, QwtPlot::yRight);
    bigPlotController->onRangeUpdated(range);
    mainWindow->getSingleChannelControlsDockWidget()->onVcVoltageRangeSelected(idx); /*! \todo FCON vedere se questo genere di getXXXDw possono esseresostittuite con chiamate ai controller */
}

void MainController::onCcCurrentRangeSelected(int idx) {
    /*! update GUI */
    mainWindow->getDeviceControlsDockWidget()->updateParameters();

    RangedMeasurement_t range;
    msgDisp->getCCCurrentRange(range);

    for (auto consumer : consumers) {
        consumer->onCurrentRangeChanged(range);
    }
    for (auto controller : centralWidgetControllers) {
        controller->onCurrentRangeChanged(range);
    }
    chessboardController->onRangeUpdated(range);
    bigPlotController->onRangeUpdated(range);
    mainWindow->getSingleChannelControlsDockWidget()->onCcCurrentRangeSelected(idx);
}

void MainController::onCcVoltageRangeSelected(int idx) {
    /*! update GUI */
    mainWindow->getDeviceControlsDockWidget()->updateParameters();

    RangedMeasurement_t range;
    msgDisp->getCCVoltageRange(range);

    for (auto consumer : consumers) {
        consumer->onVoltageRangeChanged(range);
    }
    for (auto controller : centralWidgetControllers) {
        controller->onVoltageRangeChanged(range);
    }
    //this should be useless?
    //chessboardController->onRangeUpdated(range, QwtPlot::yRight);
    bigPlotController->onRangeUpdated(range);
    mainWindow->getSingleChannelControlsDockWidget()->onCcVoltageRangeSelected(idx); /*! \todo FCON vedere se questo genere di getXXXDw possono esseresostittuite con chiamate ai controller */
}

void MainController::onVcVoltageFilterSelected(int) {
    /*! update GUI */
    mainWindow->getDeviceControlsDockWidget()->updateParameters();
}

void MainController::onCcCurrentFilterSelected(int) {
    /*! update GUI */
    mainWindow->getDeviceControlsDockWidget()->updateParameters();
}

void MainController::onSamplingRateSelected(int) {
    /*! update GUI */
    mainWindow->getDeviceControlsDockWidget()->updateParameters();

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
    /*! update GUI */
    mainWindow->getDeviceControlsDockWidget()->updateParameters();

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
    mainWindow->getDeviceControlsDockWidget()->updateParameters();

    mainWindow->getProtocolDockWidget()->onSetClampingModality(mode);
    /*! \todo FCON qualcuno da notificare che la clamping modality è cambiata? */
}

//TODO this could be moved at the controller level and b managed by single controllers
void MainController::onStartRecording() {
    std::vector <uint16_t> selectedChannels;
    msgDisp->getSelectedChannelsIndexes(selectedChannels);
    std::vector <bool> values(selectedChannels.size(), true);
    for (auto consumer : dataWriterConsumers) {
        consumer->onRecordSelectedChannels(selectedChannels, values);
    }
}

void MainController::onStopRecording() {
    for (auto consumer : dataWriterConsumers) {
        consumer->onStopConsuming();
    }
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

    if (abfDataWriterConsumer!= nullptr) {
        abfDataWriterConsumer->onStopConsuming();
        delete abfDataWriterConsumer;
        abfDataWriterConsumer = nullptr;
    }

    if (deviceDataProducer!= nullptr) {
        deviceDataProducer->onStopProducing();
        delete deviceDataProducer;
        deviceDataProducer = nullptr;
    }
    consumers.clear();
    centralWidgetControllers.clear();
    dataWriterConsumers.clear();
}
