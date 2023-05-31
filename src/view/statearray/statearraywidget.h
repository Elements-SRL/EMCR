#ifndef STATEARRAYWIDGET_H
#define STATEARRAYWIDGET_H
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QWidget>
#include <model/state.h>

class StateArrayWidget : public QWidget
{
    Q_OBJECT

public:
    StateArrayWidget(QWidget *parent = nullptr, YAML::State state = {});
    ~StateArrayWidget();

    void setState(YAML::State);
    void insertStateAfter();
    void setStateChecboxesRanges(int min, int max);
    void setStateCount(int count);

private:
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
    void sigStateChanged(int, YAML::State);
    void sigDeleteButtonPressed(int);
};
#endif // STATEARRAYWIDGET_H
