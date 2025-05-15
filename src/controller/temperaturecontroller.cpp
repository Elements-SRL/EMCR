#include "temperaturecontroller.h"

TemperatureController::TemperatureController(ApplicationStatus * appStatus, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    view = new TemperatureDockWidget(mainWindow);
    mainWindow->setDockWidget(MainWindow::DWTemperatureSensors, view, true, Qt::RightDockWidgetArea);

    view->setChannels(appStatus->getTemperatureChannelsNum());
    e384cl::RangedMeasurement_t fanSpeedRange;
    appStatus->getMessageDispatcher()->getCoolingFansSpeedRange(fanSpeedRange);
    view->enableFansControls(fanSpeedRange);

    connect(view, &TemperatureDockWidget::sigSetFanSpeed, this, [=](e384cl::Measurement_t speed) {
        appStatus->getMessageDispatcher()->setCoolingFansSpeed(speed, true);
    });

    connect(view, &TemperatureDockWidget::sigEnableTControl, this, &TemperatureController::onEnableTControl);
}

void TemperatureController::onTemperatureRead(std::vector <e384cl::Measurement_t> values) {
    view->onTemperatureRead(values);

    values[0].convertValue(UnitPfxNone);
    double Tm = values[0].value;
    auto Ts = view->getTSet();
    Ts.convertValue(UnitPfxNone);
    if (tControlEnabled) {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - then).count();
        if (elapsed < 800.0) {
            return;
        }
        then = now;
        double e = Tm-Ts.value;
        ie += e;
        ie = std::max(-ieMax, std::min(ieMax, ie));
        double RT = std::max(0.1, e*pg+ie*ig);
        e384cl::Measurement_t speed = {maxFanSpeed*minRT/RT, UnitPfxNone, "rpm"};
        appStatus->getMessageDispatcher()->setCoolingFansSpeed(speed, true);
        qDebug() << Tm << Ts.value << e << ie << RT << speed.value;
    }
}

void TemperatureController::onEnableTControl(bool enable) {
    if (enable == tControlEnabled) {
        return;
    }
    tControlEnabled = enable;
    ie = 0.0;
    then = std::chrono::steady_clock::now();
}
