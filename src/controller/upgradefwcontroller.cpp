#include "upgradefwcontroller.h"

#include "messagedispatcher.h"

UpgradeFwController::UpgradeFwController() {
    view = new UpgradeFwView;
    view->setVisible(false);

    connect(view, &UpgradeFwView::sigUpgradeFw, this, &UpgradeFwController::onUpgradeFw);
    connect(this, &UpgradeFwController::sigEnableView, view, &UpgradeFwView::setEnabled);
    connect(this, &UpgradeFwController::sigUpgradeAvilable, view, &UpgradeFwView::onUpgradeAvailable);
    connect(this, &UpgradeFwController::sigSetMessage, view, &UpgradeFwView::onSetMessage);
}

UpgradeFwController::~UpgradeFwController() {
    if (this->isRunning()) {
        this->quit();
        this->wait();
    }

    if (view != nullptr) {
        delete view;
        view = nullptr;
    }
}

void UpgradeFwController::openView(QString deviceId) {
    this->deviceId = deviceId;
    if (MessageDispatcher::isDeviceUpgradable(deviceId.toStdString()) == Success) {
        view->onUpgradeAvailable(true);
        view->onSetMessage("The device " +
                           deviceId +
                           " can be upgraded.\n"
                           "Click UPGRADE to start the firmware upgrade procedure.\n"
                           "The upgrade can take several minutes, please keep the device\n"
                           "plugged and the computer connected to the power supply\n"
                           "until the procedure has finished.");

    } else {
        view->onUpgradeAvailable(false);
        view->onSetMessage("Device not upgradable.");
    }
    view->exec();
}

void UpgradeFwController::run() {
    ErrorCodes_t ret = MessageDispatcher::upgradeDevice(deviceId.toStdString());
    if (ret == Success) {
        emit sigEnableView(true);
        emit sigUpgradeAvilable(false);
        emit sigSetMessage("Device successfully upgraded!\n"
                           "Please unplug the device and plug it\n"
                           "back before connecting to it.");

    } else {
        emit sigEnableView(true);
        emit sigUpgradeAvilable(true);
        emit sigSetMessage("Device upgrade failed.\n"
                           "If you have tried this more than\n"
                           "once please follow the installation\n"
                           "guide available at our website.");
    }
}

void UpgradeFwController::onUpgradeFw() {
    view->setEnabled(false);
    view->onSetMessage("Upgrading...\n"
                       "Please do not disconnect");
    this->start();
}
