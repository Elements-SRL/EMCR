#include "statearray/statearraydockwidget.h"
#include <QWidget>
#include <QHBoxLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QPushButton>
#include <QString>
#include <QFileDialog>
#include "model/state.h"

StateArrayDockWidget::StateArrayDockWidget(QWidget *parent)
    : QDockWidget(parent)
{
    QWidget * mainWg = new QWidget();
    mainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setWindowTitle("State Array");

    setWidget(mainWg);

    currentStateIdx = 0;
    // Constructor implementation
    QVBoxLayout * mainLayout = new QVBoxLayout();

    // STATE ARRAY CONFIGURATION
    QHBoxLayout * stateArrayConfigurationLayout = new QHBoxLayout();

    QVBoxLayout * numberOfStatesLayout = new QVBoxLayout();
    QLabel * numberOfStatesLabel = new QLabel("Number of States");
    numberOfStatesSpinbox = new QSpinBox(this);
    numberOfStatesLayout ->addWidget(numberOfStatesLabel);
    numberOfStatesLayout ->addWidget(numberOfStatesSpinbox);
    stateArrayConfigurationLayout->addLayout(numberOfStatesLayout);

    QVBoxLayout * initialStateLayout = new QVBoxLayout();
    QLabel * initialStateLabel = new QLabel("Initial State");
    initialStateSpinbox = new QSpinBox(this);
//    TODO LROSSI min &max bound to numberOfstates
    initialStateLayout ->addWidget(initialStateLabel);
    initialStateLayout ->addWidget(initialStateSpinbox);
    stateArrayConfigurationLayout->addLayout(initialStateLayout);
    connect(initialStateSpinbox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [=](int value){
        emit this->sigInitialStateChanged(value);
    });

    ///////////////// CRUD BUTTONS /////////////////
    QGroupBox *insertDeleteGroupBox = new QGroupBox(this);
    // Create the QHBoxLayout for the checkbox and its label
    QVBoxLayout *insertDeleteLayout = new QVBoxLayout(insertDeleteGroupBox);

    QHBoxLayout * deleteStateLayout = new QHBoxLayout();
    QPushButton * deleteStateButton = new QPushButton("Delete state");
    //    TODO LROSSI min &max bound to numberOfstates
    deleteStateSpinBox = new QSpinBox(this);
    deleteStateLayout->addWidget(deleteStateButton);
    deleteStateLayout->addWidget(deleteStateSpinBox);
    insertDeleteLayout->addLayout(deleteStateLayout);
    connect(deleteStateButton, &QPushButton::clicked, this, [=](){
        emit this->sigDeleteButtonPressed(deleteStateSpinBox->value());
    });
    stateArrayConfigurationLayout->addWidget(insertDeleteGroupBox);

    QHBoxLayout * insertStateLayout = new QHBoxLayout();
    QPushButton * insertStateButton = new QPushButton("Insert state after");
    //    TODO LROSSI min &max bound to numberOfstates
    insertStateSpinBox = new QSpinBox(this);
    insertStateLayout->addWidget(insertStateButton);
    insertStateLayout->addWidget(insertStateSpinBox);
    insertDeleteLayout->addLayout(insertStateLayout);
    connect(insertStateButton, &QPushButton::clicked, this, [=](){
        emit this->sigInsertStateAfter(insertStateSpinBox->value());
    });
    stateArrayConfigurationLayout->addWidget(insertDeleteGroupBox);
    mainLayout->addLayout(stateArrayConfigurationLayout);

    // Create the QHBoxLayout and QDoubleSpinBox objects
    QHBoxLayout *stateLayout = new QHBoxLayout();
    stateSpinbox = new QSpinBox(this);
    connect(stateSpinbox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [=](int value){
        emit this->sigStateChanged(value, currentStateIdx, {activeTimeoutCheckbox->isChecked(),
                                                            timeoutDoubleSpinbox->text().toDouble(),
                                                            timeoutStateSpinbox->value(),
                                                            activeTriggerCheckbox->isChecked(),
                                                            deltaTriggerCheckbox->isChecked(),
                                                            minTrigLevelDoubleSpinbox->text().toDouble(),
                                                            maxTrigLeveDoubleSpinbox->text().toDouble(),
                                                            triggerStateSpinbox->value(),
                                                            getTriggerTypeFromString(triggerTypeComboBox->currentText().toStdString()),
                                                            voltageSpinbox->value()
                                                            });
        currentStateIdx = value;
//        emit this->sigStateChanged(value, {});

    });
    // Set the range and properties of the QDoubleSpinBox

    // Add the QDoubleSpinBox to the QHBoxLayout
    stateLayout->addWidget(stateSpinbox);

    /////////// LINE THAT SEPARATES THE FIRST PART FROM THE SECOND ///////////
    QFrame *firstLine = new QFrame(this);
    firstLine->setFrameShape(QFrame::HLine);
    mainLayout->addWidget(firstLine);

    ////////////////// Voltage //////////////////
    QVBoxLayout *voltageLayout = new QVBoxLayout();

    QLabel *voltageLabel = new QLabel("Voltage (V)");
    voltageSpinbox = new QDoubleSpinBox();
    voltageLayout->addWidget(voltageLabel);
    voltageLayout->addWidget(voltageSpinbox);

    stateLayout->addLayout(voltageLayout);

    ////////////////// Timeout Layout //////////////////
    QGroupBox *timeoutGroupBox = new QGroupBox(this);
    // Create the QHBoxLayout for the checkbox and its label
    QHBoxLayout *activeTimeoutLayout = new QHBoxLayout(timeoutGroupBox);
    activeTimeoutCheckbox = new QCheckBox("Active", this);

    // Add the checkbox and its label to the checkboxLayout
    activeTimeoutLayout->addWidget(activeTimeoutCheckbox);

    // Create the first QLineEdit for the timeout (sec)
    QVBoxLayout *timeoutLayout = new QVBoxLayout();
    QLabel *timeoutLabel = new QLabel("Timeout (sec):", this);
    timeoutDoubleSpinbox = new QDoubleSpinBox(this);
    timeoutDoubleSpinbox->setDecimals(4);
    // Add the label and line edit for the timeout to the timeoutLayout
    timeoutLayout->addWidget(timeoutLabel);
    timeoutLayout->addWidget(timeoutDoubleSpinbox);

    // Create the second QLineEdit for the timeout state
    QVBoxLayout *timeoutStateLayout = new QVBoxLayout();
    QLabel *timeoutStateLabel = new QLabel("Timeout state:", this);
    timeoutStateSpinbox = new QSpinBox(this);

    // Add the label and line edit for the timeout state to the timeoutStateLayout
    timeoutStateLayout->addWidget(timeoutStateLabel);
    timeoutStateLayout->addWidget(timeoutStateSpinbox);

    // Add the checkbox layout, timeout layout, and timeout state layout to the main layout
    activeTimeoutLayout->addLayout(timeoutLayout);
    activeTimeoutLayout->addLayout(timeoutStateLayout);

    //////////////////  Trigger Layout  //////////////////
    ///// Create a container widget (QGroupBox)
    QGroupBox *triggersGroupBox = new QGroupBox(this);
    QHBoxLayout * triggerLayout = new QHBoxLayout(triggersGroupBox);

    QVBoxLayout * triggerCheckboxesLayout = new QVBoxLayout();
    activeTriggerCheckbox = new QCheckBox("Active");
    deltaTriggerCheckbox = new QCheckBox("Delta");
    triggerCheckboxesLayout->addWidget(activeTriggerCheckbox);
    triggerCheckboxesLayout->addWidget(deltaTriggerCheckbox);
    triggerLayout->addLayout(triggerCheckboxesLayout);

    QVBoxLayout * triggerLevelsLayout = new QVBoxLayout();
    QLabel *minTriggerLevelLabel = new QLabel("Min Trig Level");
    minTrigLevelDoubleSpinbox = new QDoubleSpinBox();
    QLabel *maxTrigLevelLabel = new QLabel("Max Trig Level");
    maxTrigLeveDoubleSpinbox = new QDoubleSpinBox();

    triggerLevelsLayout->addWidget(minTriggerLevelLabel);
    triggerLevelsLayout->addWidget(minTrigLevelDoubleSpinbox);
    triggerLevelsLayout->addWidget(maxTrigLevelLabel);
    triggerLevelsLayout->addWidget(maxTrigLeveDoubleSpinbox);
    triggerLayout->addLayout(triggerLevelsLayout);

    QVBoxLayout * triggerStateTypeLayout = new QVBoxLayout();
    QLabel *triggerStateLabel = new QLabel("Trigger State");
    triggerStateSpinbox = new QSpinBox();
    QLabel *triggerType = new QLabel("Trigger Type");
    triggerTypeComboBox = new QComboBox();
    QStringList l = getListOfTriggerStates(new QStringList());
    triggerTypeComboBox->addItems(l);

    triggerStateTypeLayout->addWidget(triggerStateLabel);
    triggerStateTypeLayout->addWidget(triggerStateSpinbox);
    triggerStateTypeLayout->addWidget(triggerType);
    triggerStateTypeLayout->addWidget(triggerTypeComboBox);
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
    cancelButton->setVisible(false);
    QSpacerItem * spacer = new QSpacerItem(20, 40);

    connect(startButton, &QPushButton::clicked, this, [=](){
        emit this->sigStartButtonPressed();
    });
    connect(openButton, &QPushButton::clicked, this, [=](){
        QString filename = QFileDialog::getOpenFileName(nullptr, "Open File", "", "YAML files (*.yaml);;");
        if (filename.isEmpty()) {
            return;
        }
        emit this->sigOpenButtonPressed(filename.toStdString());
    });
    connect(saveAsButton, &QPushButton::clicked, this, [=](){
        QString filename = QFileDialog::getSaveFileName(nullptr, "Save File", "", "YAML files (*.yaml);;");
        if (filename.isEmpty()) {
            return;
        }
        emit this->sigSaveAsButtonPressed(filename.toStdString());
    });

    buttonsLayout->addWidget(openButton);
    buttonsLayout->addWidget(saveAsButton);
    buttonsLayout->addItem(spacer);
    buttonsLayout->addWidget(startButton);
    buttonsLayout->addWidget(cancelButton);

    mainLayout->addLayout(buttonsLayout);

    // Set the QVBoxLayout as the main layout of the widget
    mainWg->setLayout(mainLayout);
}

