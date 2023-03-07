#ifndef DEVICEDETECTOR_H
#define DEVICEDETECTOR_H

#include <QVector>
#include <QString>
#include <QObject>
#include <QTimer>

#include "messagedispatcher.h"

using namespace std;
namespace e384cl = e384CommLib;

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

    vector <string> devicesList;
    vector <string> detectedList;
    QTimer * detectDevicesTmr;

private slots:
    void detectDevices();

signals:
    void devicesListChanged(vector <string>);
};

#endif // DEVICEDETECTOR_H
