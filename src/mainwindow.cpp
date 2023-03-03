#include "mainwindow.h"

#include <QBoxLayout>

#include "messagedispatcher.h"
#include "elementslogowidget.h"
#include "globaldefines.h"

MainWindow::MainWindow(QWidget * parent) :
    QMainWindow(parent) {

    /*! This line ensures that showMaximized() in main.cpp works */
    this->setGeometry(0, 0, 800, 600);
    this->setObjectName("mainWindow");

    this->setWindowTitle(QString(GLB_SOFTWARE_NAME) + " " + GLB_SOFTWARE_VERSION_NUMBER);

    this->setCentralWidget(new ElementsLogoWidget);

    this->setCorner(Qt::TopRightCorner, Qt::RightDockWidgetArea);

    /************\
     * menu bar *
    \************/

    QMenuBar * menuBar = this->menuBar();

    /*! View menu */
    menuView = new QMenu("View");
    menuBar->addMenu(menuView);

    /************************\
     * device detector dock *
    \************************/

    deviceDetectorDw = new QDockWidget;
    deviceDetectorDw->setWindowTitle("Connection");
    deviceDetectorDw->setObjectName("deviceDetectorDw");
    menuView->addAction(deviceDetectorDw->toggleViewAction());
    deviceDetectorDw->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    this->addDockWidget(Qt::TopDockWidgetArea, deviceDetectorDw);

    QWidget * deviceDetectorWid = new QWidget;
    deviceDetectorWid->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    deviceDetectorDw->setWidget(deviceDetectorWid);

    QHBoxLayout * deviceDetectorHl = new QHBoxLayout;
    deviceDetectorHl->setContentsMargins(1, 1, 1, 1);
    deviceDetectorHl->setSpacing(3);
    deviceDetectorWid->setLayout(deviceDetectorHl);

    devicesComboBox = new QComboBox;
    devicesComboBox->setEnabled(false);
    devicesComboBox->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    QFontMetrics fm((QFont()));
    devicesComboBox->setFixedWidth(fm.width("device device device"));
    deviceDetectorHl->addWidget(devicesComboBox);

    connectBtn = new QPushButton("Connect");
    connectBtn->setEnabled(false);
    connectBtn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    connectBtn->setCheckable(true);
//    connect(connectBtn, &QPushButton::clicked, this, &MainWindow::onConnect);
    deviceDetectorHl->addWidget(connectBtn);

    deviceDetectorHl->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));

    /*! Set up device detector */
    deviceDetector = new DeviceDetector;
    deviceDetector->moveToThread(&deviceDetectorThread);

    connect(this, &MainWindow::startDetecting, deviceDetector, &DeviceDetector::onStartDetecting);
    connect(this, &MainWindow::stopDetecting, deviceDetector, &DeviceDetector::onStopDetecting);

    connect(deviceDetector, &DeviceDetector::devicesListChanged, this, &MainWindow::onDevicesListChanged);

    deviceDetectorThread.start();

    emit startDetecting();
}

MainWindow::~MainWindow() {
    deviceDetectorThread.quit();
    deviceDetectorThread.wait();

    if (deviceDetector != nullptr) {
        delete deviceDetector;
        deviceDetector = nullptr;
    }

    if (deviceConnected) {
//        this->destroyGuiControls();

        messageDispatcher->disconnectDevice();
        messageDispatcher->deinit();
    }
}

void MainWindow::onDevicesListChanged(vector <string> devicesList) {
    if (devicesList.size() > 0) {
        if (!deviceConnected) {
            devicesComboBox->clear();

            for (unsigned int idx = 0; idx < devicesList.size(); idx++) {
                QString deviceName = QString::fromStdString(devicesList[idx]);
                devicesComboBox->addItem(deviceName);
            }
            devicesComboBox->setEnabled(true);
            connectBtn->setEnabled(true);

        } else {
            QString connectedDeviceName = devicesComboBox->itemText(devicesComboBox->currentIndex());
            devicesComboBox->clear();

            int connectedDeviceIdx = -1;
            for (unsigned int idx = 0; idx < devicesList.size(); idx++) {
                QString deviceName = QString::fromStdString(devicesList[idx]);
                devicesComboBox->addItem(deviceName);
                if (connectedDeviceName == deviceName) {
                    connectedDeviceIdx = (int)idx;
                }
            }
            if (connectedDeviceIdx >= 0) {
                devicesComboBox->setCurrentIndex(connectedDeviceIdx);

            } else {
//                this->onConnect(false);
                connectBtn->setChecked(false);
            }
        }

    } else {
        if (!deviceConnected) {
            devicesComboBox->clear();

            devicesComboBox->setEnabled(false);
            connectBtn->setEnabled(false);
        }
    }
}
