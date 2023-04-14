#include "controllermain.h"

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

void ControllerMain::onDevicesListChanged(vector <string> devicesList) {
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

        emit connectDevice(connectionSuccessful, ret);
        mDev->setConnected(connectionSuccessful);

        if (connectionSuccessful) {
            emit stopDetecting();
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

    /***********\
     * Connect *
    \***********/

    connect(controllerChannel, &ControllerChannel::sigSelectedChannelsUpdated,   mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::onUpdate);
    connect(controllerChannel, &ControllerChannel::sigSelectedChannelsUpdated,   mainWindow->getChessaboard(), &Chessboard::onSelectedPlotsUdpated);

    /*! No signals from controllerBoard */

    connect(controllerDevice, &ControllerDevice::sigVcCurrentRangeSelected,     this, &ControllerMain::onVcCurrentRangeSelected);
    connect(controllerDevice, &ControllerDevice::sigVcVoltageRangeSelected,     this, &ControllerMain::onVcVoltageRangeSelected);
    connect(controllerDevice, &ControllerDevice::sigSamplingRateSelected,       this, &ControllerMain::onSamplingRateSelected);

    connect(mainWindow->getChessaboard(), &Chessboard::allChannelsClicked,      controllerChannel, &ControllerChannel::onAllChannelsClicked);
    connect(mainWindow->getChessaboard(), &Chessboard::oneRowClicked,           controllerChannel, &ControllerChannel::onOneRowClicked);
    connect(mainWindow->getChessaboard(), &Chessboard::oneBoardClicked,         controllerChannel, &ControllerChannel::onOneBoardClicked);
    connect(mainWindow->getChessaboard(), &Chessboard::singleChannelClicked,    controllerChannel, &ControllerChannel::onSingleChannelClicked);

    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigAppliedTurnChannelOnOff,      controllerChannel, &ControllerChannel::onApplyTurnChannelOnOff);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigAppliedTurnStimulsOnOff,      controllerChannel, &ControllerChannel::onApplyTurnStimulusOnOff);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigAppliedTurnDocOnOff,          controllerChannel, &ControllerChannel::onApplyTurnDocOnOff);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigAppliedVoltageHoldValues,     controllerChannel, &ControllerChannel::onApplyVoltageHoldValues);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigStartRecording,               this, &ControllerMain::onStartRecording);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigStopRecording,                this, &ControllerMain::onStopRecording);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigStartRecording,               mainWindow->getDeviceControlsDockWidget(), &DeviceControlDockWidget::onStartRecording);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigStopRecording,                mainWindow->getDeviceControlsDockWidget(), &DeviceControlDockWidget::onStopRecording);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigAppliedPlotToBigPlot,         bigPlotConsumer, &PlotConsumer::onSelectChannels);

    connect(mainWindow->getBoardControlsDockWidget(), &BoardControlDockWidget::sigGateSourceVoltagesApplied,    controllerBoard, &ControllerBoard::onGateSourceVoltagesApplied);

    connect(mainWindow->getDeviceControlsDockWidget(), &DeviceControlDockWidget::sigVcCurrentRangeSelected,     controllerDevice, &ControllerDevice::onVcCurrentRangeSelected);
    connect(mainWindow->getDeviceControlsDockWidget(), &DeviceControlDockWidget::sigVcVoltageRangeSelected,     controllerDevice, &ControllerDevice::onVcVoltageRangeSelected);
    connect(mainWindow->getDeviceControlsDockWidget(), &DeviceControlDockWidget::sigSamplingRateSelected,       controllerDevice, &ControllerDevice::onSamplingRateSelected);

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

    connect(stampPlotConsumer, &GapFreePlotConsumer::setPlotData,       mainWindow->getChessaboard(), &Chessboard::onSetGapFreePlotData);
    connect(stampPlotConsumer, &GapFreePlotConsumer::plotDataUpdated,   mainWindow->getChessaboard(), &Chessboard::onReplot);

    connect(bigPlotConsumer, &GapFreePlotConsumer::setPlotData,         mainWindow->getBigPlotWidget(), &BigPlotDockWidget::onSetGapFreePlotData);
    connect(bigPlotConsumer, &GapFreePlotConsumer::plotDataUpdated,     mainWindow->getBigPlotWidget(), &BigPlotDockWidget::onReplot);

    connect(abfDataWriterConsumer, &AbfDataWriterConsumer::sigFileSizeComputed,     mainWindow->getRecordSettingsDialog(), &RecordSettingsDialog::onFileSizeComputed);
    connect(abfDataWriterConsumer, &DataWriterConsumer::sigRecording,               mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::onSigRecording);

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

    vector<uint16_t> channelIndexes(currentChannelsNumber);
    for(int i = 0; i < currentChannelsNumber; i++){
        channelIndexes[i] = i;
    }

    stampPlotConsumer->setMaxSamplesPerPlot(256);
    vector<bool> onValues(currentChannelsNumber, true);
    stampPlotConsumer->onSelectChannels(channelIndexes, onValues);

    bigPlotConsumer->setMaxSamplesPerPlot(4096);
    vector<bool> offValues(currentChannelsNumber, false);
    bigPlotConsumer->onSelectChannels(channelIndexes, offValues);

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

    this->stopAndDestroyProducerConsumers();

    mDev->flushBoardList();

    emit startDetecting();
}

/*! Message forward from ControllerMain to other consumers */

void ControllerMain::onVcCurrentRangeSelected(int idx) {
    vector <RangedMeasurement_t> ranges;
    mDev->getVcCurrentRangesFeatures(ranges);
    mDev->setVcCurrentRange(ranges[idx]);

    for (auto consumer : consumers) {
        consumer->onCurrentRangeChanged(mDev->getVcCurrentRange());
    }

    /*! \todo FCON Questi metodi protebbero cambiare di significato se si lavora in CC: l'asse da cambiare sarebbe quello destro probabilmente */
    mainWindow->getChessaboard()->onRangeUpdated(mDev->getVcCurrentRange(), QwtPlot::yLeft);
    mainWindow->getBigPlotWidget()->onRangeUpdated(mDev->getVcCurrentRange(), QwtPlot::yLeft);
}

void ControllerMain::onVcVoltageRangeSelected(int idx) {
    vector <RangedMeasurement_t> ranges;
    mDev->getVcVoltageRangesFeatures(ranges);
    mDev->setVcVoltageRange(ranges[idx]);

    for (auto consumer : consumers) {
        consumer->onVoltageRangeChanged(mDev->getVcVoltageRange());
    }
    mainWindow->getBigPlotWidget()->onRangeUpdated(mDev->getVcVoltageRange(), QwtPlot::yRight);

    /*! \todo FCON anche qui si potrebbe dover cambiare gli assi dei plot in CC o con più range di stimolo in Vc */
}

void ControllerMain::onSamplingRateSelected(int idx) {
    vector <Measurement_t> samplingRates;
    mDev->getSamplingRatesFeatures(samplingRates);
    mDev->setSamplingRate(samplingRates[idx]);

    for (auto consumer : consumers) {
        consumer->onSamplingRateChanged(mDev->getSamplingRate());
    }
}

void ControllerMain::onStartRecording(vector<uint16_t> channelIndexes, vector<bool> onValues) {
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

    if (deviceDataProducer!= nullptr) {
        deviceDataProducer->onStopProducing();
        delete deviceDataProducer;
        deviceDataProducer = nullptr;
    }
}
