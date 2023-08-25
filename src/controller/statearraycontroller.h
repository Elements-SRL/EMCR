#ifndef STATEARRAYCONTROLLER_H
#define STATEARRAYCONTROLLER_H

#include <QObject>

#include "state.h"
#include "statearray.h"
#include "statearraydockwidget.h"

#include "messagedispatcher.h"
#include "mainwindow.h"

class StateArrayController : public QObject {
    Q_OBJECT

public:
    StateArrayController(MessageDispatcher * msgDisp, MainWindow * mainWindow);
    ~StateArrayController();
private:
    MessageDispatcher * msgDisp = nullptr;
    StateArrayDockWidget * stateArrayDockWidget;
    YAML::StateArray stateArray;
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
