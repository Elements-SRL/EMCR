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
    stateArrayWidget = new StateArrayWidget(nullptr, stateArray.states[0]);
    connect(stateArrayWidget, &StateArrayWidget::sigOpenButtonPressed, this, [=](std::string s){
        this->open(s);
    });
    connect(stateArrayWidget, &StateArrayWidget::sigSaveAsButtonPressed, this, [=](std::string s){
        this->writeToFile(s);
    });
    connect(stateArrayWidget, &StateArrayWidget::sigStateChanged, this, [=](int idx){
        stateArrayWidget->setState(stateArray.states[idx]);
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
    updateUI();
}

void ControllerStateArray::writeToFile(std::string fname){
    writeStateArrayToFile(stateArray, fname);
}

void ControllerStateArray::insertState(int idx, YAML::State s){
//    stateArray = insertState(stateArray, s, idx);
    stateArray.states.insert(stateArray.states.begin()+idx, s);
    updateUI();
    stateArrayWidget->setState(s);
}

void ControllerStateArray::deleteState(int idx){
    if (stateArray.states.size() <= 1){
        return;
    }
    stateArray.states.erase(stateArray.states.begin()+idx);
    updateUI();
}

void ControllerStateArray::updateUI(){
    stateArrayWidget->setStateChecboxesRanges(0, stateArray.states.size()-1);
    stateArrayWidget->setStateCount(stateArray.states.size());
    stateArrayWidget->setState(stateArray.states.front());
}
