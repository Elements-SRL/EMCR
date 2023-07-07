#ifndef BIGPLOTCONTROLLER_H
#define BIGPLOTCONTROLLER_H

#include "bigplotwidget.h"
#include "mainwindow.h"
#include "messagedispatcher.h"

class BigPlotController {
public:
    BigPlotController(MessageDispatcher * msgDisp, MainWindow * mainWindow);

private:
    MessageDispatcher * msgDisp = nullptr;
    MainWindow * mainWindow = nullptr;
    BigPlotWidget * bigPlotWidget = nullptr;
};

#endif // BIGPLOTCONTROLLER_H
