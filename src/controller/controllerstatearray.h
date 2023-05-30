#ifndef CONTROLLERSTATEARRAY_H
#define CONTROLLERSTATEARRAY_H

#include "QObject"
#include "model/state.h"
#include "view/statearray/statearraywidget.h"

class ControllerStateArray : public QObject {
    Q_OBJECT

public:
    ControllerStateArray();
    void showWidget();
    void printYaml();
    void deleteState(int idx);
    void insertState(int idx);
    void open(std::string);
    void saveAs();
    void start();
    void cancel();
    void writeToFile(std::string);

private:
    StateArrrayWidget * stateArrayWidget;
    std::vector<YAML::State_t> stateArray;
};
#endif // CONTROLLERSTATEARRAY_H
