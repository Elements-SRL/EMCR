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
}
