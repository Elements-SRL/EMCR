#include "debugcontroller.h"

DebugController::DebugController(ApplicationStatus * appStatus, MainWindow * mainWindow) :
    appStatus(appStatus),
    mainWindow(mainWindow) {

    MessageDispatcher * msgDisp = appStatus->getMessageDispatcher();
    auto view = new DebugDockWidget(mainWindow);
    mainWindow->setDockWidget(MainWindow::DWDebug, view, true, Qt::RightDockWidgetArea);

    connect(view, &DebugDockWidget::setDebugBit, this, [=] (int word, int bit, bool flag) {
        msgDisp->setDebugBit(word, bit, flag);
    });
    connect(view, &DebugDockWidget::setDebugWord, this, [=] (int word, int value) {
        msgDisp->setDebugWord(word, value);
    });
    connect(view, &DebugDockWidget::setDebugTrigger, this, [=] (int bit) {
        msgDisp->setDebugTrigger(bit);
    });
    connect(view, &DebugDockWidget::sigWriteCalibrationEeprom, this, [=] (std::vector <uint32_t> value, std::vector <uint32_t> address, std::vector <uint32_t> size) {
        msgDisp->setCalibrationMode(true);
        msgDisp->writeCalibrationEeprom(value, address, size);
        msgDisp->setCalibrationMode(false);
    });
}
