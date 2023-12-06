#ifndef STATEARRAYDOCKWIDGET_H
#define STATEARRAYDOCKWIDGET_H
#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QWidget>
#include <model/state.h>
#include "e384commlib_global_addendum.h"

class StateArrayDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    StateArrayDockWidget(QWidget *parent = nullptr);
    ~StateArrayDockWidget();

    void setState(YAML::State, int);
    void setStateChecboxesRanges(int min, int max);
    void setStateCount(int count);    
    void setInitialState(int);
    void setRanges(int minVoltage, int maxVoltage, int minCurrent, int maxCurrent);

private:
    int currentStateIdx;
    QSpinBox * insertStateSpinBox;
    QSpinBox * deleteStateSpinBox;
    QSpinBox *stateSpinbox;
    QDoubleSpinBox *voltageSpinbox;
    QSpinBox * numberOfStatesSpinbox;
    QSpinBox * initialStateSpinbox;
    QDoubleSpinBox * reactionTimeSpinbox;
    QCheckBox * activeTimeoutCheckbox;
    QDoubleSpinBox * timeoutDoubleSpinbox;
    QSpinBox * timeoutStateSpinbox;
    QCheckBox * activeTriggerCheckbox;
    QCheckBox * deltaTriggerCheckbox;
    QDoubleSpinBox * minTrigLevelDoubleSpinbox;
    QDoubleSpinBox * maxTrigLevelDoubleSpinbox;
    QSpinBox * triggerStateSpinbox;
    QComboBox * triggerTypeComboBox;
    // Private member variables and functions

signals:
    void sigActiveTimeoutCheckbox(bool, int);
    void sigDeltaTriggerCheckbox(bool, int);
    void sigActiveTriggerCheckbox(bool, int);
    void sigStateChanged(int);
    void sigMaxTrigLeveDoubleSpinbox(double, int);
    void sigMinTrigLevelDoubleSpinbox(double, int);
    void sigvoltageSpinbox(double, int);
    void sigTimeoutStateSpinboxChanged(double, int);
    void sigTimeoutDoubleSpinboxChanged(double, int);
    void sigOpenButtonPressed(std::string fname);
    void sigSaveAsButtonPressed(std::string fname);
    void sigDeleteButtonPressed(int);
    void sigInsertStateAfter(int);
    void sigInitialStateChanged(int);
    void sigReactionTimeChanged(double value); /*! us */
    void sigStartButtonPressed();
    void sigTriggerTypeChanged(std::string, int);
    void sigStateArrayCheckBoxClicked(bool, int);
    void sigTriggerStateCheckBoxClicked(int, int);
};
#endif // STATEARRAYDOCKWIDGET_H
