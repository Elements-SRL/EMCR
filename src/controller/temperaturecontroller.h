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
    void onEnableKTControl(bool enable);
    void onEnableTControl(Measurement_t temperature, bool enable);

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
    const double minFanSpeed = 3524.0;
    Measurement_t speedSet;

    /*! kalman */
    const double dt = 2.0;
    const double Rh = 0.009211645116870;
    const double Cb = 4.088122458948907e+02;
    const double Ca = 1.540082025770994e+02;
    const double offRT = 1.582470252096335;
    const double maxRT = 0.390196941234880;
    const double minRT = 0.264962843373366;
    const double powerOut = 50.0;
    const double sR2 = 0.25*0.25/12;
    const double sQ12 = sR2/20.0;
    const double sQ22 = sR2/20.0;

    const double F11 = 1.0-dt/(Rh*Cb);
    const double F12 = dt/(Rh*Cb);
    const double F21 = dt/(Rh*Ca);
    double F22p = 1.0-dt/(Rh*Ca);
    double F22 = 0.0;
    const double B11 = dt/Cb;
    double B22p = 0.0;
    double B22 = 0.0;
    double Rf = offRT;

    double x1 = 0.0;
    double x2 = 0.0;
    double Pk11 = 0.0;
    double Pk12 = 0.0;
    double Pk21 = 0.0;
    double Pk22 = 0.0;
    double y = 0.0;
    double S = 0.0;
    double K1 = 0.0;
    double K2 = 0.0;
    double w = 0.0;
};

#endif // TEMPERATURECONTROLLER_H
