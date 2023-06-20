#include "controllermain.h"
#include "controllerstatearray.h"
#include "mainwindow.h"

ControllerMain::ControllerMain() {
    mDev = new ModelDevice();
    /*! Set up device detector */
    deviceDetector = new DeviceDetector;
    deviceDetector->moveToThread(&deviceDetectorThread);

    connect(this, &ControllerMain::startDetecting, deviceDetector, &DeviceDetector::onStartDetecting);
    connect(this, &ControllerMain::stopDetecting, deviceDetector, &DeviceDetector::onStopDetecting);

    deviceDetectorThread.start();
    setMainWindow(new MainWindow());
}

void ControllerMain::setMainWindow(MainWindow * mainWindow) {
    this->mainWindow = mainWindow;

    mainWindow->setModelDevice(mDev);
    connect(deviceDetector, &DeviceDetector::devicesListChanged, this, &ControllerMain::onDevicesListChanged);
    connect(mainWindow->getConnectButton(), &QPushButton::clicked, this, &ControllerMain::onConnect);

    connect(mainWindow, &MainWindow::widgetsCreated, this, &ControllerMain::onMainWindowCreated);
    mainWindow->show();
    emit startDetecting();
}

ControllerMain::~ControllerMain() {
    deviceDetectorThread.quit();
    deviceDetectorThread.wait();

    this->onMainWindowDestroyed();

    if (deviceDetector != nullptr) {
        delete deviceDetector;
        deviceDetector = nullptr;
    }

    if (mDev->isConnected()) {
        mDev->getMessageDispatcher()->disconnectDevice();
    }
}

void ControllerMain::onDevicesListChanged(std::vector <std::string> devicesList) {
    this->mainWindow->setDevicesList(devicesList);
    if (devicesList.size() > 0) {
        if (mDev->isConnected()) {
            QString connectedDeviceName = mDev->getSerialNumber();

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
                mainWindow->connectDevice(false, Success);
            }
        }
    }
}

void ControllerMain::onConnect(bool flag) {
    emit stopDetecting();
    QString serial = mainWindow->getSelectedSerialNumber();
    mDev->setSerialNumber(serial);

    if (flag) {
        MessageDispatcher * messageDispatcher;
        ErrorCodes_t ret = MessageDispatcher::connectDevice(serial.toStdString(), messageDispatcher);
        bool connectionSuccessful = ret == Success;

        if (connectionSuccessful) {
            mDev->setMessageDispatcher(messageDispatcher);
            mDev->getChannelsNumberFeatures(voltageChannelsNumber, currentChannelsNumber);
            mDev->getBoardsNumberFeatures(boardsNumber);
            mDev->fillChannelList(boardsNumber, currentChannelsNumber/boardsNumber);
        }

        mainWindow->connectDevice(true, ret);
        mDev->setConnected(connectionSuccessful);

        if (!connectionSuccessful) {
            emit startDetecting();
        }

    } else {
        this->stopAndDestroyProducerConsumers();

        mainWindow->connectDevice(false, Success);
        mDev->setConnected(false);

        if (mDev->getMessageDispatcher() != nullptr) {
            mDev->getMessageDispatcher()->disconnect();
            delete mDev->getMessageDispatcher();
            mDev->setMessageDispatcher(nullptr);
        }

        emit startDetecting();
    }
}

