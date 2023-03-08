#include "mainwindow.h"

#include <QBoxLayout>
#include <QSettings>
#include <QTimer>

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

Chessboard * MainWindow::getChessaboard() {
    return chessboard;
}

DeviceControlDockWidget * MainWindow::getControlsDockWidget() {
    return controlsDw;
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

    mDev->getChannelsNumberFeatures(voltageChannelsNum, currentChannelsNum);

    dockWidgets.clear();

    this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

    /*****************\
     * controls dock *
    \*****************/

    controlsDw = new DeviceControlDockWidget(mDev);
    controlsDw->setObjectName("controlsDw");
    this->addDockWidget(Qt::RightDockWidgetArea, controlsDw);
    dockWidgets.append(controlsDw);

    /*********\
     * plots *
    \*********/

    chessboard = new Chessboard(mDev);
    chessboard->setObjectName("chessboard");
    delete this->takeCentralWidget();
    this->setCentralWidget(chessboard);
    this->centralWidget()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    bigPlotDw = new BigPlotDockWidget();
    bigPlotDw->setObjectName("bigPlotDw");
    this->addDockWidget(Qt::BottomDockWidgetArea, bigPlotDw);
    dockWidgets.append(bigPlotDw);

    this->restoreUISettings();

    emit widgetsCreated();
}

void MainWindow::destroyGuiControls() {
    QSettings settings;
    this->saveUISettings();

//    if (plotPreferencesDlg != nullptr) {
//        delete plotPreferencesDlg;
//        plotPreferencesDlg = nullptr;
//    }
//    actionPlotPreferences->setEnabled(false);

//    if (menuFrontEndResetDenoiser->isEnabled()) {
//        settings.setValue("Preferences/frontEndResetDenoiser", actionFrontEndResetDenoiserEnable->isChecked());
//        menuFrontEndResetDenoiser->setEnabled(false);
//    }

//    if (actionDigitalOffsetCompensationAutostop->isEnabled()) {
//        settings.setValue("Preferences/digitalOffsetCompensationAutostop", actionDigitalOffsetCompensationAutostop->isChecked());
//        actionDigitalOffsetCompensationAutostop->setEnabled(false);
//    }

//    menuReset->setEnabled(false);
//    this->removeViewActions();

    for (int dockIdx = 0; dockIdx < dockWidgets.size(); dockIdx++) {
        if (dockWidgets[dockIdx] != nullptr) {
            delete dockWidgets[dockIdx];
            dockWidgets[dockIdx] = nullptr;
        }
    }
    dockWidgets.clear();

    for (int dockIdx = 0; dockIdx < analysisWidgets.size(); dockIdx++) {
        if (analysisWidgets[dockIdx] != nullptr) {
            delete analysisWidgets[dockIdx];
            analysisWidgets[dockIdx] = nullptr;
        }
    }
    analysisWidgets.clear();

//    if (deviceDataProducer!= nullptr) {
//        deviceDataProducer->onStopProducing();
//    }

//    for (int consumerIdx = 0; consumerIdx < consumers.size(); consumerIdx++) {
//        if (consumers[consumerIdx] != nullptr) {
//            consumers[consumerIdx]->onStopConsuming();
//            delete consumers[consumerIdx];
//            consumers[consumerIdx] = nullptr;
//        }
//    }

//    if (deviceDataProducer != nullptr) {
//        delete deviceDataProducer; /*! The destructor also stops the producer thread */
//        deviceDataProducer = nullptr;
//    }

    if (chessboard != nullptr) {
        this->takeCentralWidget();
        delete chessboard;
        chessboard = nullptr;
    }

//    for (int shortcutIdx = 0; shortcutIdx < shortcuts.size(); shortcutIdx++) {
//        if (shortcuts[shortcutIdx] != nullptr) {
//            delete shortcuts[shortcutIdx];
//        }
//    }
//    shortcuts.clear();

    this->setCentralWidget(new ElementsLogoWidget);

    emit widgetsDestroyed();
}

void MainWindow::restoreUISettings() {
    QTimer * timer = new QTimer;
    timer->setInterval(10);
    timer->setSingleShot(true);

    connect(timer, &QTimer::timeout, this, [=] () {
        QSettings settings;
        QString settingsRoot = "Preferences/UI/";
        QString tag;

        for (int dockIdx = 0; dockIdx < dockWidgets.size(); dockIdx++) {
            tag = settingsRoot + dockWidgets[dockIdx]->objectName() + "/geometry";
            if (settings.contains(tag)) {
                dockWidgets[dockIdx]->setGeometry(settings.value(tag).value <QRect> ());
            }
        }

        for (int dockIdx = 0; dockIdx < analysisWidgets.size(); dockIdx++) {
            tag = settingsRoot + analysisWidgets[dockIdx]->objectName() + "/geometry";
            if (settings.contains(tag)) {
                analysisWidgets[dockIdx]->setGeometry(settings.value(tag).value <QRect> ());
            }
        }

        tag = settingsRoot + chessboard->objectName() + "/geometry";
        if (settings.contains(tag)) {
            chessboard->setGeometry(settings.value(tag).value <QRect> ());
        }

        tag = settingsRoot + this->objectName() + "/state";
        this->restoreState(settings.value(tag).toByteArray());
    });

    timer->start();
}

void MainWindow::saveUISettings() {
    QSettings settings;
    QString settingsRoot = "Preferences/UI/";
    QString tag;

    tag = settingsRoot + chessboard->objectName() + "/geometry";
    settings.setValue(tag, QVariant(chessboard->geometry()));

    for (int dockIdx = 0; dockIdx < dockWidgets.size(); dockIdx++) {
        tag = settingsRoot + dockWidgets[dockIdx]->objectName() + "/geometry";
        settings.setValue(tag, QVariant(dockWidgets[dockIdx]->geometry()));
    }

    for (int dockIdx = 0; dockIdx < analysisWidgets.size(); dockIdx++) {
        tag = settingsRoot + analysisWidgets[dockIdx]->objectName() + "/geometry";
        settings.setValue(tag, QVariant(analysisWidgets[dockIdx]->geometry()));
    }

    tag = settingsRoot + this->objectName() + "/state";
    settings.setValue(tag, this->saveState());
}
