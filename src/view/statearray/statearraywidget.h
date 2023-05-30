#ifndef STATEARRAYWIDGET_H
#define STATEARRAYWIDGET_H
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QWidget>
#include <model/state.h>

class StateArrrayWidget : public QWidget
{
    Q_OBJECT

public:
    StateArrrayWidget(QWidget *parent = nullptr, YAML::State state = {});
    ~StateArrrayWidget();

private:
    YAML::State currentState;
    QSpinBox * numberOfStatesSpinbox;
    QSpinBox * initialStateSpinbox;
    QCheckBox * activeTimeoutCheckbox;
    QLineEdit * timeoutLineEdit;
    QLineEdit *timeoutStateLineEdit;
    QCheckBox * activeTriggerCheckbox;
    QCheckBox * deltaTriggerCheckbox;
    QLineEdit * minTrigLevelLineEdit;
    QLineEdit * maxTrigLevelLineEdit;
    // Private member variables and functions
};
#endif // STATEARRAYWIDGET_H
