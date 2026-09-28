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
    mainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
    setWindowTitle("State Array");
    setWidget(mainWg);
    setObjectName("stateArrayDockWidget");
    currentStateIdx = 0;

    // Window border management when floating
    connect(this, &QDockWidget::topLevelChanged, this, [mainWg](bool isFloating) {
        if (isFloating) {
            mainWg->setStyleSheet("#customFooter {border: none; }");
        } else {
            mainWg->setStyleSheet("");
        }
    });

    // Main layout
    QVBoxLayout * mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // --- SECTION 1 - Array State machine configuration
    // Contains controls to create a new state
    QVBoxLayout * sectionOneLayout = new QVBoxLayout();
    sectionOneLayout->setContentsMargins(8, 0, 8, 8);
    sectionOneLayout->setSpacing(4);

    // TOTAL STATES - section title
    QFrame* sectionOneHeader = new QFrame();
    sectionOneHeader->setObjectName("smallHeaderContainer");

    QHBoxLayout* sectionOneHeaderLayout = new QHBoxLayout(sectionOneHeader);
    QLabel * sectionOneTitle = new QLabel("Manage states");
    sectionOneTitle->setObjectName("sectionHeader");

    // Readonly number - just a label
    numberOfStatesVal = new QLabel(this);
    numberOfStatesVal->setObjectName("numberOfStates");

    sectionOneHeaderLayout->addWidget(sectionOneTitle);
    sectionOneHeaderLayout->addStretch();
    sectionOneHeaderLayout->addWidget(numberOfStatesVal);

    // Header of section goes directly in mainlayout
    mainLayout->addWidget(sectionOneHeader);

    // ROW 1 - Inital state & reaction time settings
    QGroupBox * stateArrayConfigGroupBox = new QGroupBox(this);
    QVBoxLayout * stateConfigLayout = new QVBoxLayout(stateArrayConfigGroupBox);

    QHBoxLayout * initialStateLayout = new QHBoxLayout();
    QLabel * initialStateLabel = new QLabel("Initial State");
    initialStateSpinbox = new QSpinBox(this);
