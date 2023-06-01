#include "statearray/statearraywidget.h"
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

StateArrayWidget::StateArrayWidget(QWidget *parent, YAML::State s, int initialState)
    : QWidget(parent)
{
//    TODO LROSSI get initial value from constructor
    currentStateIdx = 0;
    // Constructor implementation
    QVBoxLayout * mainLayout = new QVBoxLayout(this);

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
    initialStateSpinbox->setValue(initialState);
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
    stateSpinBox = new QSpinBox(this);
    connect(stateSpinBox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [=](int value){
        emit this->sigStateChanged(value, currentStateIdx, {activeTimeoutCheckbox->isChecked(),
                                                            timeoutLineEdit->text().toDouble(),
                                                            timeoutStateSpinBox->value(),
                                                            activeTriggerCheckbox->isChecked(),
                                                            deltaTriggerCheckbox->isChecked(),
                                                            minTrigLevelLineEdit->text().toDouble(),
                                                            maxTrigLevelLineEdit->text().toDouble(),
                                                            triggerStateSpinBox->value(),
                                                            getTriggerTypeFromString(triggerTypeComboBox->currentText().toStdString()),
                                                            voltageSpinbox->value()
                                                            });
        currentStateIdx = value;
//        emit this->sigStateChanged(value, {});

    });
    // Set the range and properties of the QDoubleSpinBox

    // Add the QDoubleSpinBox to the QHBoxLayout
    stateLayout->addWidget(stateSpinBox);

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
    timeoutLineEdit = new QLineEdit(this);
    timeoutLineEdit->setValidator(new QIntValidator(0, 9999, this)); // Restrict input to integers

    // Add the label and line edit for the timeout to the timeoutLayout
    timeoutLayout->addWidget(timeoutLabel);
    timeoutLayout->addWidget(timeoutLineEdit);

    // Create the second QLineEdit for the timeout state
    QVBoxLayout *timeoutStateLayout = new QVBoxLayout();
    QLabel *timeoutStateLabel = new QLabel("Timeout state:", this);
    timeoutStateSpinBox = new QSpinBox(this);

    // Add the label and line edit for the timeout state to the timeoutStateLayout
    timeoutStateLayout->addWidget(timeoutStateLabel);
    timeoutStateLayout->addWidget(timeoutStateSpinBox);

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
    minTrigLevelLineEdit = new QLineEdit();
    QLabel *maxTrigLevelLabel = new QLabel("Max Trig Level");
    maxTrigLevelLineEdit = new QLineEdit();

    triggerLevelsLayout->addWidget(minTriggerLevelLabel);
    triggerLevelsLayout->addWidget(minTrigLevelLineEdit);
    triggerLevelsLayout->addWidget(maxTrigLevelLabel);
    triggerLevelsLayout->addWidget(maxTrigLevelLineEdit);
    triggerLayout->addLayout(triggerLevelsLayout);

    QVBoxLayout * triggerStateTypeLayout = new QVBoxLayout();
    QLabel *triggerStateLabel = new QLabel("Trigger State");
    triggerStateSpinBox = new QSpinBox();
    QLabel *triggerType = new QLabel("Trigger Type");
    triggerTypeComboBox = new QComboBox();
    QStringList l = getListOfTriggerStates(s, new QStringList());
    triggerTypeComboBox->addItems(l);

    triggerStateTypeLayout->addWidget(triggerStateLabel);
    triggerStateTypeLayout->addWidget(triggerStateSpinBox);
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
    QSpacerItem * spacer = new QSpacerItem(20, 40);

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
    setLayout(mainLayout);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setState(s);
}

void StateArrayWidget::setState(YAML::State s){
    numberOfStatesSpinbox->setEnabled(false);
    voltageSpinbox->setValue(s.voltage);
    timeoutLineEdit->setText(QString::fromStdString(std::to_string(s.timeout)));
    activeTimeoutCheckbox->setChecked(s.activeTimeout);
    timeoutStateSpinBox->setValue(s.timeoutState);
    activeTriggerCheckbox ->setChecked(s.activeTrigger);
    deltaTriggerCheckbox->setChecked(s.delta);
    minTrigLevelLineEdit->setText(QString::fromStdString(std::to_string(s.minTrigLevel)));
    maxTrigLevelLineEdit->setText(QString::fromStdString(std::to_string(s.maxTrigLevel)));
    triggerStateSpinBox->setValue(s.triggerState);

//    triggerTypeComboBox->setText(QString::fromStdString(std::to_string(s.getTriggerType())));
}

void StateArrayWidget::insertStateAfter(){
//    emit signal to controllerstateArray
}

void StateArrayWidget::setStateCount(int count){
    numberOfStatesSpinbox->setValue(count);
}

void StateArrayWidget::setStateChecboxesRanges(int min, int max){
    initialStateSpinbox->setRange(min, max);
    insertStateSpinBox->setRange(min, max);
    deleteStateSpinBox->setRange(min, max);
    stateSpinBox->setRange(min, max);
    timeoutStateSpinBox->setRange(min, max);
    triggerStateSpinBox->setRange(min, max);
}

StateArrayWidget::~StateArrayWidget()
{
    // Destructor implementation
}

// Member function implementations
