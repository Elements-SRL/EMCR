#include "controller/controllerstatearray.h"
#include "view/statearray/statearraydockwidget.h"
#include <iostream>
#include <fstream>
#include "model/state.h"
#include "model/statearray.h"
#include <QAction>
#include "iostream"

ControllerStateArray::ControllerStateArray(ModelDevice * mDev)
{
    this->mDev = mDev;
    stateArray = {};
}

void ControllerStateArray::setStateArrayWidget(StateArrayDockWidget * stateArrayWidget){
    this->stateArrayWidget = stateArrayWidget;
    updateUI();
    std::vector <RangedMeasurement_t> vcCurrentRangesFeatures;
    std::vector <RangedMeasurement_t> voltageRanges;
    uint16_t _;
    mDev->getVcCurrentRangesFeatures(vcCurrentRangesFeatures, _);
    mDev->getVcVoltageRangesFeatures(voltageRanges);

    RangedMeasurement_t voltageRange = voltageRanges[0];
    RangedMeasurement_t currentRange = vcCurrentRangesFeatures[0];
    voltageRange.convertValues(UnitPfx::UnitPfxNone);
    currentRange.convertValues(UnitPfx::UnitPfxPico);
    stateArrayWidget->setRanges(voltageRange.min, voltageRange.max,currentRange.min, currentRange.max);

    connect(stateArrayWidget, &StateArrayDockWidget::sigOpenButtonPressed, this, [=](std::string s){
        open(s);
    });
    connect(stateArrayWidget, &StateArrayDockWidget::sigSaveAsButtonPressed, this, [=](std::string s){
        writeToFile(s);
    });
    connect(stateArrayWidget, &StateArrayDockWidget::sigStateChanged, this, [=](int idx){
        stateArrayWidget->setState(stateArray.states[idx], idx);
    });
    connect(stateArrayWidget, &StateArrayDockWidget::sigDeleteButtonPressed, this, [=](int idx){
        deleteState(idx);
        updateUI();
    });
    connect(stateArrayWidget, &StateArrayDockWidget::sigInsertStateAfter, this, [=](int idx){
        insertState(idx, {});
    });
    connect(stateArrayWidget, &StateArrayDockWidget::sigInitialStateChanged, this, [=](int idx){
        stateArray.initialState = idx;
    });
    connect(stateArrayWidget, &StateArrayDockWidget::sigStartButtonPressed, this, [=](){
        auto md = mDev->getMessageDispatcher();
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
    connect(stateArrayWidget, &StateArrayDockWidget::sigActiveTimeoutCheckbox, this, [=](bool activeTimeout, int stateIdx){
        stateArray.states[stateIdx].activeTimeout = activeTimeout;
    });
    connect(stateArrayWidget, &StateArrayDockWidget::sigDeltaTriggerCheckbox, this, [=](bool deltaTrigger, int stateIdx){
        stateArray.states[stateIdx].delta = deltaTrigger;
    });
    connect(stateArrayWidget, &StateArrayDockWidget::sigActiveTriggerCheckbox, this, [=](bool activeTrigger, int stateIdx){
        stateArray.states[stateIdx].activeTrigger = activeTrigger;
    });
    connect(stateArrayWidget, &StateArrayDockWidget::sigMaxTrigLeveDoubleSpinbox, this, [=](double maxTrigLevel, int stateIdx){
        stateArray.states[stateIdx].maxTrigLevel = maxTrigLevel;
    });
    connect(stateArrayWidget, &StateArrayDockWidget::sigMinTrigLevelDoubleSpinbox, this, [=](double minTrigLevel, int stateIdx){
        stateArray.states[stateIdx].minTrigLevel = minTrigLevel;
    });
    connect(stateArrayWidget, &StateArrayDockWidget::sigvoltageSpinbox, this, [=](double voltage, int stateIdx){
        stateArray.states[stateIdx].voltage = voltage;
    });
    connect(stateArrayWidget, &StateArrayDockWidget::sigTimeoutStateSpinboxChanged, this, [=](int newState, int stateIdx){
        stateArray.states[stateIdx].timeoutState = newState;
    });
    connect(stateArrayWidget, &StateArrayDockWidget::sigTimeoutDoubleSpinboxChanged, this, [=](double timeout, int stateIdx){
        stateArray.states[stateIdx].timeout = timeout;
    });
    connect(stateArrayWidget, &StateArrayDockWidget::sigTriggerTypeChanged, this, [=](std::string triggerType, int stateIdx){
        stateArray.states[stateIdx].triggerType = getTriggerTypeFromString(triggerType);
    });
    connect(stateArrayWidget, &StateArrayDockWidget::sigStateArrayCheckBoxClicked, this, [=](bool enableFlag, int chIdx){
        mDev->getMessageDispatcher()->setStateArrayEnabled(chIdx, enableFlag);
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
