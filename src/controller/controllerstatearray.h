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
    void deleteState(int idx);
    void insertState(int idx);
    void open();
    void saveAs();
    void start();
    void cance();

private:
    StateArrrayWidget * stateArrayWidget;
    std::vector<YAML::State> stateArray;
};
#endif // CONTROLLERSTATEARRAY_H
