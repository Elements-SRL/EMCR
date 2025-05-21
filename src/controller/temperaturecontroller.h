#ifndef TEMPERATURECONTROLLER_H
#define TEMPERATURECONTROLLER_H

#include <chrono>

#include <QObject>

#include "temperaturedockwidget.h"
#include "mainwindow.h"

class TemperatureController : public QObject {
    Q_OBJECT

public:
    TemperatureController(ApplicationStatus * appStatus, MainWindow * mainWindow);

public slots:
    void onTemperatureRead(std::vector <e384cl::Measurement_t> values);
    void onEnableTControl(bool enable);

private:
    ApplicationStatus * appStatus = nullptr;
    MainWindow * mainWindow;

    TemperatureDockWidget * view = nullptr;

    bool tControlEnabled = false;

    std::chrono::time_point <std::chrono::steady_clock> then;

    double ie = 0.0;
    const double pg = -1.0;
    const double ig = -0.5;
    const double ieMax = 1.0;
    double maxFanSpeed = 1.0;
    const double minRT = 0.35;
    Measurement_t speedSet;
};

#endif // TEMPERATURECONTROLLER_H
