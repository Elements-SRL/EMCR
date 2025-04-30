#include "temperaturecontroller.h"

TemperatureController::TemperatureController(ApplicationStatus * appStatus, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    auto view = new TemperatureDockWidget(mainWindow);
    mainWindow->setDockWidget(MainWindow::DWTemperatureSensors, view, true, Qt::RightDockWidgetArea);

    view->setChannels(appStatus->getTemperatureChannelsNum());

    connect(this, &TemperatureController::sigTemperatureRead, view, &TemperatureDockWidget::onTemperatureRead);
}
