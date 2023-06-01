#include "controller/controllerstatearray.h"
#include "view/statearray/statearraywidget.h"
#include <iostream>
#include <fstream>
#include "model/state.h"
#include "model/statearray.h"
#include <QAction>
#include "iostream"

ControllerStateArray::ControllerStateArray(ModelDevice * mDev)
{
    this->mDev = mDev;
}

void ControllerStateArray::setStateArrayWidget(StateArrayWidget * stateArrayWidget){
    this->stateArrayWidget = stateArrayWidget;
    updateUI();
    connect(stateArrayWidget, &StateArrayWidget::sigOpenButtonPressed, this, [=](std::string s){
        this->open(s);
    });
    connect(stateArrayWidget, &StateArrayWidget::sigSaveAsButtonPressed, this, [=](std::string s){
        this->writeToFile(s);
    });
    connect(stateArrayWidget, &StateArrayWidget::sigStateChanged, this, [=](int idx, int oldIdx, YAML::State oldState){
        stateArray.states[oldIdx] = oldState;
        stateArrayWidget->setState(stateArray.states[idx], idx);
    });
    connect(stateArrayWidget, &StateArrayWidget::sigDeleteButtonPressed, this, [=](int idx){
        deleteState(idx);
        updateUI();
    });
    connect(stateArrayWidget, &StateArrayWidget::sigInsertStateAfter, this, [=](int idx){
        insertState(idx, {});
    });
    connect(stateArrayWidget, &StateArrayWidget::sigInitialStateChanged, this, [=](int idx){
        stateArray.initialState = idx;
    });
    connect(stateArrayWidget, &StateArrayWidget::sigStartButtonPressed, this, [=](){
        auto md = mDev->getMessageDispatcher();
        md->setStateArrayStructure(stateArray.states.size(), stateArray.initialState);
        for (int i = 0; i < stateArray.states.size(); i++){
            auto s = stateArray.states[i];
            md->setSateArrayState(i, s.voltage, s.activeTimeout, s.timeout, s.timeoutState, s.minTrigLevel, s.maxTrigLevel, s.triggerState);
        }
        md->startStateArray();
    });
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
    const int newIdx = idx+1;
    stateArray.states.insert(stateArray.states.begin()+newIdx, s);
    updateUI();
    stateArrayWidget->setState(s, newIdx);
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
    stateArrayWidget->setState(stateArray.states[0], 0);
}
