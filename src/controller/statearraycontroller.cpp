#include "controller/statearraycontroller.h"
#include "view/statearray/statearraydockwidget.h"
#include <iostream>
#include <fstream>
#include "model/state.h"
#include "model/statearray.h"
#include <QAction>
#include "iostream"

StateArrayController::StateArrayController(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp) {

    if(!msgDisp->isStateArrayAvailable()){
//        TODO SHOULD THIS RETURN AN ERROR?
        return;
    }
    stateArray = {};
    stateArrayDockWidget = new StateArrayDockWidget();
    mainWindow->setStateArrayDw(stateArrayDockWidget);
    updateUI();
    std::vector <RangedMeasurement_t> vcCurrentRangesFeatures;
    std::vector <RangedMeasurement_t> voltageRanges;
    uint16_t _;
    msgDisp->getVCCurrentRanges(vcCurrentRangesFeatures, _);
    msgDisp->getVCVoltageRanges(voltageRanges);

    RangedMeasurement_t voltageRange = voltageRanges[0];
    RangedMeasurement_t currentRange = vcCurrentRangesFeatures[0];
    voltageRange.convertValues(UnitPfx::UnitPfxNone);
    currentRange.convertValues(UnitPfx::UnitPfxPico);
    stateArrayDockWidget->setRanges(voltageRange.min, voltageRange.max,currentRange.min, currentRange.max);

    connect(stateArrayDockWidget, &StateArrayDockWidget::sigOpenButtonPressed, this, [=](std::string s){
        open(s);
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigSaveAsButtonPressed, this, [=](std::string s){
        writeToFile(s);
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigStateChanged, this, [=](int idx){
        stateArrayDockWidget->setState(stateArray.states[idx], idx);
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigDeleteButtonPressed, this, [=](int idx){
        deleteState(idx);
        updateUI();
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigInsertStateAfter, this, [=](int idx){
        insertState(idx, {});
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigInitialStateChanged, this, [=](int idx){
        stateArray.initialState = idx;
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigStartButtonPressed, this, [=](){
        auto md = msgDisp;
        md->setStateArrayStructure(stateArray.states.size(), stateArray.initialState);
        for (int i = 0; i < stateArray.states.size(); i++){
            auto s = stateArray.states[i];
            switch (s.triggerType) {
            case YAML::CURRENT:
                md->setSateArrayState(i, {s.voltage,UnitPfxNone, "V"}, s.activeTimeout, s.timeout, s.timeoutState, {s.minTrigLevel, UnitPfxPico, "A"},{s.maxTrigLevel, UnitPfxPico, "A"}, s.triggerState, s.activeTrigger, s.delta);
                break;
            case YAML::CONDUCTANCE:
                md->setSateArrayState(i, {s.voltage,UnitPfxNone, "V"}, s.activeTimeout, s.timeout, s.timeoutState, {s.minTrigLevel*s.voltage, UnitPfxPico, "A"},{s.maxTrigLevel*s.voltage, UnitPfxPico, "A"}, s.triggerState, s.activeTrigger, s.delta);
                break;
            }
        }
        md->startStateArray();
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigActiveTimeoutCheckbox, this, [=](bool activeTimeout, int stateIdx){
        stateArray.states[stateIdx].activeTimeout = activeTimeout;
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigDeltaTriggerCheckbox, this, [=](bool deltaTrigger, int stateIdx){
        stateArray.states[stateIdx].delta = deltaTrigger;
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigActiveTriggerCheckbox, this, [=](bool activeTrigger, int stateIdx){
        stateArray.states[stateIdx].activeTrigger = activeTrigger;
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigMaxTrigLeveDoubleSpinbox, this, [=](double maxTrigLevel, int stateIdx){
        stateArray.states[stateIdx].maxTrigLevel = maxTrigLevel;
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigMinTrigLevelDoubleSpinbox, this, [=](double minTrigLevel, int stateIdx){
        stateArray.states[stateIdx].minTrigLevel = minTrigLevel;
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigvoltageSpinbox, this, [=](double voltage, int stateIdx){
        stateArray.states[stateIdx].voltage = voltage;
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigTimeoutStateSpinboxChanged, this, [=](int newState, int stateIdx){
        stateArray.states[stateIdx].timeoutState = newState;
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigTimeoutDoubleSpinboxChanged, this, [=](double timeout, int stateIdx){
        stateArray.states[stateIdx].timeout = timeout;
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigTriggerTypeChanged, this, [=](std::string triggerType, int stateIdx){
        stateArray.states[stateIdx].triggerType = getTriggerTypeFromString(triggerType);
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigStateArrayCheckBoxClicked, this, [=](bool enableFlag, int chIdx){
        msgDisp->setStateArrayEnabled(chIdx, enableFlag);
    });
    connect(stateArrayDockWidget, &StateArrayDockWidget::sigTriggerStateCheckBoxClicked, this, [=](int value, int chIdx){
        stateArray.states[chIdx].triggerState = value;
    });
}

void StateArrayController::printYaml(){
    YAML::Node node;
    node = stateArray;
    std::cout << node << std::endl;
}

void StateArrayController::open(std::string fname){
    stateArray = readStateArrayFromFile(fname);
    updateUI();
}

void StateArrayController::writeToFile(std::string fname){
    writeStateArrayToFile(stateArray, fname);
}

void StateArrayController::insertState(int idx, YAML::State s){
    const int newIdx = idx+1;
    stateArray.states.insert(stateArray.states.begin()+newIdx, s);
    updateUI();
    stateArrayDockWidget->setState(s, newIdx);
}

void StateArrayController::deleteState(int idx){
    if (stateArray.states.size() <= 1){
        return;
    }
    stateArray.states.erase(stateArray.states.begin()+idx);
    updateUI();
}

void StateArrayController::updateUI(){
    stateArrayDockWidget->setStateChecboxesRanges(0, stateArray.states.size()-1);
    stateArrayDockWidget->setStateCount(stateArray.states.size());
    stateArrayDockWidget->setState(stateArray.states[0], 0);
}

StateArrayController::~StateArrayController(){
    delete stateArrayDockWidget;
    stateArrayDockWidget = nullptr;
}
