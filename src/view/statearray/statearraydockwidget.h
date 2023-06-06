#ifndef STATEARRAYDOCKWIDGET_H
#define STATEARRAYDOCKWIDGET_H
#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QWidget>
#include <model/state.h>

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

private:
    int currentStateIdx;
    QSpinBox * insertStateSpinBox;
    QSpinBox * deleteStateSpinBox;
    QSpinBox *stateSpinbox;
    QDoubleSpinBox *voltageSpinbox;
    QSpinBox * numberOfStatesSpinbox;
    QSpinBox * initialStateSpinbox;
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
    void sigStateChanged2(int);
    void sigMaxTrigLeveDoubleSpinbox(double, int);
    void sigMinTrigLevelDoubleSpinbox(double, int);
    void sigvoltageSpinbox(double, int);
    void sigTimeoutStateSpinboxChanged(double, int);
    void sigTimeoutDoubleSpinboxChanged(double, int);
    void sigOpenButtonPressed(std::string fname);
    void sigSaveAsButtonPressed(std::string fname);
    void sigStateChanged(int, int, YAML::State);
    void sigDeleteButtonPressed(int);
    void sigInsertStateAfter(int);
    void sigInitialStateChanged(int);
    void sigStartButtonPressed();
};
#endif // STATEARRAYDOCKWIDGET_H