//    TODO LROSSI min &max bound to numberOfstates
    initialStateLayout ->addWidget(initialStateLabel);
    initialStateLayout ->addWidget(initialStateSpinbox);

    connect(initialStateSpinbox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [=](int value){
        emit this->sigInitialStateChanged(value);
    });

    QHBoxLayout * reactionTimeLayout = new QHBoxLayout();
    QLabel * reactionTimeLabel = new QLabel("Reaction time [us]");
    reactionTimeSpinbox = new QDoubleSpinBox(this);
    reactionTimeSpinbox->setRange(0.0, 100.0);
    reactionTimeSpinbox->setValue(0.0);
    reactionTimeSpinbox->setDecimals(1);
    reactionTimeLayout->addWidget(reactionTimeLabel);
    reactionTimeLayout->addWidget(reactionTimeSpinbox);

    connect(reactionTimeSpinbox, QOverload <double> ::of(&QDoubleSpinBox::valueChanged), this, [=](double value){
        emit this->sigReactionTimeChanged(value);
    });

    stateConfigLayout->addLayout(initialStateLayout);
    stateConfigLayout->addLayout(reactionTimeLayout);

    // ROW 2 - ADD/DELETE state buttons
    QGroupBox *insertDeleteGroupBox = new QGroupBox(this);
    QVBoxLayout *insertDeleteLayout = new QVBoxLayout(insertDeleteGroupBox);

    QHBoxLayout * deleteStateLayout = new QHBoxLayout();
    QPushButton * deleteStateButton = new QPushButton("Delete state");
    deleteStateButton->setObjectName("deleteStateButton");

    //    TODO LROSSI min &max bound to numberOfstates
    deleteStateSpinBox = new QSpinBox(this);
    deleteStateLayout->addWidget(deleteStateButton);
    deleteStateLayout->addStretch();
    deleteStateLayout->addWidget(deleteStateSpinBox);
    insertDeleteLayout->addLayout(deleteStateLayout);
    connect(deleteStateButton, &QPushButton::clicked, this, [=](){
        emit this->sigDeleteButtonPressed(deleteStateSpinBox->value());
    });

    QHBoxLayout * insertStateLayout = new QHBoxLayout();
    QPushButton * insertStateButton = new QPushButton("Insert state after");
    insertStateButton->setObjectName("insertStateButton");
    //    TODO LROSSI min &max bound to numberOfstates
    insertStateSpinBox = new QSpinBox(this);
    insertStateLayout->addWidget(insertStateButton);
    insertStateLayout->addStretch();
    insertStateLayout->addWidget(insertStateSpinBox);
    insertDeleteLayout->addLayout(insertStateLayout);
    connect(insertStateButton, &QPushButton::clicked, this, [=](){
        emit this->sigInsertStateAfter(insertStateSpinBox->value());
    });

    QGroupBox * enableStateArrayGroupBox = new QGroupBox(this);
    enableStateArrayGroupBox->setTitle("Active channels");
    enableStateArrayGroupBox->setObjectName("enableStateArrayGroupBox");
    QVBoxLayout * enableStateArrayLayout = new QVBoxLayout(enableStateArrayGroupBox);
    std::vector<QCheckBox *> checkboxes;
    for(int i=0; i < 4; i++){
        QCheckBox * ch = new QCheckBox(QString("Channel %1").arg(i+1));
        checkboxes.push_back(ch);
        enableStateArrayLayout->addWidget(ch);
        connect(ch, &QCheckBox::clicked, this, [=](bool enabledFlag){
            emit sigStateArrayCheckBoxClicked(enabledFlag, i);
        });
    }

    // Composing section with settings
    sectionOneLayout->addWidget(stateArrayConfigGroupBox);
    sectionOneLayout->addWidget(insertDeleteGroupBox);
    sectionOneLayout->addWidget(enableStateArrayGroupBox);
    mainLayout->addLayout(sectionOneLayout);

    // --- SECTION 2 - controls to set-up a state
    QVBoxLayout * sectionTwoLayout = new QVBoxLayout();
    sectionTwoLayout->setContentsMargins(8, 8, 8, 8);

    // State selector for edit its own settings
    stateSpinbox = new QSpinBox(this);
    stateSpinbox->setObjectName("stateSpinbox");
    stateSpinbox->setPrefix("CURRENT STATE: ");
    connect(stateSpinbox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [=](int value){
        emit sigStateChanged(value);
    });

    // Section 2 header - contains the state selector
    QFrame* sectionTwoHeader = new QFrame();
    sectionTwoHeader->setObjectName("smallHeaderContainer");
    QHBoxLayout* sectionTwoHeaderLayout = new QHBoxLayout(sectionTwoHeader);

    QLabel * sectionTwoTitle = new QLabel("State configuration");
    sectionTwoTitle->setObjectName("sectionHeader");

    sectionTwoHeaderLayout->addWidget(sectionTwoTitle);
    sectionTwoHeaderLayout->addStretch();
    sectionTwoHeaderLayout->addWidget(stateSpinbox);

    // Section 2 header goes in main layout
    mainLayout->addWidget(sectionTwoHeader);

    // VOLTAGE - mandatory setting
    QFrame * voltageContainer = new QFrame(this);
    QHBoxLayout *voltageLayout = new QHBoxLayout(voltageContainer);

    QLabel *voltageLabel = new QLabel("Voltage (V)");
    voltageLabel->setObjectName("voltageLabel");
    voltageSpinbox = new QDoubleSpinBox();
    voltageSpinbox->setDecimals(4);

    voltageLayout->addWidget(voltageLabel);
    voltageLayout->addWidget(voltageSpinbox);

    // TIMEOUT - optional setting
    QGroupBox *timeoutGroupBox = new QGroupBox(this);
    QHBoxLayout *activeTimeoutLayout = new QHBoxLayout(timeoutGroupBox);

    activeTimeoutCheckbox = new QCheckBox("Active", this);
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

    // TRIGGERS - how to switch from states
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
    maxTrigLevelDoubleSpinbox = new QDoubleSpinBox();

    minTrigLevelDoubleSpinbox->setDecimals(1);
    minTrigLevelDoubleSpinbox->setMaximum(1e6);
    minTrigLevelDoubleSpinbox->setMinimum(-1e6);
    maxTrigLevelDoubleSpinbox->setDecimals(1);
    maxTrigLevelDoubleSpinbox->setMaximum(1e6);
    maxTrigLevelDoubleSpinbox->setMinimum(-1e6);
    triggerLevelsLayout->addWidget(minTriggerLevelLabel);
    triggerLevelsLayout->addWidget(minTrigLevelDoubleSpinbox);
    triggerLevelsLayout->addWidget(maxTrigLevelLabel);
    triggerLevelsLayout->addWidget(maxTrigLevelDoubleSpinbox);
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

    sectionTwoLayout->addWidget(voltageContainer);
    sectionTwoLayout->addLayout(controlLayout);
    mainLayout->addLayout(sectionTwoLayout);

    // FOOTER - Control buttons
    QFrame* footerFrame = new QFrame();
    footerFrame->setObjectName("customFooter");
    QHBoxLayout * buttonsLayout = new QHBoxLayout(footerFrame);
    buttonsLayout->setContentsMargins(8, 8, 8, 8);

    // Control Buttons
    QPushButton * openButton = new QPushButton("Open");
    QPushButton * saveAsButton = new QPushButton("Save As");
    QPushButton * startButton = new QPushButton("Start");
    QPushButton * stopButton = new QPushButton("Stop");
    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);

    connect(startButton, &QPushButton::clicked, this, &StateArrayDockWidget::sigStartButtonPressed);
    connect(stopButton, &QPushButton::clicked, this, &StateArrayDockWidget::sigStopButtonPressed);
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
    connect(triggerTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index){
        emit sigTriggerTypeChanged(triggerTypeComboBox->itemText(index).toStdString(), currentStateIdx);
    });
    connect(activeTimeoutCheckbox, &QCheckBox::clicked, this, [=](){
        emit sigActiveTimeoutCheckbox(activeTimeoutCheckbox->isChecked(), currentStateIdx);
    });
    connect(deltaTriggerCheckbox, &QCheckBox::clicked, this, [=](){
        emit sigDeltaTriggerCheckbox(deltaTriggerCheckbox->isChecked(), currentStateIdx);
    });
    connect(activeTriggerCheckbox, &QCheckBox::clicked, this, [=](){
        emit sigActiveTriggerCheckbox(activeTriggerCheckbox->isChecked(), currentStateIdx);
    });

    connect(timeoutDoubleSpinbox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [=](double value){
        emit sigTimeoutDoubleSpinboxChanged(value, currentStateIdx);
    });
    connect(voltageSpinbox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [=](double value){
        emit sigvoltageSpinbox(value, currentStateIdx);
    });
    connect(maxTrigLevelDoubleSpinbox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [=](double value){
        emit sigMaxTrigLeveDoubleSpinbox(value, currentStateIdx);
    });
    connect(minTrigLevelDoubleSpinbox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, [=](double value){
        emit sigMinTrigLevelDoubleSpinbox(value, currentStateIdx);
    });

    connect(timeoutStateSpinbox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [=](int value){
        emit sigTimeoutStateSpinboxChanged(value, currentStateIdx);
    });
    connect(triggerStateSpinbox, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [=](int value){
        emit sigTriggerStateCheckBoxClicked(value, currentStateIdx);
    });

    // Composing footer frame with buttons
    buttonsLayout->addWidget(openButton);
    buttonsLayout->addWidget(saveAsButton);
    buttonsLayout->addWidget(spacer);
    buttonsLayout->addWidget(startButton);
    buttonsLayout->addWidget(stopButton);

    // Add footer to main layout
    mainLayout->addWidget(footerFrame);
    mainLayout->addStretch();

    // Set the main layout of the widget
    mainWg->setLayout(mainLayout);
}