void ControllerMain::onMainWindowCreated() {
    consumers.clear();
    dataWriterConsumers.clear();

    /***************\
     * Controllers *
    \***************/
    controllerChannel = new ControllerChannel(mDev, mainWindow);
    controllerBoard = new ControllerBoard(mDev, mainWindow);
    controllerDevice = new ControllerDevice(mDev, mainWindow);
    voltageProtocolManager = new ProtocolManager(mDev);
    currentProtocolManager = new ProtocolManager(mDev);
//    voltageProtocolManager = new ProtocolManager(mDev, e384CommLib::VOLTAGE_CLAMP);
//    currentProtocolManager = new ProtocolManager(mDev, e384CommLib::CURRENT_CLAMP);
    controllerCompensation = new ControllerCompensation(mDev, mainWindow);

//    mainWindow->setProtocolDw(voltageProtocolManager->getProtocolDockWidget());
//    mainWindow->setProtocolDw(currentProtocolManager->getProtocolDockWidget());

    if(mDev->getMessageDispatcher()->isStateArrayAvailable()){
        controllerStateArray = new ControllerStateArray(mDev);
        controllerStateArray->setStateArrayWidget(mainWindow->getStateArrayDockWidget());
    }
    /************\
     * Producer *
    \************/

    deviceDataProducer = new DeviceDataProducer(mDev);

    /*************\
     * Consumers *
    \*************/

    stampPlotConsumer = new GapFreePlotConsumer(mDev, deviceDataProducer);
    consumers.append(stampPlotConsumer);

    bigPlotConsumer = new GapFreePlotConsumer(mDev, deviceDataProducer);
    consumers.append(bigPlotConsumer);

    abfDataWriterConsumer = new AbfDataWriterConsumer(mDev, deviceDataProducer);
    consumers.append(abfDataWriterConsumer);
    dataWriterConsumers.append(abfDataWriterConsumer);

    liveNoiseConsumer = new LiveNoiseConsumer(mDev, deviceDataProducer);
    consumers.append(liveNoiseConsumer);
    
    calibratorConsumer = new CalibrationConsumer(mDev, deviceDataProducer);
    consumers.append(calibratorConsumer);

    /***********\
     * Connect *
    \***********/
    connect(controllerDevice, &ControllerDevice::sigVcCurrentRangeSelected,     this, &ControllerMain::onVcCurrentRangeSelected);
    connect(controllerDevice, &ControllerDevice::sigVcVoltageRangeSelected,     this, &ControllerMain::onVcVoltageRangeSelected);
    connect(controllerDevice, &ControllerDevice::sigCcCurrentRangeSelected,     this, &ControllerMain::onCcCurrentRangeSelected);
    connect(controllerDevice, &ControllerDevice::sigCcVoltageRangeSelected,     this, &ControllerMain::onCcVoltageRangeSelected);
    connect(controllerDevice, &ControllerDevice::sigVcVoltageFilterSelected,    this, &ControllerMain::onVcVoltageFilterSelected);
    connect(controllerDevice, &ControllerDevice::sigCcCurrentFilterSelected,    this, &ControllerMain::onCcCurrentFilterSelected);
    connect(controllerDevice, &ControllerDevice::sigSamplingRateSelected,       this, &ControllerMain::onSamplingRateSelected);
    connect(controllerDevice, &ControllerDevice::sigClampingModalitySelected,   this, &ControllerMain::onClampingModalitySelected);

    connect(voltageProtocolManager, &ProtocolManager::protocolStarted,          mainWindow->getProtocolDockWidget()->getVoltageProtocolList(), &ProtocolList::protocolStarted);
    connect(voltageProtocolManager, &ProtocolManager::protocolRequestOutcome,   mainWindow->getProtocolDockWidget()->getVoltageProtocolList(), &ProtocolList::onProtocolRequestOutcome);
    connect(voltageProtocolManager, &ProtocolManager::currentApplied,           mainWindow->getProtocolDockWidget()->getVoltageProtocolList(), &ProtocolList::currentApplied);
#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    connect(voltageProtocolManager, &ProtocolManager::protocolSaveRequest,      mainWindow->getProtocolDockWidget()->getVoltageProtocolList(), &ProtocolList::protocolSaveRequest);
#endif
    connect(currentProtocolManager, &ProtocolManager::protocolStarted,          mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::protocolStarted);
    connect(currentProtocolManager, &ProtocolManager::protocolRequestOutcome,   mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::onProtocolRequestOutcome);
    connect(currentProtocolManager, &ProtocolManager::currentApplied,           mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::currentApplied);
#ifdef GLB_RECORD_CONTROLS_IN_PROTOCOL_WIDGET
    connect(currentProtocolManager, &ProtocolManager::protocolSaveRequest,      mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::protocolSaveRequest);
#endif
    connect(mainWindow->getChessaboard(), &Chessboard::sigExportLiveNoiseEstimates, liveNoiseConsumer, &LiveNoiseConsumer::onExportLiveNoiseEstimates);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigStartRecording,               this, &ControllerMain::onStartRecording);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigStopRecording,                this, &ControllerMain::onStopRecording);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigAppliedPlotToBigPlot,         bigPlotConsumer, &PlotConsumer::onSelectChannels);

    connect(mainWindow->getProtocolDockWidget(), &ProtocolDockWidget::startProtocol,    this, [=] () {
        mainWindow->getProtocolDockWidget()->getVoltageProtocolList()->onStartProtocol();
    });
    connect(mainWindow->getProtocolDockWidget(), &ProtocolDockWidget::stopProtocol,     mainWindow->getProtocolDockWidget()->getVoltageProtocolList(), &ProtocolList::onStopProtocol);
    connect(mainWindow->getProtocolDockWidget(), &ProtocolDockWidget::startProtocol,    this, [=] () {
        mainWindow->getProtocolDockWidget()->getCurrentProtocolList()->onStartProtocol();
    });
    connect(mainWindow->getProtocolDockWidget(), &ProtocolDockWidget::stopProtocol,     mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::onStopProtocol);

    connect(mainWindow->getProtocolDockWidget()->getVoltageProtocolList(), &ProtocolList::startProtocolRequest, voltageProtocolManager, &ProtocolManager::onStartProtocolRequest);
    connect(mainWindow->getProtocolDockWidget()->getVoltageProtocolList(), &ProtocolList::increaseProtocolId,   currentProtocolManager, &ProtocolManager::onIncreaseProtocolId);
    connect(mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::startProtocolRequest, currentProtocolManager, &ProtocolManager::onStartProtocolRequest);
    connect(mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::increaseProtocolId,   voltageProtocolManager, &ProtocolManager::onIncreaseProtocolId);

    connect(mainWindow->getRecordSettingsDialog(), &RecordSettingsDialog::sigSettingsSet,   abfDataWriterConsumer, &DataWriterConsumer::onRecordingSettingsSet);

    connect(mainWindow, &MainWindow::setDebugBit, this, [=] (int word, int bit, bool flag) {
        mDev->getMessageDispatcher()->setDebugBit(word, bit, flag);
    });
    connect(mainWindow, &MainWindow::setDebugWord, this, [=] (int word, int value) {
        mDev->getMessageDispatcher()->setDebugWord(word, value);
    });
    connect(mainWindow, &MainWindow::debugInitialization, this, [=] () {
        mDev->getMessageDispatcher()->initializeDevice();
    });

    connect(deviceDataProducer, &DeviceDataProducer::bitRateComputed, this, [=] (double value) {
        if (value > 1.0e6) {
            mainWindow->SRLbl->setText(QString("%1 Msps").arg(value/1.0e6));

        } else {
            mainWindow->SRLbl->setText(QString("%1 ksps").arg(value/1.0e3));
        }
    });

    connect(stampPlotConsumer, &GapFreePlotConsumer::setPlotData,       mainWindow->getChessaboard(), &Chessboard::onSetGapFreePlotData);
    connect(stampPlotConsumer, &GapFreePlotConsumer::plotDataUpdated,   mainWindow->getChessaboard(), &Chessboard::onReplot);

    connect(bigPlotConsumer, &GapFreePlotConsumer::setPlotData,         mainWindow->getBigPlotWidget(), &BigPlotDockWidget::onSetGapFreePlotData);
    connect(bigPlotConsumer, &GapFreePlotConsumer::plotDataUpdated,     mainWindow->getBigPlotWidget(), &BigPlotDockWidget::onReplot);

    connect(abfDataWriterConsumer, &AbfDataWriterConsumer::sigFileSizeComputed,     mainWindow->getRecordSettingsDialog(), &RecordSettingsDialog::onFileSizeComputed);
    connect(abfDataWriterConsumer, &DataWriterConsumer::sigRecording,               mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::onSigRecording);

    connect(liveNoiseConsumer, &LiveNoiseConsumer::sigResult,   mainWindow->getChessaboard(), &Chessboard::onNoiseValueUpdated);

    /*! \todo at the moment only for debug mode*/
    connect(mainWindow, &MainWindow::sigModelCellChanged,   calibratorConsumer, &CalibrationConsumer::onModelCellChanged);
    connect(mainWindow, &MainWindow::sigPerformCalibration, calibratorConsumer, &CalibrationConsumer::onPerformCalibration);

    connect(calibratorConsumer, QOverload <QString> ::of(&CalibrationConsumer::sigCalibLoadingMsg),         mainWindow, QOverload <QString> ::of(&MainWindow::onCalibLoadingMsg));
    connect(calibratorConsumer, QOverload <ErrorCodes_t> ::of(&CalibrationConsumer::sigCalibLoadingMsg),    mainWindow, QOverload <ErrorCodes_t> ::of(&MainWindow::onCalibLoadingMsg));
    connect(calibratorConsumer, &CalibrationConsumer::sigManualCalibDoneMsg,                                mainWindow, &MainWindow::onManualCalibDoneMsg);
    connect(calibratorConsumer, &CalibrationConsumer::sigNeedToChangeModelCellMsg,                          mainWindow, &MainWindow::onNeedToChangeModelCellMsg);

    /*! Plots durations */
    /*! \todo FCON Capire come gestire le durate dei plot */
    Measurement_t defaultPlotDuration = {2.0, UnitPfxNone, "s"};

    stampPlotConsumer->onDurationChanged(defaultPlotDuration);
    bigPlotConsumer->onDurationChanged(defaultPlotDuration);

    mainWindow->getChessaboard()->onDurationUpdated(defaultPlotDuration);
    mainWindow->getBigPlotWidget()->onDurationUpdated(defaultPlotDuration);

    /*! Forced initialization at start */
    mainWindow->getDeviceControlsDockWidget()->forceEmit();
    mainWindow->getRecordSettingsDialog()->forceSettingsEmit();
    stampPlotConsumer->forceAxisUpdate();
    bigPlotConsumer->forceAxisUpdate();

    /*! \todo FCON questo potrebbe essere parametrizzato */

    std::vector<uint16_t> channelIndexes(currentChannelsNumber);
    for(int i = 0; i < currentChannelsNumber; i++){
        channelIndexes[i] = i;
    }

    stampPlotConsumer->setMaxSamplesPerPlot(256);
    std::vector<bool> onValues(currentChannelsNumber, true);
    stampPlotConsumer->onSelectChannels(channelIndexes, onValues);

    bigPlotConsumer->setMaxSamplesPerPlot(4096);
    std::vector<bool> offValues(currentChannelsNumber, false);
    bigPlotConsumer->onSelectChannels(channelIndexes, offValues);

