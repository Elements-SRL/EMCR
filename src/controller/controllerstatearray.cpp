#include "controller/controllerstatearray.h"
#include "view/statearray/statearraywidget.h"
#include <iostream>
#include <fstream>
#include "model/state.h"
#include "model/statearray.h"
#include <QAction>
#include "iostream"

ControllerStateArray::ControllerStateArray()
{
    stateArrayWidget = new StateArrrayWidget(nullptr, stateArray.states[0]);
    connect(stateArrayWidget, &StateArrrayWidget::sigOpenFileButtonPressed, this, [=](std::string s){
        this->open(s);
    });
}

void ControllerStateArray::showWidget(){
    stateArrayWidget->show();
}

void ControllerStateArray::printYaml(){
    YAML::Node node;
    node = stateArray;
    std::cout << node << std::endl;
}

void ControllerStateArray::open(std::string fname){
    stateArray = readStateArrayFromFile(fname);
}

void ControllerStateArray::writeToFile(std::string fname){
    writeStateArrayToFile(stateArray, fname);
}

void ControllerStateArray::insertState(int idx, YAML::State s){
//    stateArray = insertState(stateArray, s, idx);
    stateArray.states.insert(stateArray.states.begin()+idx, s);
    stateArrayWidget->setStateChecboxesRanges(0, stateArray.states.size()-1);
    stateArrayWidget->setStateCount(stateArray.states.size());
    stateArrayWidget->setState(s);
}

void ControllerStateArray::deleteState(int idx){
    if (stateArray.states.size() <= 1){
        return;
    }
    stateArray.states.erase(stateArray.states.begin()+idx);
    stateArrayWidget->setStateChecboxesRanges(0, stateArray.states.size()-1);
    stateArrayWidget->setStateCount(stateArray.states.size());
    stateArrayWidget->setState(stateArray.states.front());
}
