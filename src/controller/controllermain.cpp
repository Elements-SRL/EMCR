#include "controllermain.h"
#include "controllerstatearray.h"

ControllerMain::ControllerMain() {
    mDev = new ModelDevice();

    /*! Set up device detector */
    deviceDetector = new DeviceDetector;
    deviceDetector->moveToThread(&deviceDetectorThread);

    connect(this, &ControllerMain::startDetecting, deviceDetector, &DeviceDetector::onStartDetecting);
    connect(this, &ControllerMain::stopDetecting, deviceDetector, &DeviceDetector::onStopDetecting);

    deviceDetectorThread.start();
}

void ControllerMain::setMainWindow(MainWindow * mainWindow) {
    this->mainWindow = mainWindow;

    mainWindow->setModelDevice(mDev);
    connect(deviceDetector, &DeviceDetector::devicesListChanged, this, &ControllerMain::onDevicesListChanged);
    connect(this, &ControllerMain::devicesListChanged, mainWindow, &MainWindow::onDevicesListChanged);
    connect(this, &ControllerMain::setConnectedDeviceIdx, mainWindow, &MainWindow::onSetConnectedDeviceIdx);
    connect(this, &ControllerMain::connectDevice, mainWindow, &MainWindow::onConnect);
    connect(mainWindow->getConnectButton(), &QPushButton::clicked, this, &ControllerMain::onConnect);

    connect(mainWindow, &MainWindow::widgetsCreated, this, &ControllerMain::onMainWindowCreated);

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
    emit devicesListChanged(devicesList);
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
                emit setConnectedDeviceIdx(connectedDeviceIdx);

            } else {
                emit connectDevice(false, Success);
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

        emit connectDevice(true, ret);
        mDev->setConnected(connectionSuccessful);

        if (!connectionSuccessful) {
            emit startDetecting();
        }

    } else {
        this->stopAndDestroyProducerConsumers();

        emit connectDevice(false, Success);
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

    controllerChannel = new ControllerChannel(mDev);
    controllerBoard = new ControllerBoard(mDev);
    controllerDevice = new ControllerDevice(mDev);

    voltageProtocolManager = new ProtocolManager(mDev);
    currentProtocolManager = new ProtocolManager(mDev);
    controllerStateArray = new ControllerStateArray(mDev);
    controllerStateArray->setStateArrayWidget(mainWindow->getStateArrayDockWidget());
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

    connect(controllerChannel, &ControllerChannel::sigSelectedChannelsUpdated,   mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::onUpdate);
    connect(controllerChannel, &ControllerChannel::sigSelectedChannelsUpdated,   mainWindow->getChessaboard(), &Chessboard::onSelectedPlotsUdpated);

    /*! No signals from controllerBoard */

    connect(controllerDevice, &ControllerDevice::sigVcCurrentRangeSelected,     this, &ControllerMain::onVcCurrentRangeSelected);
    connect(controllerDevice, &ControllerDevice::sigVcVoltageRangeSelected,     this, &ControllerMain::onVcVoltageRangeSelected);
    connect(controllerDevice, &ControllerDevice::sigCcCurrentRangeSelected,     this, &ControllerMain::onCcCurrentRangeSelected);
    connect(controllerDevice, &ControllerDevice::sigCcVoltageRangeSelected,     this, &ControllerMain::onCcVoltageRangeSelected);
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

    connect(mainWindow->getChessaboard(), &Chessboard::allChannelsClicked,      controllerChannel, &ControllerChannel::onAllChannelsClicked);
    connect(mainWindow->getChessaboard(), &Chessboard::oneRowClicked,           controllerChannel, &ControllerChannel::onOneRowClicked);
    connect(mainWindow->getChessaboard(), &Chessboard::oneBoardClicked,         controllerChannel, &ControllerChannel::onOneBoardClicked);
    connect(mainWindow->getChessaboard(), &Chessboard::singleChannelClicked,    controllerChannel, &ControllerChannel::onSingleChannelClicked);

    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigAppliedTurnChannelOnOff,      controllerChannel, &ControllerChannel::onApplyTurnChannelOnOff);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigAppliedTurnStimulsOnOff,      controllerChannel, &ControllerChannel::onApplyTurnStimulusOnOff);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigAppliedTurnDocOnOff,          controllerChannel, &ControllerChannel::onApplyTurnDocOnOff);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigAppliedHoldValues,            controllerChannel, &ControllerChannel::onApplyHoldValues);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigStartRecording,               this, &ControllerMain::onStartRecording);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigStopRecording,                this, &ControllerMain::onStopRecording);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigStartRecording,               mainWindow->getDeviceControlsDockWidget(), &DeviceControlDockWidget::onStartRecording);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigStopRecording,                mainWindow->getDeviceControlsDockWidget(), &DeviceControlDockWidget::onStopRecording);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigAppliedPlotToBigPlot,         bigPlotConsumer, &PlotConsumer::onSelectChannels);

    connect(mainWindow->getBoardControlsDockWidget(), &BoardControlDockWidget::sigGateSourceVoltagesApplied,    controllerBoard, &ControllerBoard::onGateSourceVoltagesApplied);

    connect(mainWindow->getDeviceControlsDockWidget(), &DeviceControlDockWidget::sigVcCurrentRangeSelected,     controllerDevice, &ControllerDevice::onVcCurrentRangeSelected);
    connect(mainWindow->getDeviceControlsDockWidget(), &DeviceControlDockWidget::sigVcVoltageRangeSelected,     controllerDevice, &ControllerDevice::onVcVoltageRangeSelected);
    connect(mainWindow->getDeviceControlsDockWidget(), &DeviceControlDockWidget::sigCcCurrentRangeSelected,     controllerDevice, &ControllerDevice::onCcCurrentRangeSelected);
    connect(mainWindow->getDeviceControlsDockWidget(), &DeviceControlDockWidget::sigCcVoltageRangeSelected,     controllerDevice, &ControllerDevice::onCcVoltageRangeSelected);
    connect(mainWindow->getDeviceControlsDockWidget(), &DeviceControlDockWidget::sigSamplingRateSelected,       controllerDevice, &ControllerDevice::onSamplingRateSelected);
    connect(mainWindow->getDeviceControlsDockWidget(), &DeviceControlDockWidget::sigClampingModalitySelected,   controllerDevice, &ControllerDevice::onClampingModalitySelected);


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
    connect(mainWindow->getProtocolDockWidget()->getVoltageProtocolList(), &ProtocolList::requestCurrentRange,  controllerDevice, &ControllerDevice::onVcCurrentRangeSelected);
    connect(mainWindow->getProtocolDockWidget()->getVoltageProtocolList(), &ProtocolList::requestVoltageRange,  controllerDevice, &ControllerDevice::onVcVoltageRangeSelected);
    connect(mainWindow->getProtocolDockWidget()->getVoltageProtocolList(), &ProtocolList::requestSamplingRate,  controllerDevice, &ControllerDevice::onSamplingRateSelected);

    connect(mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::startProtocolRequest, currentProtocolManager, &ProtocolManager::onStartProtocolRequest);
    connect(mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::increaseProtocolId,   voltageProtocolManager, &ProtocolManager::onIncreaseProtocolId);
    connect(mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::requestCurrentRange,  controllerDevice, &ControllerDevice::onCcCurrentRangeSelected);
    connect(mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::requestVoltageRange,  controllerDevice, &ControllerDevice::onCcVoltageRangeSelected);
    connect(mainWindow->getProtocolDockWidget()->getCurrentProtocolList(), &ProtocolList::requestSamplingRate,  controllerDevice, &ControllerDevice::onSamplingRateSelected);

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

    connect(mainWindow->getCompensationControlsDockWidget(), &CompensationControlDockWidget::sigCompensationsApplied,    controllerChannel, &ControllerChannel::onCompensationApplied);
    connect(controllerChannel, &ControllerChannel::sigCompValuesDispatched, mainWindow->getCompensationControlsDockWidget(), &CompensationControlDockWidget::onCompValuesDispatched);

    connect(stampPlotConsumer, &GapFreePlotConsumer::setPlotData,       mainWindow->getChessaboard(), &Chessboard::onSetGapFreePlotData);
    connect(stampPlotConsumer, &GapFreePlotConsumer::plotDataUpdated,   mainWindow->getChessaboard(), &Chessboard::onReplot);

    connect(bigPlotConsumer, &GapFreePlotConsumer::setPlotData,         mainWindow->getBigPlotWidget(), &BigPlotDockWidget::onSetGapFreePlotData);
    connect(bigPlotConsumer, &GapFreePlotConsumer::plotDataUpdated,     mainWindow->getBigPlotWidget(), &BigPlotDockWidget::onReplot);

    connect(abfDataWriterConsumer, &AbfDataWriterConsumer::sigFileSizeComputed,     mainWindow->getRecordSettingsDialog(), &RecordSettingsDialog::onFileSizeComputed);
    connect(abfDataWriterConsumer, &DataWriterConsumer::sigRecording,               mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::onSigRecording);

    connect(liveNoiseConsumer, &LiveNoiseConsumer::sigResult,   mainWindow->getChessaboard(), &Chessboard::onNoiseValueUpdated);

    /*! \todo at the moment only for debug mode*/
    connect(mainWindow, &MainWindow::sigPerformCalibration,              calibratorConsumer, &CalibrationConsumer::onPerformCalibration);
    connect(calibratorConsumer, &CalibrationConsumer::sigCalibLoadingMsg,   mainWindow, &MainWindow::onCalibLoadingMsg);
    connect(calibratorConsumer, &CalibrationConsumer::sigManualCalibDoneMsg,   mainWindow, &MainWindow::onManualCalibDoneMsg);
    connect(calibratorConsumer, &CalibrationConsumer::sigNeedToChangeModelCellMsg,   mainWindow, &MainWindow::onNeedToChangeModelCellMsg);
    connect(mainWindow, &MainWindow::sigModelCellChanged,   calibratorConsumer, &CalibrationConsumer::onModelCellChanged);
    connect(calibratorConsumer, &CalibrationConsumer::sigNeedToCheckFirstModelCellMsg,   mainWindow, &MainWindow::onNeedToCheckFirstModelCellMsg);
    connect(mainWindow, &MainWindow::sigFirstModelMounted,   calibratorConsumer, &CalibrationConsumer::onFirstModelMounted);




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

    calibratorConsumer->loadInitialCalibParams(calibratorConsumer->getCalibrationPath(), "boardMapping.csv");

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
