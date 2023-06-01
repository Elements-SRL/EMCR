#ifndef CONTROLLERSTATEARRAY_H
#define CONTROLLERSTATEARRAY_H

#include "QObject"
#include "model/state.h"
#include "model/statearray.h"
#include "view/statearray/statearraydockwidget.h"

#include <modeldevice.h>

class ControllerStateArray : public QObject {
    Q_OBJECT

public:
    ControllerStateArray(ModelDevice * mDev);
    void setStateArrayWidget(StateArrayDockWidget * stateArrayWidget);

private:
    ModelDevice *mDev;
    StateArrayDockWidget * stateArrayWidget;
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
#endif // CONTROLLERSTATEARRAY_H
