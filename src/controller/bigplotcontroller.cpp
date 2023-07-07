#include "bigplotcontroller.h"

BigPlotController::BigPlotController(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp),
    mainWindow(mainWindow) {

    bigPlotWidget = new BigPlotWidget(msgDisp);

    mainWindow->setBigPlotWidget(bigPlotWidget);
}
