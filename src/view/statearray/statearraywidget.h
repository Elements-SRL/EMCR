#ifndef STATEARRAYWIDGET_H
#define STATEARRAYWIDGET_H
#include <QCheckBox>
#include <QComboBox>
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

    void setState(YAML::State);
private:
    QDoubleSpinBox *voltageSpinbox;
    QSpinBox * numberOfStatesSpinbox;
    QSpinBox * initialStateSpinbox;
    QCheckBox * activeTimeoutCheckbox;
    QLineEdit * timeoutLineEdit;
    QLineEdit *timeoutStateLineEdit;
    QCheckBox * activeTriggerCheckbox;
    QCheckBox * deltaTriggerCheckbox;
    QLineEdit * minTrigLevelLineEdit;
    QLineEdit * maxTrigLevelLineEdit;
    QLineEdit * triggerStateLineEdit;
    QComboBox * triggerTypeComboBox;
    // Private member variables and functions
};
#endif // STATEARRAYWIDGET_H
