#ifndef COMPENSATIONCONTROLLER_H
#define COMPENSATIONCONTROLLER_H

#include <QObject>

#include "messagedispatcher.h"
#include "compensationcontroldockwidget.h"
#include "mainwindow.h"

class CompensationController : public QObject {
    Q_OBJECT

public:
    CompensationController(MessageDispatcher * msgDisp, MainWindow * mainWindow);
    CompensationControlDockWidget * getCompensationDockWidget();
    ~CompensationController();

private:
    MessageDispatcher * msgDisp = nullptr;
    CompensationControlDockWidget * compensationControlDockWidget = nullptr;
};
#endif // COMPENSATIONCONTROLLER_H
