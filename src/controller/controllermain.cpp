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
                emit connectDevice(false, e384cl::Success);
            }
        }
    }
}

void ControllerMain::onConnect(bool flag) {
    QString serial = mainWindow->getSelectedSerialNumber();
    mDev->setSerialNumber(serial);

    if (flag) {
        MessageDispatcher * messageDispatcher;
        e384cl::ErrorCodes_t ret = MessageDispatcher::connectDevice(serial.toStdString(), messageDispatcher);

        mDev->setMessageDispatcher(messageDispatcher);
        uint16_t voltageChannelsNumber;
        uint16_t currentChannelsNumber;
        uint16_t boardsNumber; // da estrarre da MessageDispatcher
        mDev->getMessageDispatcher()->getChannelNumberFeatures(voltageChannelsNumber, currentChannelsNumber);
        mDev->getMessageDispatcher()->getBoardsNumberFeatures(boardsNumber);
        mDev->fillChannelList(boardsNumber, currentChannelsNumber/boardsNumber);

        bool connectionSuccessful = ret == e384cl::Success;
        emit connectDevice(connectionSuccessful, ret);
        mDev->setConnected(connectionSuccessful);

        if (connectionSuccessful) {
            emit stopDetecting();
        }

    } else {
        emit connectDevice(false, e384cl::Success);
        mDev->setConnected(false);

        if (mDev->getMessageDispatcher() != nullptr) {
            mDev->getMessageDispatcher()->disconnect();
            delete mDev->getMessageDispatcher();
            mDev->setMessageDispatcher(nullptr);
        }
    }
}

void ControllerMain::onMainWindowCreated() {
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

    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigAppliedTurnChannelOnOff, controllerChannel, &ControllerChannel::onApplyTurnChannelOnOff);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigAppliedTurnStimulsOnOff, controllerChannel, &ControllerChannel::onApplyTurnStimulusOnOff);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigAppliedTurnDocOnOff, controllerChannel, &ControllerChannel::onApplyTurnDocOnOff);
    connect(mainWindow->getChannelControlsDockWidget(), &ChannelControlDockWidget::sigAppliedVoltageHoldValues, controllerChannel, &ControllerChannel::onApplyVoltageHoldValues);

    connect(mainWindow, &MainWindow::setDebugBit, this, [=] (int word, int bit, bool flag) {
        mDev->getMessageDispatcher()->setDebugBit(word, bit, flag);
    });
    connect(mainWindow, &MainWindow::setDebugWord, this, [=] (int word, int value) {
        mDev->getMessageDispatcher()->setDebugWord(word, value);
    });
    connect(mainWindow, &MainWindow::debugInitialization, this, [=] () {
        mDev->getMessageDispatcher()->initializeDevice();
    });

    deviceDataProducer = new DeviceDataProducer(mDev);

    deviceDataProducer->start();
}

void ControllerMain::onMainWindowDestroyed() {
    if (controllerChannel != nullptr) {
        delete controllerChannel;
        controllerChannel = nullptr;
    }

    if (controllerDevice != nullptr) {
        delete controllerDevice;
        controllerDevice = nullptr;
    }

    mDev->flushBoardList();

    emit startDetecting();
}
