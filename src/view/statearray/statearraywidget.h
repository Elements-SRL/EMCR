#ifndef STATEARRAYWIDGET_H
#define STATEARRAYWIDGET_H
#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QWidget>
#include <model/state.h>

class StateArrayWidget : public QDockWidget
{
    Q_OBJECT

public:
    StateArrayWidget(QWidget *parent = nullptr);
    ~StateArrayWidget();

    void setState(YAML::State, int);
    void setStateChecboxesRanges(int min, int max);
    void setStateCount(int count);    
    void setInitialState(int);

private:
    int currentStateIdx;
    QSpinBox * insertStateSpinBox;
    QSpinBox * deleteStateSpinBox;
    QSpinBox *stateSpinBox;
    QDoubleSpinBox *voltageSpinbox;
    QSpinBox * numberOfStatesSpinbox;
    QSpinBox * initialStateSpinbox;
    QCheckBox * activeTimeoutCheckbox;
    QLineEdit * timeoutLineEdit;
    QSpinBox * timeoutStateSpinBox;
    QCheckBox * activeTriggerCheckbox;
    QCheckBox * deltaTriggerCheckbox;
    QLineEdit * minTrigLevelLineEdit;
    QLineEdit * maxTrigLevelLineEdit;
    QSpinBox * triggerStateSpinBox;
    QComboBox * triggerTypeComboBox;
    // Private member variables and functions

signals:
    void sigOpenButtonPressed(std::string fname);
    void sigSaveAsButtonPressed(std::string fname);
    void sigStateChanged(int, int, YAML::State);
    void sigDeleteButtonPressed(int);
    void sigInsertStateAfter(int);
    void sigInitialStateChanged(int);
    void sigStartButtonPressed();
};
#endif // STATEARRAYWIDGET_H
