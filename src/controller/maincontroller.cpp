#include "maincontroller.h"

MainController::MainController() {
    mDev = new ModelDevice();

    /*! Set up device detector */
    deviceDetector = new DeviceDetector;
    deviceDetector->moveToThread(&deviceDetectorThread);

    connect(this, &MainController::startDetecting, deviceDetector, &DeviceDetector::onStartDetecting);
    connect(this, &MainController::stopDetecting, deviceDetector, &DeviceDetector::onStopDetecting);

    deviceDetectorThread.start();

}

void MainController::setMainWindow(MainWindow * mainWindow) {
    this->mainWindow = mainWindow;

    mainWindow->setModelDevice(mDev);

    connect(deviceDetector, &DeviceDetector::devicesListChanged, this, &MainController::onDevicesListChanged);
    connect(this, &MainController::devicesListChanged, mainWindow, &MainWindow::onDevicesListChanged);
    connect(this, &MainController::setConnectedDeviceIdx, mainWindow, &MainWindow::onSetConnectedDeviceIdx);
    connect(this, &MainController::connectDevice, mainWindow, &MainWindow::onConnect);
    connect(mainWindow->getConnectButton(), &QPushButton::clicked, this, &MainController::onConnect);

    connect(mainWindow, &MainWindow::widgetsCreated, this, &MainController::onMainWindowCreated);

    emit startDetecting();
}

MainController::~MainController() {
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

void MainController::onDevicesListChanged(vector <string> devicesList) {
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

void MainController::onConnect(bool flag) {
    QString serial = mainWindow->getSelectedSerialNumber();
    mDev->setSerialNumber(serial);

    if (flag) {
        MessageDispatcher * messageDispatcher;
        e384cl::ErrorCodes_t ret = MessageDispatcher::connectDevice(serial.toStdString(), messageDispatcher);

        mDev->setMessageDispatcher(messageDispatcher);
        /*! \todo MPAX fillare i model */
        uint16_t voltageChannelsNumber;
        uint16_t currentChannelsNumber;
        uint16_t boardsNumber; // da estrarre da MessageDispatcher
        mDev->getMessageDispatcher()->getChannelNumberFeatures(voltageChannelsNumber, currentChannelsNumber);
        mDev->getMessageDispatcher()->getBoardsNumberFeatures(boardsNumber);
        mDev->fillChannelList(boardsNumber, currentChannelsNumber);


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

void MainController::onMainWindowCreated() {
    /*! tante connect */
}

void MainController::onMainWindowDestroyed() {
    emit startDetecting();
}
