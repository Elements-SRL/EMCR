#ifndef DEVICEDETECTOR_H
#define DEVICEDETECTOR_H

#include <QVector>
#include <QString>
#include <QObject>
#include <QTimer>

#include "messagedispatcher.h"

class DeviceDetector : public QObject {
    Q_OBJECT

public:
    DeviceDetector();
    ~DeviceDetector();

public slots:
    void onStartDetecting();
    void onStopDetecting();

private:
    bool detectFlag = false;
    bool ftdiDriverErrorReported = false; /*!< report the missing FTDI library once, until the situation changes */

    std::vector <std::string> devicesList;
    std::vector <std::string> detectedList;
    QTimer * detectDevicesTmr;

private slots:
    void detectDevices();

signals:
    void devicesListChanged(std::vector <std::string>);
    void ftdiDriverError(QString details);
};

#endif // DEVICEDETECTOR_H
