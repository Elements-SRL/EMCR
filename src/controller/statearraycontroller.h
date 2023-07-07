#ifndef STATEARRAYCONTROLLER_H
#define STATEARRAYCONTROLLER_H

#include <QObject>

#include "state.h"
#include "statearray.h"
#include "statearraydockwidget.h"

#include "modeldevice.h"
#include "mainwindow.h"

class StateArrayController : public QObject {
    Q_OBJECT

public:
    StateArrayController(MessageDispatcher * msgDisp, MainWindow * mainWindow);

private:
    MessageDispatcher * msgDisp = nullptr;
    MainWindow * mainWindow;
    StateArrayDockWidget * stateArrayDockWidget;
    YAML::StateArray stateArray;
    QAction * actionOpenFile = nullptr;
    void updateUI();
    void printYaml();
    void deleteState(int idx);
    void insertState(int idx, YAML::State s);
    void open(std::string);
    void saveAs();
    void start();
    void cancel();
    void writeToFile(std::string);
};
#endif // STATEARRAYCONTROLLER_H
