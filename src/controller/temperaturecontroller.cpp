#include "temperaturecontroller.h"

TemperatureController::TemperatureController(ApplicationStatus * appStatus, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    view = new TemperatureDockWidget(mainWindow);
    mainWindow->setDockWidget(MainWindow::DWTemperatureSensors, view, true, Qt::RightDockWidgetArea);

    view->setChannels(appStatus->getTemperatureChannelsNum());
    e384cl::RangedMeasurement_t fanSpeedRange;
    appStatus->getMessageDispatcher()->getCoolingFansSpeedRange(fanSpeedRange);
    maxFanSpeed = fanSpeedRange.getMax().getNoPrefixValue();
    view->enableFansControls(fanSpeedRange);
    speedSet = {maxFanSpeed, e384cl::UnitPfxNone, "rpm"};
    appStatus->getMessageDispatcher()->setCoolingFansSpeed(speedSet, true);

    connect(view, &TemperatureDockWidget::sigSetFanSpeed, this, [=](e384cl::Measurement_t speed) {
        speedSet = speed;
        appStatus->getMessageDispatcher()->setCoolingFansSpeed(speed, true);
    });

    connect(view, &TemperatureDockWidget::sigEnableKTControl, this, &TemperatureController::onEnableKTControl);
    connect(view, &TemperatureDockWidget::sigEnableTControl, this, &TemperatureController::onEnableTControl);
    connect(view, &TemperatureDockWidget::sigPidParams, this, &TemperatureController::onPidParams);
}

void TemperatureController::onTemperatureRead(std::vector <e384cl::Measurement_t> values) {
    view->onTemperatureRead(values);

    values[0].convertValue(UnitPfxNone);
    double Tm0 = values[0].value;
    values[1].convertValue(UnitPfxNone);
    double Tm1 = values[1].value;
    auto Ts = view->getTSet();
    Ts.convertValue(UnitPfxNone);
    // qDebug() << Tm0 << Tm1 << speedSet.value;
    if (tControlEnabled) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - then).count();
        if (elapsed < 1800.0) {
            return;
        }
        then = now;

        // if (w < minFanSpeed) {
        //     Rf = offRT;
        // }
        // else {
        //     Rf = maxRT+(minRT-maxRT)*(w-minFanSpeed)/(maxFanSpeed-minFanSpeed);
        // }
        // F22 = F22p-dt/(Rf*Ca);
        // B22 = B22p+dt/(Rf*Ca);

        // auto x1r = x1;
        // x1 = F11*x1r+F12*x2+B11*powerOut;
        // x2 = F21*x1r+F22*x2+B22*Tm1;

        // auto Pk11r = Pk11;
        // auto Pk12r = Pk12;
        // auto Pk21r = Pk21;
        // auto Pk22r = Pk22;
        // Pk11 = F11*(F11*Pk11r+F12*Pk21r)+F12*(F11*Pk12r+F12*Pk22r)+sQ12;
        // Pk12 = F21*(F11*Pk11r+F12*Pk21r)+F22*(F11*Pk12r+F12*Pk22r);
        // Pk21 = F11*(F21*Pk11r+F22*Pk21r)+F12*(F21*Pk12r+F22*Pk22r);
        // Pk22 = F21*(F21*Pk11r+F22*Pk21r)+F22*(F21*Pk12r+F22*Pk22r)+sQ22;
        // y = Tm0-x1;
        // S = Pk11+sR2;
        // K1 = Pk11/S;
        // K2 = Pk21/S;
        // x1 += K1*y;
        // x2 += K2*y;
        // Pk11r = Pk11;
        // Pk12r = Pk12;
        // Pk11 = (1.0-K1)*Pk11r;
        // Pk12 = (1.0-K1)*Pk12r;
        // Pk21 = -K2*Pk11r+Pk21;
        // Pk22 = -K2*Pk12r+Pk22;

        // Tm0 = x1;

        double e = Ts.value-Tm0;
        ie += e;
        ie = std::max(-ieMax, std::min(ieMax, ie));
        double RT = e*pg+ie*ig;
        e384cl::Measurement_t speed = {maxFanSpeed+(minFanSpeed-maxFanSpeed)*(RT-minRT)/(maxRT-minRT), UnitPfxNone, "rpm"};
        appStatus->getMessageDispatcher()->setCoolingFansSpeed(speed, true);
        w = speed.value;
        qDebug() << Tm0 << Ts.value << e << ie << RT << speed.value;
    }
    else {
        x1 = Tm0;
        x2 = 0.5*(Tm0+Tm1);
    }
}

void TemperatureController::onEnableKTControl(bool enable) {
    if (enable == tControlEnabled) {
        return;
    }
    tControlEnabled = enable;
    ie = 0.0;
    then = std::chrono::steady_clock::now();
}

void TemperatureController::onEnableTControl(Measurement_t temperature, bool enable) {
    appStatus->getMessageDispatcher()->setTemperatureControl(temperature, enable);
}

void TemperatureController::onPidParams(double pg, double ig, double ieMax) {
    this->pg = pg;
    this->ig = ig;
    this->ieMax = ieMax;
}