void StateArrayDockWidget::setState(YAML::State s, int index){
    stateSpinbox->setValue(index);
    numberOfStatesSpinbox->setEnabled(false);
    voltageSpinbox->setValue(s.voltage);
    timeoutDoubleSpinbox->setValue(s.timeout);
    activeTimeoutCheckbox->setChecked(s.activeTimeout);
    timeoutStateSpinbox->setValue(s.timeoutState);
    activeTriggerCheckbox ->setChecked(s.activeTrigger);
    deltaTriggerCheckbox->setChecked(s.delta);
    minTrigLevelDoubleSpinbox->setValue(s.minTrigLevel);
    maxTrigLeveDoubleSpinbox->setValue(s.maxTrigLevel);
    triggerStateSpinbox->setValue(s.triggerState);

//    triggerTypeComboBox->setText(QString::fromStdString(std::to_string(s.getTriggerType())));
}

void StateArrayDockWidget::setStateCount(int count){
    numberOfStatesSpinbox->setValue(count);
}

void StateArrayDockWidget::setStateChecboxesRanges(int min, int max){
    initialStateSpinbox->setRange(min, max);
    insertStateSpinBox->setRange(min, max);
    deleteStateSpinBox->setRange(min, max);
    stateSpinbox->setRange(min, max);
    timeoutStateSpinbox->setRange(min, max);
    triggerStateSpinbox->setRange(min, max);
}

void StateArrayDockWidget::setInitialState(int initialState){
    initialStateSpinbox->setValue(initialState);
}

StateArrayDockWidget::~StateArrayDockWidget()
{
    // Destructor implementation
}

// Member function implementations