//    calibratorConsumer->loadInitialCalibParams(calibratorConsumer->getCalibrationPath(), "boardMapping.csv");
    calibratorConsumer->loadInitialCalibParams(calibratorConsumer->getCalibrationDir(), calibratorConsumer->getCalibrationMappingFilePath());

    /*! Start threads */
    this->startProducerConsumers();
}

void ControllerMain::onMainWindowDestroyed() {
    if (controllerChannel != nullptr) {
        delete controllerChannel;
        controllerChannel = nullptr;
    }

    if (controllerBoard != nullptr) {
        delete controllerBoard;
        controllerBoard = nullptr;
    }

    if (controllerDevice != nullptr) {
        delete controllerDevice;
        controllerDevice = nullptr;
    }

    if (voltageProtocolManager != nullptr) {
        delete voltageProtocolManager;
        voltageProtocolManager = nullptr;
    }

    if (currentProtocolManager != nullptr) {
        delete currentProtocolManager;
        currentProtocolManager = nullptr;
    }

    this->stopAndDestroyProducerConsumers();

    mDev->flushBoardList();

    emit startDetecting();
}

/*! Message forward from ControllerMain to other consumers */

void ControllerMain::onVcCurrentRangeSelected(int idx) {
    /*! update GUI */
    mainWindow->getDeviceControlsDockWidget()->updateParameters();

    /*! Invio dati a FPGA con massageDispatcher*/
    calibratorConsumer->updateCalibParams();

    for (auto consumer : consumers) {
        consumer->onCurrentRangeChanged(mDev->getVcCurrentRange());
    }

    mainWindow->getChessaboard()->onRangeUpdated(mDev->getVcCurrentRange(), QwtPlot::yLeft);
    mainWindow->getBigPlotWidget()->onRangeUpdated(mDev->getVcCurrentRange(), QwtPlot::yLeft);
}

