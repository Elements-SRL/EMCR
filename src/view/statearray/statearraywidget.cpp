#include "statearray/statearraywidget.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QPushButton>

StateArrrayWidget::StateArrrayWidget(QWidget *parent)
    : QWidget(parent)
{
    // Constructor implementation
    QVBoxLayout * mainLayout = new QVBoxLayout(this);

    // STATE ARRAY CONFIGURATION
    QHBoxLayout * stateArrayConfigurationLayout = new QHBoxLayout();

    QVBoxLayout * numberOfStatesLayout = new QVBoxLayout();
    QLabel * numberOfStatesLabel = new QLabel("Number of States");
    QSpinBox * numberOfStatesSpinbox = new QSpinBox(this);
    numberOfStatesLayout ->addWidget(numberOfStatesLabel);
    numberOfStatesLayout ->addWidget(numberOfStatesSpinbox);
    stateArrayConfigurationLayout->addLayout(numberOfStatesLayout);

    QVBoxLayout * initialStateLayout = new QVBoxLayout();
    QLabel * initialStateLabel = new QLabel("Initial State");
    QSpinBox * initialStateSpinbox = new QSpinBox(this);
//    TODO LROSSI min &max bound to numberOfstates
    initialStateLayout ->addWidget(initialStateLabel);
    initialStateLayout ->addWidget(initialStateSpinbox);
    stateArrayConfigurationLayout->addLayout(initialStateLayout);


    ///////////////// CRUD BUTTONS /////////////////
    QGroupBox *insertDeleteGroupBox = new QGroupBox(this);
    // Create the QHBoxLayout for the checkbox and its label
    QVBoxLayout *insertDeleteLayout = new QVBoxLayout(insertDeleteGroupBox);

    QHBoxLayout * deleteStateLayout = new QHBoxLayout();
    QPushButton * deleteStateButton = new QPushButton("Delete state");
    //    TODO LROSSI min &max bound to numberOfstates
    QSpinBox * deleteStateSpinBox = new QSpinBox(this);
    deleteStateLayout->addWidget(deleteStateButton);
    deleteStateLayout->addWidget(deleteStateSpinBox);
    insertDeleteLayout->addLayout(deleteStateLayout);

    stateArrayConfigurationLayout->addWidget(insertDeleteGroupBox);

    QHBoxLayout * insertStateLayout = new QHBoxLayout();
    QPushButton * insertStateButton = new QPushButton("Insert state after");
    //    TODO LROSSI min &max bound to numberOfstates
    QSpinBox * insertStateSpinBox = new QSpinBox(this);
    insertStateLayout->addWidget(insertStateButton);
    insertStateLayout->addWidget(insertStateSpinBox);
    insertDeleteLayout->addLayout(insertStateLayout);

    stateArrayConfigurationLayout->addWidget(insertDeleteGroupBox);

    mainLayout->addLayout(stateArrayConfigurationLayout);

    // Create the QHBoxLayout and QDoubleSpinBox objects
    QHBoxLayout *stateLayout = new QHBoxLayout();
    QSpinBox *stateSpinBox = new QSpinBox(this);
    // Set the range and properties of the QDoubleSpinBox
    stateSpinBox->setRange(0.0, 100.0);  // Set the allowed range of values

    // Add the QDoubleSpinBox to the QHBoxLayout
    stateLayout->addWidget(stateSpinBox);

    /////////// LINE THAT SEPARATES THE FIRST PART FROM THE SECOND ///////////
    QFrame *firstLine = new QFrame(this);
    firstLine->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(firstLine);

    ////////////////// Voltage //////////////////
    QVBoxLayout *voltageLayout = new QVBoxLayout();

    QLabel *voltageLabel = new QLabel("Voltage (V)");
    QDoubleSpinBox *voltageSpinbox = new QDoubleSpinBox();

    voltageLayout->addWidget(voltageLabel);
    voltageLayout->addWidget(voltageSpinbox);

    stateLayout->addLayout(voltageLayout);

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

    stateLayout->addLayout(controlLayout);
    mainLayout->addLayout(stateLayout);


    // Set the QHBoxLayout as the main layout of the widget

    /////////// LINE THAT SEPARATES THE FIRST PART FROM THE SECOND ///////////
    QFrame *secondLine = new QFrame(this);
    secondLine->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(secondLine);


    ///////// BUTTONS TO OPEN, SAVE, START AND CANCEL ////////////

    QHBoxLayout * buttonsLayout = new QHBoxLayout();
    QPushButton * openButton = new QPushButton("Open");
    QPushButton * saveAsButton = new QPushButton("Save As");
    QPushButton * startButton = new QPushButton("Start");
    QPushButton * cancelButton = new QPushButton("Cancel");
    QSpacerItem * spacer = new QSpacerItem(20, 40);

    buttonsLayout->addWidget(openButton);
    buttonsLayout->addWidget(saveAsButton);
    buttonsLayout->addItem(spacer);
    buttonsLayout->addWidget(startButton);
    buttonsLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonsLayout);

    // Set the QVBoxLayout as the main layout of the widget
    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

StateArrrayWidget::~StateArrrayWidget()
{
    // Destructor implementation
}

// Member function implementations
