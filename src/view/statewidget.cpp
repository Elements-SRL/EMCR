#include "statewidget.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>

StateWidget::StateWidget(QWidget *parent)
    : QWidget(parent)
{
    // Constructor implementation

    // Create the QHBoxLayout and QDoubleSpinBox objects
    QHBoxLayout *mainLayout = new QHBoxLayout(this);

    QSpinBox *stateSpinBox = new QSpinBox(this);
    // Set the range and properties of the QDoubleSpinBox
    stateSpinBox->setRange(0.0, 100.0);  // Set the allowed range of values

    // Add the QDoubleSpinBox to the QHBoxLayout
    mainLayout->addWidget(stateSpinBox);

    ////////////////// Voltage //////////////////
    QVBoxLayout *voltageLayout = new QVBoxLayout();

    QLabel *voltageLabel = new QLabel("Voltage (V)");
    QDoubleSpinBox *voltageSpinbox = new QDoubleSpinBox();

    voltageLayout->addWidget(voltageLabel);
    voltageLayout->addWidget(voltageSpinbox);

    mainLayout->addLayout(voltageLayout);

    ////////////////// Timeout Layout //////////////////
    QGroupBox *timeoutGroupBox = new QGroupBox(this);
    // Create the QHBoxLayout for the checkbox and its label
    QHBoxLayout *activeTimeoutLayout = new QHBoxLayout(timeoutGroupBox);
    QCheckBox *activeTimeoutCheckbox = new QCheckBox("Active", this);

    // Add the checkbox and its label to the checkboxLayout
    activeTimeoutLayout->addWidget(activeTimeoutCheckbox);

    // Create the first QLineEdit for the timeout (sec)
    QVBoxLayout *timeoutLayout = new QVBoxLayout();
    QLabel *timeoutLabel = new QLabel("Timeout (sec):", this);
    QLineEdit *timeoutLineEdit = new QLineEdit(this);
    timeoutLineEdit->setValidator(new QIntValidator(0, 9999, this)); // Restrict input to integers

    // Add the label and line edit for the timeout to the timeoutLayout
    timeoutLayout->addWidget(timeoutLabel);
    timeoutLayout->addWidget(timeoutLineEdit);

    // Create the second QLineEdit for the timeout state
    QVBoxLayout *timeoutStateLayout = new QVBoxLayout();
    QLabel *timeoutStateLabel = new QLabel("Timeout state:", this);
    QLineEdit *timeoutStateLineEdit = new QLineEdit(this);
    timeoutStateLineEdit->setValidator(new QIntValidator(0, 9999, this)); // Restrict input to integers

    // Add the label and line edit for the timeout state to the timeoutStateLayout
    timeoutStateLayout->addWidget(timeoutStateLabel);
    timeoutStateLayout->addWidget(timeoutStateLineEdit);

    // Add the checkbox layout, timeout layout, and timeout state layout to the main layout
    activeTimeoutLayout->addLayout(timeoutLayout);
    activeTimeoutLayout->addLayout(timeoutStateLayout);

//    mainLayout->addLayout(activeTimeoutLayout);
    //////////////////  Trigger Layout  //////////////////
    /// \brief triggerLayout
    ///// Create a container widget (QGroupBox)
    QGroupBox *triggersGroupBox = new QGroupBox(this);
    QHBoxLayout * triggerLayout = new QHBoxLayout(triggersGroupBox);

    QVBoxLayout * triggerCheckboxesLayout = new QVBoxLayout();
    QCheckBox * activeTriggerCheckbox = new QCheckBox("Active");
    QCheckBox * deltaTriggerCheckbox = new QCheckBox("Delta");

    triggerCheckboxesLayout->addWidget(activeTriggerCheckbox);
    triggerCheckboxesLayout->addWidget(deltaTriggerCheckbox);
    triggerLayout->addLayout(triggerCheckboxesLayout);

//    mainLayout->addLayout(triggerLayout);

    QVBoxLayout * triggerLevelsLayout = new QVBoxLayout();
    QLabel *minTriggerLevelLabel = new QLabel("Min Trig Level");
    QLineEdit * minTrigLevelLineEdit = new QLineEdit();
    QLabel *maxTrigLevelLabel = new QLabel("Max Trig Level");
    QLineEdit * maxTrigLevelLineEdit = new QLineEdit();

    triggerLevelsLayout->addWidget(minTriggerLevelLabel);
    triggerLevelsLayout->addWidget(minTrigLevelLineEdit);
    triggerLevelsLayout->addWidget(maxTrigLevelLabel);
    triggerLevelsLayout->addWidget(maxTrigLevelLineEdit);
    triggerLayout->addLayout(triggerLevelsLayout);

    QVBoxLayout * triggerStateTypeLayout = new QVBoxLayout();
    QLabel *triggerStateLabel = new QLabel("Trigger State");
    QLineEdit * triggerStateLineEdit = new QLineEdit();
    QLabel *triggerType = new QLabel("Trigger Type");
    QLineEdit * triggerTypeLineEdit = new QLineEdit();

    triggerStateTypeLayout->addWidget(triggerStateLabel);
    triggerStateTypeLayout->addWidget(triggerStateLineEdit);
    triggerStateTypeLayout->addWidget(triggerType);
    triggerStateTypeLayout->addWidget(triggerTypeLineEdit);
    triggerLayout->addLayout(triggerStateTypeLayout);

    QVBoxLayout * controlLayout = new QVBoxLayout();
    controlLayout->addWidget(timeoutGroupBox);
    controlLayout->addWidget(triggersGroupBox);

    mainLayout->addLayout(controlLayout);

    // Set the QHBoxLayout as the main layout of the widget
    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

StateWidget::~StateWidget()
{
    // Destructor implementation
}

// Member function implementations