void ControllerMain::onVcVoltageRangeSelected(int idx) {
    /*! update GUI */
    mainWindow->getDeviceControlsDockWidget()->updateParameters();

    /*! Invio dati a FPGA con massageDispatcher*/
    calibratorConsumer->updateCalibParams();

    for (auto consumer : consumers) {
        consumer->onVoltageRangeChanged(mDev->getVcVoltageRange());
    }
    mainWindow->getChessaboard()->onRangeUpdated(mDev->getVcVoltageRange(), QwtPlot::yRight);
    mainWindow->getBigPlotWidget()->onRangeUpdated(mDev->getVcVoltageRange(), QwtPlot::yRight);
    mainWindow->getChannelControlsDockWidget()->onVcVoltageRangeSelected(idx);
}

void ControllerMain::onCcCurrentRangeSelected(int idx) {
    /*! update GUI */
    mainWindow->getDeviceControlsDockWidget()->updateParameters();

    /*! Invio dati a FPGA con massageDispatcher*/
    calibratorConsumer->updateCalibParams();

    for (auto consumer : consumers) {
        consumer->onCurrentRangeChanged(mDev->getVcCurrentRange());
    }

    mainWindow->getChessaboard()->onRangeUpdated(mDev->getCcCurrentRange(), QwtPlot::yLeft);
    mainWindow->getBigPlotWidget()->onRangeUpdated(mDev->getCcCurrentRange(), QwtPlot::yLeft);
    mainWindow->getChannelControlsDockWidget()->onCcCurrentRangeSelected(idx);
}

