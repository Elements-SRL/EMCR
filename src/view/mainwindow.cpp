#include "mainwindow.h"

#include <QBoxLayout>
#include <QSettings>

#include "messagedispatcher.h"
#include "elementslogowidget.h"
#include "errormanager.h"
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
    deviceDetectorHl->addWidget(connectBtn);

    deviceDetectorHl->addItem(new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));
}

MainWindow::~MainWindow() {

}

void MainWindow::setModelDevice(ModelDevice * modelDevice) {
    mDev = modelDevice;
}

QPushButton * MainWindow::getConnectButton() {
    return connectBtn;
}

QString MainWindow::getSelectedSerialNumber() {
    return devicesComboBox->itemText(devicesComboBox->currentIndex());
}

void MainWindow::onDevicesListChanged(vector <string> devicesList) {
    if (devicesList.size() > 0) {
        devicesComboBox->clear();

        for (unsigned int idx = 0; idx < devicesList.size(); idx++) {
            QString deviceName = QString::fromStdString(devicesList[idx]);
            devicesComboBox->addItem(deviceName);
        }
        devicesComboBox->setEnabled(true);
        connectBtn->setEnabled(true);

    } else {
        devicesComboBox->clear();

        devicesComboBox->setEnabled(false);
        connectBtn->setEnabled(false);
    }
}

void MainWindow::onSetConnectedDeviceIdx(int idx) {
    devicesComboBox->setCurrentIndex(idx);
}

void MainWindow::onConnect(bool flag, e384cl::ErrorCodes_t err) {
    QString serial = devicesComboBox->itemText(devicesComboBox->currentIndex());

    if (flag) {
        if (err == e384cl::Success) {
            connectBtn->setText("Disconnect");

            devicesComboBox->setEnabled(false);
            this->createGuiControls();
            connectBtn->setChecked(true);

        } else {
            ErrorManager e(err);
            connectBtn->setChecked(false);
        }

    } else {
        connectBtn->setText(QString::fromStdString("Connect"));
        connectBtn->setChecked(false);

        //            this->destroyGuiControls();
        devicesComboBox->setEnabled(true);
    }
}

void MainWindow::createGuiControls() {
    QSettings settings;

    this->setStyleSheet("QSplitter::handle{image: url(:/imgs/splitter handle.png)}");

    mDev->getChannelsNumber(voltageChannelsNum, currentChannelsNum);

    dockWidgets.clear();







    emit widgetsCreated();
}