void StateArrayDockWidget::setState(YAML::State s, int index){
    numberOfStatesVal->blockSignals(true);
    numberOfStatesVal->setEnabled(false);
    numberOfStatesVal->blockSignals(false);
    voltageSpinbox->blockSignals(true);
    voltageSpinbox->setValue(s.voltage);
    voltageSpinbox->blockSignals(false);
    timeoutDoubleSpinbox->blockSignals(true);
    timeoutDoubleSpinbox->setValue(s.timeout);
    timeoutDoubleSpinbox->blockSignals(false);
    activeTimeoutCheckbox->blockSignals(true);
    activeTimeoutCheckbox->setChecked(s.activeTimeout);
    activeTimeoutCheckbox->blockSignals(false);
    timeoutStateSpinbox->blockSignals(true);
    timeoutStateSpinbox->setValue(s.timeoutState);
    timeoutStateSpinbox->blockSignals(false);
    activeTriggerCheckbox->blockSignals(true);
    activeTriggerCheckbox ->setChecked(s.activeTrigger);
    activeTriggerCheckbox->blockSignals(false);
    deltaTriggerCheckbox->blockSignals(true);
    deltaTriggerCheckbox->setChecked(s.delta);
    deltaTriggerCheckbox->blockSignals(false);
    minTrigLevelDoubleSpinbox->blockSignals(true);
    minTrigLevelDoubleSpinbox->setValue(s.minTrigLevel);
    minTrigLevelDoubleSpinbox->blockSignals(false);
    maxTrigLevelDoubleSpinbox->blockSignals(true);
    maxTrigLevelDoubleSpinbox->setValue(s.maxTrigLevel);
    maxTrigLevelDoubleSpinbox->blockSignals(false);
    triggerStateSpinbox->blockSignals(true);
    triggerStateSpinbox->setValue(s.triggerState);
    triggerStateSpinbox->blockSignals(false);
    stateSpinbox->blockSignals(true);
    stateSpinbox->setValue(index);
    stateSpinbox->blockSignals(false);
    currentStateIdx = index;
//    triggerTypeComboBox->setText(QString::fromStdString(std::to_string(s.getTriggerType())));
}

void StateArrayDockWidget::setStateCount(int count){
    numberOfStatesVal->setText("TOTAL STATES: " + QString::number(count));
}

void StateArrayDockWidget::setReactiontimeUs(double t){
    reactionTimeSpinbox->setValue(t);
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

void StateArrayDockWidget::setRanges(int minVoltage, int maxVoltage, int minCurrent, int maxCurrent){
    voltageSpinbox->blockSignals(true);
    voltageSpinbox->setMinimum(minVoltage);
    voltageSpinbox->setMaximum(maxVoltage);
    voltageSpinbox->blockSignals(false);
}

StateArrayDockWidget::~StateArrayDockWidget()
{
    // Destructor implementation
}

// Member function implementations
