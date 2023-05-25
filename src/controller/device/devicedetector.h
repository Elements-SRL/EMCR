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

    std::vector <std::string> devicesList;
    std::vector <std::string> detectedList;
    QTimer * detectDevicesTmr;

private slots:
    void detectDevices();

signals:
    void devicesListChanged(std::vector <std::string>);
};

#endif // DEVICEDETECTOR_H
