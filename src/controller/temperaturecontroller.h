#ifndef TEMPERATURECONTROLLER_H
#define TEMPERATURECONTROLLER_H

#include <QObject>

#include "temperaturedockwidget.h"
#include "mainwindow.h"

class TemperatureController : public QObject {
    Q_OBJECT

public:
    TemperatureController(ApplicationStatus * appStatus, MainWindow * mainWindow);

private:
    ApplicationStatus * appStatus = nullptr;
    MainWindow * mainWindow;

signals:
    void sigTemperatureRead(std::vector <e384cl::Measurement_t> values);
};

#endif // TEMPERATURECONTROLLER_H
