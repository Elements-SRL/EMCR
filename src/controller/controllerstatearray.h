#ifndef CONTROLLERSTATEARRAY_H
#define CONTROLLERSTATEARRAY_H

#include "QObject"
#include "model/state.h"
#include "model/statearray.h"
#include "view/statearray/statearraywidget.h"

class ControllerStateArray : public QObject {
    Q_OBJECT

public:
    ControllerStateArray();
    void showWidget();
    void printYaml();
    void deleteState(int idx);
    void insertState(int idx, YAML::State s);
    void open(std::string);
    void saveAs();
    void start();
    void cancel();
    void writeToFile(std::string);

private:
    StateArrayWidget * stateArrayWidget;
    YAML::StateArray stateArray;
    QAction * actionOpenFile = nullptr;
};
#endif // CONTROLLERSTATEARRAY_H
