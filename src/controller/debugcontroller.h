#ifndef DEBUGCONTROLLER_H
#define DEBUGCONTROLLER_H

#include <QObject>

#include "debugdockwidget.h"
#include "mainwindow.h"

class DebugController : public QObject {
    Q_OBJECT

public:
    DebugController(ApplicationStatus * appStatus, MainWindow * mainWindow);

private:
    ApplicationStatus * appStatus = nullptr;
    MainWindow * mainWindow;

signals:
    void sigWriteDebugData(std::vector <int16_t> debugData);
};

#endif // DEBUGCONTROLLER_H