void ControllerMain::onCcVoltageRangeSelected(int idx) {
    std::vector <RangedMeasurement_t> ranges;
    mDev->getCcVoltageRangesFeatures(ranges);

    /*! update GUI */
    mainWindow->getDeviceControlsDockWidget()->updateParameters();

    /*! Invio dati a FPGA con massageDispatcher*/
    calibratorConsumer->updateCalibParams();

    for (auto consumer : consumers) {
        consumer->onVoltageRangeChanged(mDev->getCcVoltageRange());
    }
    mainWindow->getChessaboard()->onRangeUpdated(mDev->getCcVoltageRange(), QwtPlot::yRight);
    mainWindow->getBigPlotWidget()->onRangeUpdated(mDev->getCcVoltageRange(), QwtPlot::yRight);
}

void ControllerMain::onVcVoltageFilterSelected(int idx) {
    std::vector <Measurement_t> meas;
    mDev->getVoltageStimulusLpfsFeatures(meas);

    /*! update GUI */
    mainWindow->getDeviceControlsDockWidget()->updateParameters();
}

void ControllerMain::onCcCurrentFilterSelected(int idx) {
    std::vector <Measurement_t> meas;
    mDev->getCurrentStimulusLpfsFeatures(meas);

    /*! update GUI */
    mainWindow->getDeviceControlsDockWidget()->updateParameters();
}

void ControllerMain::onSamplingRateSelected(int idx) {

    /*! update GUI */
    mainWindow->getDeviceControlsDockWidget()->updateParameters();

    for (auto consumer : consumers) {
        consumer->onSamplingRateChanged(mDev->getSamplingRate());
    }
}

void ControllerMain::onClampingModalitySelected(int idx) {
    /*! update GUI */
    mainWindow->getDeviceControlsDockWidget()->updateParameters();

    /*! Invio dati a FPGA con massageDispatcher*/
    calibratorConsumer->updateCalibParams();

    /*! \todo FCON qualcuno da notificare che la clamping modality è cambiata? */
}

void ControllerMain::onStartRecording(std::vector<uint16_t> channelIndexes, std::vector<bool> onValues) {
    for (auto consumer : dataWriterConsumers) {
        consumer->onRecordSelectedChannels(channelIndexes, onValues);
    }
}

void ControllerMain::onStopRecording() {
    for (auto consumer : dataWriterConsumers) {
        consumer->onStopConsuming();
    }
}

void ControllerMain::startProducerConsumers() {
    deviceDataProducer->start();
    stampPlotConsumer->onStartConsuming();
    bigPlotConsumer->onStartConsuming();
    liveNoiseConsumer->onStartConsuming(); /*! \todo FCON valutare se farlo partire solo a richiesta */
}

void ControllerMain::stopAndDestroyProducerConsumers() {
    if (stampPlotConsumer!= nullptr) {
        stampPlotConsumer->onStopConsuming();
        delete stampPlotConsumer;
        stampPlotConsumer = nullptr;
    }

    if (bigPlotConsumer!= nullptr) {
        bigPlotConsumer->onStopConsuming();
        delete bigPlotConsumer;
        bigPlotConsumer = nullptr;
    }

    if (abfDataWriterConsumer!= nullptr) {
        abfDataWriterConsumer->onStopConsuming();
        delete abfDataWriterConsumer;
        abfDataWriterConsumer = nullptr;
    }

    if (liveNoiseConsumer!= nullptr) {
        liveNoiseConsumer->onStopConsuming();
        delete liveNoiseConsumer;
        liveNoiseConsumer = nullptr;
    }
    
    if (calibratorConsumer!= nullptr) {
        calibratorConsumer->onStopConsuming();
        delete calibratorConsumer;
        calibratorConsumer = nullptr;
    }

    if (deviceDataProducer!= nullptr) {
        deviceDataProducer->onStopProducing();
        delete deviceDataProducer;
        deviceDataProducer = nullptr;
    }
}
