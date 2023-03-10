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

    if (deviceDetector != nullptr) {
        delete deviceDetector;
        deviceDetector = nullptr;
    }

    if (mDev->isConnected()) {
        //        this->destroyGuiControls();

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

        mDev->setMessageDispatcher(messageDispatcher);
        uint16_t voltageChannelsNumber;
        uint16_t currentChannelsNumber;
        uint16_t boardsNumber; // da estrarre da MessageDispatcher
        mDev->getMessageDispatcher()->getChannelNumberFeatures(voltageChannelsNumber, currentChannelsNumber);
        mDev->getMessageDispatcher()->getBoardsNumberFeatures(boardsNumber);
        mDev->fillChannelList(boardsNumber, currentChannelsNumber/boardsNumber);

        bool connectionSuccessful = ret == Success;
        emit connectDevice(connectionSuccessful, ret);
        mDev->setConnected(connectionSuccessful);

        if (connectionSuccessful) {
            emit stopDetecting();
        }

    } else {
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

    /***************\
     * Controllers *
    \***************/

    controllerChannel = new ControllerChannel(mDev);
    controllerBoard = new ControllerBoard(mDev);
    controllerDevice = new ControllerDevice(mDev);

    connect(mainWindow->getChessaboard(), &Chessboard::allChannelsClicked, controllerChannel, &ControllerChannel::onAllChannelsClicked);
    connect(mainWindow->getChessaboard(), &Chessboard::oneRowClicked, controllerChannel, &ControllerChannel::onOneRowClicked);
    connect(mainWindow->getChessaboard(), &Chessboard::oneBoardClicked, controllerChannel, &ControllerChannel::onOneBoardClicked);
    connect(mainWindow->getChessaboard(), &Chessboard::singleChannelClicked, controllerChannel, &ControllerChannel::onSingleChannelClicked);
    connect(mainWindow->getDeviceControlsDockWidget(), &DeviceControlDockWidget::sigVcCurrentRangeSelected, controllerDevice, &ControllerDevice::onVcCurrentRangeSelected);
    connect(controllerChannel, &ControllerChannel::sigUpdateChannelControlDockWidget, mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::onUpdate);
    connect(mainWindow->getBoardControlsDockWidget(), &BoardControlDockWidget::sigGateSourceVoltagesApplied, controllerBoard, &ControllerBoard::onGateSourceVoltagesApplied);

    connect(mainWindow, &MainWindow::setDebugBit, this, [=] (int word, int bit, bool flag) {
        mDev->getMessageDispatcher()->setDebugBit(word, bit, flag);
    });
    connect(mainWindow, &MainWindow::setDebugWord, this, [=] (int word, int value) {
        mDev->getMessageDispatcher()->setDebugWord(word, value);
    });
    connect(mainWindow, &MainWindow::debugInitialization, this, [=] () {
        mDev->getMessageDispatcher()->initializeDevice();
    });

    /**************************\
     * Producer and Consumers *
    \**************************/

    deviceDataProducer = new DeviceDataProducer(mDev);
    stampPlotConsumer = new GapFreePlotConsumer(mDev, deviceDataProducer);

    connect(stampPlotConsumer, &GapFreePlotConsumer::setPlotData, mainWindow->getChessaboard(), &Chessboard::onSetGapFreePlotData);
    connect(stampPlotConsumer, &GapFreePlotConsumer::plotDataUpdated, mainWindow->getChessaboard(), &Chessboard::onReplot);

    vector <RangedMeasurement_t> vcCurrentRanges;
    vector <RangedMeasurement_t> vcVoltageRanges;
    vector <Measurement_t> samplingRates;

    /*! \todo FCON carico come valori di default i primi disponibili per le varie feature, meglio allineare prima il model e prendere i valori da lì */
    mDev->getVcCurrentRangesFeatures(vcCurrentRanges);
    mDev->getVcVoltageRangesFeatures(vcVoltageRanges);
    mDev->getSamplingRatesFeatures(samplingRates);

    stampPlotConsumer->onCurrentRangeChanged(vcCurrentRanges[0]);
    stampPlotConsumer->onVoltageRangeChanged(vcVoltageRanges[0]);
    stampPlotConsumer->onSamplingRateChanged(samplingRates[0]);
    stampPlotConsumer->onDurationChanged({2.0, UnitPfxNone, "s"});
    stampPlotConsumer->forceAxisUpdate();

    mainWindow->getChessaboard()->onRangeUpdated(vcCurrentRanges[0]);
    mainWindow->getChessaboard()->onDurationUpdated({2.0, UnitPfxNone, "s"});

    deviceDataProducer->start();
    stampPlotConsumer->onStartConsuming();
}

void ControllerMain::onMainWindowDestroyed() {
    if (deviceDataProducer!= nullptr) {
        deviceDataProducer->onStopProducing();
    }

    if (stampPlotConsumer!= nullptr) {
        stampPlotConsumer->onStopConsuming();
    }

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

    mDev->flushBoardList();

    emit startDetecting();
}
