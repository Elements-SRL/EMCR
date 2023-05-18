#include "protocoleditor.h"

#include <QSplitter>

#include "protocolwidget.h"

ProtocolEditor::ProtocolEditor(e4gcl::CommLib * commLib, ProtocolWidget * protocolWidget, QString name) :
    commLib(commLib),
    parentWidget(protocolWidget),
    name(name) {

    /*! Hide the help and close button on the window bar */
    this->setWindowFlags(Qt::WindowTitleHint);

    this->setWindowTitle(name + " editor");

    this->setModal(true);

    mainVl = new QVBoxLayout;
    this->setLayout(mainVl);

    editorHl = new QHBoxLayout;
    mainVl->addLayout(editorHl);

    /*! Library */
    QVBoxLayout * libraryVl = new QVBoxLayout;
    editorHl->addLayout(libraryVl);

    titlesFont.setPointSize(12);

    QLabel * libraryTitle = new QLabel("Library");
    libraryTitle->setFont(titlesFont);
    libraryVl->addWidget(libraryTitle);

    libraryPidl = new ProtocolItemDragList();
    libraryVl->addWidget(libraryPidl);

    /*! Protocol wide controls */
    QLabel * protocolWideTitle = new QLabel("Protocol wide controls");
    protocolWideTitle->setFont(titlesFont);
    libraryVl->addWidget(protocolWideTitle);

    protocolWideCtrlsGl = new QGridLayout;

    int sweepsNum = 1;
    sweepsNumName = new QLabel("Sweeps");
    sweepsNumEdit = new QSpinBox();
    sweepsNumEdit->setRange(1, 100);
    sweepsNumEdit->setValue(sweepsNum);

    protocolWideCtrlsGl->addWidget(sweepsNumName, PTE_SWEEPS_ROW, 0, Qt::AlignRight);
    protocolWideCtrlsGl->addWidget(sweepsNumEdit, PTE_SWEEPS_ROW, 1);

    e4gcl::ErrorCodes_t ret = e4gcl::Success;

    vector <e4gcl::Measurement_t> availableSamplingRates;
    ret = commLib->getSamplingRates(availableSamplingRates);

    if (ret == e4gcl::Success) {
        uint32_t samplingRatesNum = availableSamplingRates.size();
        QLabel * samplingRateName = new QLabel("Set sampling rate");
        samplingRateEdit = new QComboBox();
        samplingRateEdit->addItem("None");
        for (unsigned int idx = 0; idx < samplingRatesNum; idx++) {
            samplingRateEdit->addItem(QString::fromStdString(availableSamplingRates[idx].label()));
        }
        protocolWideCtrlsGl->addWidget(samplingRateName, PTE_SAMPLING_RATE_ROW, 0, Qt::AlignRight);
        protocolWideCtrlsGl->addWidget(samplingRateEdit, PTE_SAMPLING_RATE_ROW, 1);

        if (samplingRatesNum < 2) {
            samplingRateName->setVisible(false);
            samplingRateEdit->setVisible(false);
        }
    } /*! \todo FCON gestire l'errore */

    libraryVl->addLayout(protocolWideCtrlsGl);

    /*! OK/Canc buttons */
    QHBoxLayout * okCancHl = new QHBoxLayout;
    mainVl->addLayout(okCancHl);

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    okCancHl->addWidget(spacer);

    QPushButton * cancelBtn = new QPushButton("Cancel");
    okCancHl->addWidget(cancelBtn);
    connect(cancelBtn, &QPushButton::clicked, this, &ProtocolEditor::reject);

    QPushButton * doneBtn = new QPushButton("Ok");
    okCancHl->addWidget(doneBtn);
    connect(doneBtn, &QPushButton::clicked, this, &ProtocolEditor::onUpdateProtocol);
    connect(doneBtn, &QPushButton::clicked, this, &ProtocolEditor::accept);

    this->setGeometry(100, 100, 900, 700);
}

ProtocolEditor::ProtocolEditor() {

}

ProtocolEditor::~ProtocolEditor() {
    if (libraryPidl != nullptr) {
        delete libraryPidl;
        libraryPidl = nullptr;
    }

    if (phasesPidl != nullptr) {
        delete phasesPidl;
        phasesPidl = nullptr;
    }

    if (ctrlPidl != nullptr) {
        delete ctrlPidl;
        ctrlPidl = nullptr;
    }

    if (analysisPidl != nullptr) {
        delete analysisPidl;
        analysisPidl = nullptr;
    }

    if (protocolItemCtrlManager != nullptr) {
        delete protocolItemCtrlManager;
        protocolItemCtrlManager = nullptr;
    }

    if (protocolPreview != nullptr) {
        delete protocolPreview;
        protocolPreview = nullptr;
    }
}

bool ProtocolEditor::importEpml(EpmlManager * epmlManager, EpmlStatus_t &epmlStatus) {
    QString tag = "";
    int depth = EPML_PROTOCOL_PARAM_DEPTH;
    QString parentTag = "protocol";

    double hold = 0.0;
    bool holdRef = false;
    int sweepsNum = 1;
    QString currentRangeStr = "None";
    QString voltageRangeStr = "None";
    QString samplingRateStr = "None";

    while (tag != "controls") {
        if (!epmlManager->getNextAny(tag, depth, parentTag, epmlStatus)) {
            epmlStatus = EpmlSyntaxError;
            return false;
        }

        if (tag == stimulusAbbrName.toLower() + "hold") {
            hold = epmlManager->getDouble(tag, depth, epmlStatus);
            if (epmlStatus != EpmlValueFound) {
                epmlStatus = EpmlSyntaxError;
                return false;
            }

        } else if (tag == stimulusAbbrName.toLower() + "holdref") {
            holdRef = epmlManager->getBool(tag, depth, epmlStatus);
            if (epmlStatus != EpmlValueFound) {
                epmlStatus = EpmlSyntaxError;
                return false;
            }

        } else if (tag == "sweeps") {
            sweepsNum = epmlManager->getInt(tag, depth, epmlStatus);
            if (epmlStatus != EpmlValueFound) {
                epmlStatus = EpmlSyntaxError;
                return false;
            }

        } else if (tag == "currentrange") {
            currentRangeStr = epmlManager->getString(tag, depth, epmlStatus);
            if (epmlStatus != EpmlValueFound) {
                epmlStatus = EpmlSyntaxError;
                return false;
            }

        } else if (tag == "voltagerange") {
            voltageRangeStr = epmlManager->getString(tag, depth, epmlStatus);
            if (epmlStatus != EpmlValueFound) {
                epmlStatus = EpmlSyntaxError;
                return false;
            }

        } else if (tag == "samplingrate") {
            samplingRateStr = epmlManager->getString(tag, depth, epmlStatus);
            if (epmlStatus != EpmlValueFound) {
                epmlStatus = EpmlSyntaxError;
                return false;
            }
        }
    }

    holdRefEdit->setChecked(holdRef);
    sweepsNumEdit->setValue(sweepsNum);
    /*! Accept also similar values by checking all characters except for the first one, so 200pA can be matched with 300pA */
    int currentRangeIdx = currentRangeEdit->findText("[1-9]" + currentRangeStr.right(currentRangeStr.size()-1), Qt::MatchRegExp);
    if (currentRangeIdx >= 0) {
        currentRangeEdit->setCurrentIndex(currentRangeIdx);

    } else {
        currentRangeIdx = 0;
    }
    int voltageRangeIdx = voltageRangeEdit->findText("[1-9]" + voltageRangeStr.right(voltageRangeStr.size()-1), Qt::MatchRegExp);
    if (voltageRangeIdx >= 0) {
        voltageRangeEdit->setCurrentIndex(voltageRangeIdx);

    } else {
        voltageRangeIdx = 0;
    }
    samplingRateEdit->setCurrentText(samplingRateStr);
    this->setHoldingRange();
    holdEdit->setValue(hold);

    /*! tag = "controls"; this is the exit condition from the while loop.
     *  from now on tags controls, phases, cursors and analysis are mandatory in this order */
    if (!ctrlPidl->importEpml(epmlManager, tag, epmlStatus, voltageRangeIdx, currentRangeIdx)) {
        /*! \todo FCON gestire messaggi di errore */
        return false;
    }

    tag = "phases";

    if (!epmlManager->getNext(tag, depth, parentTag, epmlStatus)) {
        epmlStatus = EpmlSyntaxError;
        return false;
    }

    if (!phasesPidl->importEpml(epmlManager, tag, epmlStatus, voltageRangeIdx, currentRangeIdx)) {
        /*! \todo FCON gestire messaggi di errore */
        return false;
    }

    if (phasesPidl->getDropItems()->size() > 0) {
        this->onUpdateProtocol();
    }

    protocolPreview->updateView();

    depth = EPML_CURSOR_LIST_DEPTH;

    tag = "cursors";

    if (!epmlManager->getNext(tag, depth, parentTag, epmlStatus)) {
        epmlStatus = EpmlSyntaxError;
        return false;
    }

    if (!protocolPreview->importEpml(epmlManager, tag, epmlStatus)) {
        /*! \todo FCON gestire messaggi di errore */
        return false;
    }

    depth = EPML_ANALYSIS_LIST_DEPTH;

    tag = "analysis";

    if (!epmlManager->getNext(tag, depth, parentTag, epmlStatus)) {
        epmlStatus = EpmlSyntaxError;
        return false;
    }

    if (!analysisPidl->importEpml(epmlManager, tag, epmlStatus, voltageRangeIdx, currentRangeIdx)) {
        /*! \todo FCON gestire messaggi di errore */
        return false;
    }

    return true;
}

bool ProtocolEditor::exportEpml(EpmlManager * epmlManager, EpmlStatus_t &epmlStatus) {
    int depth = EPML_PROTOCOL_PARAM_DEPTH;
    QString parentTag = "protocol";

    QString tag = stimulusAbbrName.toLower() + "hold";
    epmlManager->addDoubleValue(tag, depth, holdEdit->value());

    tag = stimulusAbbrName.toLower() + "holdref";
    epmlManager->addBoolValue(tag, depth, holdRefEdit->isChecked());

    tag = "sweeps";
    epmlManager->addIntValue(tag, depth, sweepsNumEdit->value());

    tag = "currentrange";
    epmlManager->addStringValue(tag, depth, currentRangeEdit->currentText());

    tag = "voltagerange";
    epmlManager->addStringValue(tag, depth, voltageRangeEdit->currentText());

    tag = "samplingrate";
    epmlManager->addStringValue(tag, depth, samplingRateEdit->currentText());

    depth = EPML_PROTOCOL_ITEM_LIST_DEPTH;

    tag = "controls";
    if (!epmlManager->createSection(tag, depth, parentTag, epmlStatus)) {
        return false;

    } else {
        ctrlPidl->exportEpml(epmlManager, tag, epmlStatus);
    }

    tag = "phases";
    if (!epmlManager->createSection(tag, depth, parentTag, epmlStatus)) {
        return false;

    } else {
        phasesPidl->exportEpml(epmlManager, tag, epmlStatus);
    }

    depth = EPML_CURSOR_LIST_DEPTH;

    tag = "cursors";
    if (!epmlManager->createSection(tag, depth, parentTag, epmlStatus)) {
        return false;

    } else {
        protocolPreview->exportEpml(epmlManager, tag, epmlStatus);
    }

    depth = EPML_ANALYSIS_LIST_DEPTH;

    tag = "analysis";
    if (!epmlManager->createSection(tag, depth, parentTag, epmlStatus)) {
        return false;

    } else {
        analysisPidl->exportEpml(epmlManager, tag, epmlStatus);
    }

    return true;
}

void ProtocolEditor::onUpdateProtocol() {
    protocolPreview->updateView();
    QString propertyDialogProtocol;
    if (parentWidget->isPropertyDialogOpened(propertyDialogProtocol) && (propertyDialogProtocol == parentWidget->getName())) {
        parentWidget->getProtocolPreview()->setProtocol(parentWidget);
        parentWidget->getProtocolPreview()->updateView();
    }
}

QVector <ProtocolDropItem *> * ProtocolEditor::getDropItems() {
    return phasesPidl->getDropItems();
}

void ProtocolEditor::setTooManyTriggersWarning(bool flag) {
    protocolPreview->setTooManyTriggersWarning(flag);
}

void ProtocolEditor::onUpdateCtrlItem() {
    this->onUpdateProtocol();
}

void ProtocolEditor::onUpdateHold(double value) {
    phasesPidl->onUpdateHold(value);
    this->onUpdateProtocol();
}

void ProtocolEditor::onUpdateHoldRef() {
    this->onUpdateProtocol();
}

void ProtocolEditor::onStimulusRangeSelected(int rangeIdx) {
    this->stimulusRangeSelected(rangeIdx);
    this->onUpdateProtocol();
}

QVector <ProtocolDropControlItem *> * ProtocolEditor::getCtrlItems() {
    return ctrlPidl->getCtrlItems();
}

bool ProtocolEditor::analysisRequested(ProtocolConsumerType_t consumerType) {
    return analysisPidl->analysisRequested(consumerType);
}

bool ProtocolEditor::analysisValid(ProtocolConsumerType_t consumerType) {
    return analysisPidl->analysisValid(consumerType);
}

bool ProtocolEditor::allAnalysisValid() {
    for (int analysisIdx = PTD_PROTOCOL_ANALYSIS_OFFSET; analysisIdx < ProtocolConsumerTypesNum; analysisIdx++) {
        if (analysisPidl->analysisValid((ProtocolConsumerType_t)analysisIdx) != analysisPidl->analysisRequested((ProtocolConsumerType_t)analysisIdx)) {
            return false;
        }
    }
    return true;
}

QVector <int> ProtocolEditor::getAnalysisCursorsMapping(ProtocolConsumerType_t consumerType) {
    return analysisPidl->getAnalysisCursorsMapping(consumerType);
}

void ProtocolEditor::setName(QString value) {
    name = value;
    this->setWindowTitle(name + " editor");
}

void ProtocolEditor::setHold(double value) {
    holdEdit->setValue(value);
}

void ProtocolEditor::setSweepsNum(int value) {
    sweepsNumEdit->setValue(value);
}

QString ProtocolEditor::getName() {
    return name;
}

double ProtocolEditor::getHold() {
    return holdEdit->value();
}

SteppedSpinBox * ProtocolEditor::getHoldEdit() {
    return holdEdit;
}

void ProtocolEditor::setHoldingDelta(e4gcl::Measurement_t &holdingDelta) {
    protocolPreview->setHoldingDelta(holdingDelta);
    this->onUpdateProtocol();
}

bool ProtocolEditor::getHoldRef() {
    return holdRefEdit->isChecked();
}

QCheckBox * ProtocolEditor::getHoldRefEdit() {
    return holdRefEdit;
}

int ProtocolEditor::getSweepsNum() {
    return sweepsNumEdit->value();
}

QSpinBox * ProtocolEditor::getSweepsNumEdit() {
    return sweepsNumEdit;
}

int ProtocolEditor::getSamplingRateIndex() {
    return samplingRateEdit->currentIndex()-1; /*!< -1 is to remove the offset due to the "none" option */
}

QComboBox * ProtocolEditor::getSamplingRateEdit() {
    return samplingRateEdit;
}

VoltageProtocolEditor::VoltageProtocolEditor() {
    stimulusAbbrName = "V";

    /*! Library */
    QToolButton * btn;
    QVector <int> itemIdxs;
    itemIdxs.clear();
    int itemIdx = 0;

    libraryPidl->addItem(new ProtocolDragSeparator());
    btn = libraryPidl->setSeparator("Protocol items", PROT_EDITOR_STIMULUS_SEPARATOR_COLOR);
    itemIdx++;

    if (commLib->hasProtocolStep() == e4gcl::Success) {
        libraryPidl->addItem(new ProtocolDragVHoldItem());
        itemIdxs.append(itemIdx++);
        libraryPidl->addItem(new ProtocolDragVConstItem());
        itemIdxs.append(itemIdx++);
#if (GLB_ENABLE_XSTEP_PROTOCOL_ITEM == true)
        libraryPidl->addItem(new ProtocolDragVStepItem());
        itemIdxs.append(itemIdx++);
#endif
#if (GLB_ENABLE_XTSTEP_PROTOCOL_ITEM == true)
        libraryPidl->addItem(new ProtocolDragVTStepItem());
        itemIdxs.append(itemIdx++);
#endif
        libraryPidl->addItem(new ProtocolDragVStepTStepItem());
        itemIdxs.append(itemIdx++);
    }

    if (commLib->hasProtocolRamp() == e4gcl::Success) {
        libraryPidl->addItem(new ProtocolDragVRampItem());
        itemIdxs.append(itemIdx++);
    }

    if (commLib->hasProtocolSin() == e4gcl::Success) {
        libraryPidl->addItem(new ProtocolDragVSinItem());
        itemIdxs.append(itemIdx++);
    }

    if (commLib->hasProtocolStep() == e4gcl::Success) {
        libraryPidl->addItem(new ProtocolDragVRestItem());
        itemIdxs.append(itemIdx++);
    }

    libraryPidl->setSeparatorItems(btn, itemIdxs);
    itemIdxs.clear();

    libraryPidl->addItem(new ProtocolDragSeparator());
    btn = libraryPidl->setSeparator("Loop items", PROT_EDITOR_LOOPS_SEPARATOR_COLOR);
    itemIdx++;

    libraryPidl->addItem(new ProtocolDragVRepSeqItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragVRepSeqWithStepsItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragVInfRepSeqItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragVRepSeqScaledItem());
    itemIdxs.append(itemIdx++);

    libraryPidl->setSeparatorItems(btn, itemIdxs);
    itemIdxs.clear();

    libraryPidl->addItem(new ProtocolDragSeparator());
    btn = libraryPidl->setSeparator("Control items", PROT_EDITOR_CONTROLS_SEPARATOR_COLOR);
    itemIdx++;

    libraryPidl->addItem(new ProtocolDragVoltageControlItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragTimeControlItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragFrequencyControlItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragNaturalNumControlItem());
    itemIdxs.append(itemIdx++);

    libraryPidl->setSeparatorItems(btn, itemIdxs);
    itemIdxs.clear();

    libraryPidl->addItem(new ProtocolDragSeparator());
    btn = libraryPidl->setSeparator("Analysis", PROT_EDITOR_ANALYSIS_SEPARATOR_COLOR);
    itemIdx++;

    libraryPidl->addItem(new ProtocolDragNoiseReportItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragHistogramItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragSpectrumItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragMembraneTestItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragIvGraphItem());
    itemIdxs.append(itemIdx++);

    libraryPidl->setSeparatorItems(btn, itemIdxs);

    /*! Protocol wide controls */
    double hold = 0.0;
    QLabel * holdName = new QLabel("V-Hold");
    holdEdit = new SteppedSpinBox();

    protocolWideCtrlsGl->addWidget(holdName, PTE_HOLD_ROW, 0, Qt::AlignRight);
    protocolWideCtrlsGl->addWidget(holdEdit, PTE_HOLD_ROW, 1);

    bool holdRef = false;
    QLabel * holdRefName = new QLabel("V-Hold referenced");
    holdRefEdit = new QCheckBox("");
    holdRefEdit->setChecked(holdRef);

    protocolWideCtrlsGl->addWidget(holdRefName, PTE_HOLDREF_ROW, 0, Qt::AlignRight);
    protocolWideCtrlsGl->addWidget(holdRefEdit, PTE_HOLDREF_ROW, 1);

    e4gcl::ErrorCodes_t ret = e4gcl::Success;

    vector <e4gcl::RangedMeasurement_t> availableCurrentRanges;
    ret = commLib->getVCCurrentRanges(availableCurrentRanges);

    if (ret == e4gcl::Success) {
        uint32_t currentRangesNum = availableCurrentRanges.size();
        QLabel * currentRangeName = new QLabel("Set current range");
        currentRangeEdit = new QComboBox();
        currentRangeEdit->addItem("None");
        for (unsigned int idx = 0; idx < currentRangesNum; idx++) {
            currentRangeEdit->addItem(QString::fromStdString(availableCurrentRanges[idx].label()));
        }
        protocolWideCtrlsGl->addWidget(currentRangeName, PTE_CURRENT_RANGE_ROW, 0, Qt::AlignRight);
        protocolWideCtrlsGl->addWidget(currentRangeEdit, PTE_CURRENT_RANGE_ROW, 1);

        if (currentRangesNum < 2) {
            currentRangeName->setVisible(false);
            currentRangeEdit->setVisible(false);
        }
    } /*! \todo FCON gestire l'errore */

    vector <e4gcl::RangedMeasurement_t> availableVoltageRanges;
    ret = commLib->getVCVoltageRanges(availableVoltageRanges);

    if (ret == e4gcl::Success) {
        uint32_t voltageRangesNum = availableVoltageRanges.size();
        QLabel * voltageRangeName = new QLabel("Set voltage range");
        voltageRangeEdit = new QComboBox();
        for (unsigned int idx = 0; idx < voltageRangesNum; idx++) {
            voltageRangeEdit->addItem(QString::fromStdString(availableVoltageRanges[idx].label()));
        }
        protocolWideCtrlsGl->addWidget(voltageRangeName, PTE_VOLTAGE_RANGE_ROW, 0, Qt::AlignRight);
        protocolWideCtrlsGl->addWidget(voltageRangeEdit, PTE_VOLTAGE_RANGE_ROW, 1);

        if (voltageRangesNum < 2) {
            voltageRangeName->setVisible(false);
            voltageRangeEdit->setVisible(false);

        } else {
            connect(voltageRangeEdit, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, &ProtocolEditor::onStimulusRangeSelected);
        }
    } /*! \todo FCON gestire l'errore */

    /*! setHoldingRange inherits the range from the current range, so it has to be defined after the creation of its control */
    this->setHoldingRange();
    holdEdit->setValue(hold);

    /*! Control items */
    QVBoxLayout * ctrlItemsVl = new QVBoxLayout;
    editorHl->addLayout(ctrlItemsVl);

    QLabel * ctrlTitle = new QLabel("Control items");
    ctrlTitle->setFont(titlesFont);
    ctrlItemsVl->addWidget(ctrlTitle);

    ctrlPidl = new CtrlProtocolItemDropList(commLib, holdEdit, E4GCL_VOLTAGE_CLAMP_MODE);
    ctrlItemsVl->addWidget(ctrlPidl);

    protocolItemCtrlManager = new ProtocolItemCtrlManager(ctrlPidl);

    ctrlPidl->setCtrlManager(protocolItemCtrlManager);

    /*! Analysis items */
    QVBoxLayout * analysisItemsVl = new QVBoxLayout;
    editorHl->addLayout(analysisItemsVl);

    QLabel * analysisTitle = new QLabel("Analysis");
    analysisTitle->setFont(titlesFont);
    analysisItemsVl->addWidget(analysisTitle);

    analysisPidl = new AnalysisProtocolItemDropList(commLib, holdEdit, E4GCL_VOLTAGE_CLAMP_MODE);
    analysisItemsVl->addWidget(analysisPidl);

    connect(analysisPidl, &AnalysisProtocolItemDropList::analysisChanged, parentWidget, &ProtocolWidget::onCheckAnalysisValid);

    connect(holdEdit, QOverload <double> ::of(&SteppedSpinBox::valueChanged), this, &ProtocolEditor::onUpdateHold);
    connect(holdRefEdit, &QCheckBox::stateChanged, this, &ProtocolEditor::onUpdateHoldRef);
    connect(sweepsNumEdit, QOverload <int> ::of(&QSpinBox::valueChanged), this, &ProtocolEditor::onUpdateProtocol);

    connect(ctrlPidl, &CtrlProtocolItemDropList::updateProtocol, this, &ProtocolEditor::onUpdateCtrlItem);

    /*! Protocol previewer */
    e4gcl::RangedMeasurement_t stimulusRange;
    commLib->getVoltageProtocolRange(0, stimulusRange);
    holdEdit->setRangedMeasurement(stimulusRange, SteppedSpinBox::MinMaxRange);
    e4gcl::RangedMeasurement_t timeRange;
    commLib->getTimeProtocolRange(timeRange);
    timeRange.convertValues(e4gcl::UnitPfxMilli);
    protocolPreview = new ProtocolPreview(commLib, timeRange, stimulusRange, "Protocol Preview");
    protocolPreview->setProtocol(parentWidget);
    protocolPreview->setAnalysisPidl(static_cast <AnalysisProtocolItemDropList *> (analysisPidl));

    mainVl->addWidget(protocolPreview);

    mainVl->setStretch(0, 2);
    mainVl->setStretch(1, 1);
}

int VoltageProtocolEditor::getCurrentRangeIndex() {
    return currentRangeEdit->currentIndex()-1; /*!< -1 is to remove the offset due to the "none" option */
}

QComboBox * VoltageProtocolEditor::getCurrentRangeEdit() {
    return currentRangeEdit;
}

int VoltageProtocolEditor::getVoltageRangeIndex() {
    return voltageRangeEdit->currentIndex(); /*!< there is no "none" option in voltage clamp */
}

QComboBox * VoltageProtocolEditor::getVoltageRangeEdit() {
    return voltageRangeEdit;
}

void VoltageProtocolEditor::setHoldingRange() {
    e4gcl::RangedMeasurement_t stimulusRange;
    commLib->getVoltageProtocolRange((unsigned int)(voltageRangeEdit->currentIndex()), stimulusRange);
    holdEdit->setRangedMeasurement(stimulusRange, SteppedSpinBox::MinMaxRange);
}

void VoltageProtocolEditor::stimulusRangeSelected(int rangeIdx) {
    e4gcl::RangedMeasurement_t stimulusRange;
    commLib->getVoltageProtocolRange((unsigned int)rangeIdx, stimulusRange);
    holdEdit->setRangedMeasurement(stimulusRange, SteppedSpinBox::MinMaxRange);

    protocolPreview->setStimulusRange(stimulusRange);
    parentWidget->getProtocolPreview()->setStimulusRange(stimulusRange);
    phasesPidl->setStimulusRange(stimulusRange);
    ctrlPidl->setStimulusRange(stimulusRange);
}

CurrentProtocolEditor::CurrentProtocolEditor() {
    stimulusAbbrName = "I";

    /*! Library */
    QToolButton * btn;
    QVector <int> itemIdxs;
    itemIdxs.clear();
    int itemIdx = 0;

    libraryPidl->addItem(new ProtocolDragSeparator());
    btn = libraryPidl->setSeparator("Protocol items", PROT_EDITOR_STIMULUS_SEPARATOR_COLOR);
    itemIdx++;

    if (commLib->hasProtocolStep() == e4gcl::Success) {
        libraryPidl->addItem(new ProtocolDragIHoldItem());
        itemIdxs.append(itemIdx++);
        libraryPidl->addItem(new ProtocolDragIConstItem());
        itemIdxs.append(itemIdx++);
#if (GLB_ENABLE_XSTEP_PROTOCOL_ITEM == true)
        libraryPidl->addItem(new ProtocolDragIStepItem());
        itemIdxs.append(itemIdx++);
#endif
#if (GLB_ENABLE_XTSTEP_PROTOCOL_ITEM == true)
        libraryPidl->addItem(new ProtocolDragITStepItem());
        itemIdxs.append(itemIdx++);
#endif
        libraryPidl->addItem(new ProtocolDragIStepTStepItem());
        itemIdxs.append(itemIdx++);
    }

    if (commLib->hasProtocolRamp() == e4gcl::Success) {
        libraryPidl->addItem(new ProtocolDragIRampItem());
        itemIdxs.append(itemIdx++);
    }

    if (commLib->hasProtocolSin() == e4gcl::Success) {
        libraryPidl->addItem(new ProtocolDragISinItem());
        itemIdxs.append(itemIdx++);
    }

    if (commLib->hasProtocolStep() == e4gcl::Success) {
        libraryPidl->addItem(new ProtocolDragIRestItem());
        itemIdxs.append(itemIdx++);
    }

    libraryPidl->setSeparatorItems(btn, itemIdxs);
    itemIdxs.clear();

    libraryPidl->addItem(new ProtocolDragSeparator());
    btn = libraryPidl->setSeparator("Loop items", PROT_EDITOR_LOOPS_SEPARATOR_COLOR);
    itemIdx++;

    libraryPidl->addItem(new ProtocolDragIRepSeqItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragIRepSeqWithStepsItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragIInfRepSeqItem());
    itemIdxs.append(itemIdx++);

    libraryPidl->setSeparatorItems(btn, itemIdxs);
    itemIdxs.clear();

    libraryPidl->addItem(new ProtocolDragSeparator());
    btn = libraryPidl->setSeparator("Control items", PROT_EDITOR_CONTROLS_SEPARATOR_COLOR);
    itemIdx++;

    libraryPidl->addItem(new ProtocolDragCurrentControlItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragTimeControlItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragFrequencyControlItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragNaturalNumControlItem());
    itemIdxs.append(itemIdx++);

    libraryPidl->setSeparatorItems(btn, itemIdxs);
    itemIdxs.clear();

    libraryPidl->addItem(new ProtocolDragSeparator());
    btn = libraryPidl->setSeparator("Analysis", PROT_EDITOR_ANALYSIS_SEPARATOR_COLOR);
    itemIdx++;

    libraryPidl->addItem(new ProtocolDragNoiseReportItem());
    itemIdxs.append(itemIdx++);
    //  libraryPidl->addItem(new ProtocolDragHistogramItem());
    //  itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragSpectrumItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragResistanceEstimationItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragIvGraphItem());
    itemIdxs.append(itemIdx++);
#ifndef GLB_DISABLE_VOLTAGE_TRACKING
    libraryPidl->addItem(new ProtocolDragVoltageTrackingItem());
    itemIdxs.append(itemIdx++);
#endif
    libraryPidl->addItem(new ProtocolDragApThresholdItem());
    itemIdxs.append(itemIdx++);
    libraryPidl->addItem(new ProtocolDragApStatisticsItem());
    itemIdxs.append(itemIdx++);

    libraryPidl->setSeparatorItems(btn, itemIdxs);

    /*! Protocol wide controls */
    double hold = 0.0;
    QLabel * holdName = new QLabel("I-Hold");
    holdEdit = new SteppedSpinBox();

    protocolWideCtrlsGl->addWidget(holdName, PTE_HOLD_ROW, 0, Qt::AlignRight);
    protocolWideCtrlsGl->addWidget(holdEdit, PTE_HOLD_ROW, 1);

    bool holdRef = false;
    QLabel * holdRefName = new QLabel("I-Hold referenced");
    holdRefEdit = new QCheckBox("");
    holdRefEdit->setChecked(holdRef);

    protocolWideCtrlsGl->addWidget(holdRefName, PTE_HOLDREF_ROW, 0, Qt::AlignRight);
    protocolWideCtrlsGl->addWidget(holdRefEdit, PTE_HOLDREF_ROW, 1);

    e4gcl::ErrorCodes_t ret = e4gcl::Success;

    vector <e4gcl::RangedMeasurement_t> availableCurrentRanges;
    ret = commLib->getCCCurrentRanges(availableCurrentRanges);

    if (ret == e4gcl::Success) {
        uint32_t currentRangesNum = availableCurrentRanges.size();
        QLabel * currentRangeName = new QLabel("Set current range");
        currentRangeEdit = new QComboBox();
        for (unsigned int idx = 0; idx < currentRangesNum; idx++) {
            currentRangeEdit->addItem(QString::fromStdString(availableCurrentRanges[idx].label()));
        }
        protocolWideCtrlsGl->addWidget(currentRangeName, PTE_CURRENT_RANGE_ROW, 0, Qt::AlignRight);
        protocolWideCtrlsGl->addWidget(currentRangeEdit, PTE_CURRENT_RANGE_ROW, 1);

        if (currentRangesNum < 2) {
            currentRangeName->setVisible(false);
            currentRangeEdit->setVisible(false);

        } else {
            connect(currentRangeEdit, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, &ProtocolEditor::onStimulusRangeSelected);
        }
    } /*! \todo FCON gestire l'errore */

    vector <e4gcl::RangedMeasurement_t> availableVoltageRanges;
    ret = commLib->getCCVoltageRanges(availableVoltageRanges);

    if (ret == e4gcl::Success) {
        uint32_t voltageRangesNum = availableVoltageRanges.size();
        QLabel * voltageRangeName = new QLabel("Set voltage range");
        voltageRangeEdit = new QComboBox();
        voltageRangeEdit->addItem("None");
        for (unsigned int idx = 0; idx < voltageRangesNum; idx++) {
            voltageRangeEdit->addItem(QString::fromStdString(availableVoltageRanges[idx].label()));
        }
        protocolWideCtrlsGl->addWidget(voltageRangeName, PTE_VOLTAGE_RANGE_ROW, 0, Qt::AlignRight);
        protocolWideCtrlsGl->addWidget(voltageRangeEdit, PTE_VOLTAGE_RANGE_ROW, 1);

        if (voltageRangesNum < 2) {
            voltageRangeName->setVisible(false);
            voltageRangeEdit->setVisible(false);
        }
    } /*! \todo FCON gestire l'errore */

    /*! setHoldingRange inherits the range from the current range, so it has to be defined after the creation of its control */
    this->setHoldingRange();
    holdEdit->setValue(hold);

    /*! Control items */
    QVBoxLayout * ctrlItemsVl = new QVBoxLayout;
    editorHl->addLayout(ctrlItemsVl);

    QLabel * ctrlTitle = new QLabel("Control items");
    ctrlTitle->setFont(titlesFont);
    ctrlItemsVl->addWidget(ctrlTitle);

    ctrlPidl = new CtrlProtocolItemDropList(commLib, holdEdit, E4GCL_CURRENT_CLAMP_MODE);
    ctrlItemsVl->addWidget(ctrlPidl);

    protocolItemCtrlManager = new ProtocolItemCtrlManager(ctrlPidl);

    ctrlPidl->setCtrlManager(protocolItemCtrlManager);

    /*! Analysis items */
    QVBoxLayout * analysisItemsVl = new QVBoxLayout;
    editorHl->addLayout(analysisItemsVl);

    QLabel * analysisTitle = new QLabel("Analysis");
    analysisTitle->setFont(titlesFont);
    analysisItemsVl->addWidget(analysisTitle);

    analysisPidl = new AnalysisProtocolItemDropList(commLib, holdEdit, E4GCL_CURRENT_CLAMP_MODE);
    analysisItemsVl->addWidget(analysisPidl);

    connect(analysisPidl, &AnalysisProtocolItemDropList::analysisChanged, parentWidget, &ProtocolWidget::onCheckAnalysisValid);

    connect(holdEdit, QOverload <double> ::of(&SteppedSpinBox::valueChanged), this, &ProtocolEditor::onUpdateHold);
    connect(holdRefEdit, &QCheckBox::stateChanged, this, &ProtocolEditor::onUpdateHoldRef);
    connect(sweepsNumEdit, QOverload <int> ::of(&QSpinBox::valueChanged), this, &ProtocolEditor::onUpdateProtocol);

    connect(ctrlPidl, &CtrlProtocolItemDropList::updateProtocol, this, &ProtocolEditor::onUpdateCtrlItem);

    /*! Protocol previewer */
    e4gcl::RangedMeasurement_t stimulusRange;
    commLib->getCurrentProtocolRange(0, stimulusRange);
    holdEdit->setRangedMeasurement(stimulusRange, SteppedSpinBox::MinMaxRange);
    e4gcl::RangedMeasurement_t timeRange;
    commLib->getTimeProtocolRange(timeRange);
    timeRange.convertValues(e4gcl::UnitPfxMilli);
    protocolPreview = new ProtocolPreview(commLib, timeRange, stimulusRange, "Protocol Preview");
    protocolPreview->setProtocol(parentWidget);
    protocolPreview->setAnalysisPidl(static_cast <AnalysisProtocolItemDropList *> (analysisPidl));

    mainVl->addWidget(protocolPreview);

    mainVl->setStretch(0, 2);
    mainVl->setStretch(1, 1);
}

int CurrentProtocolEditor::getCurrentRangeIndex() {
    return currentRangeEdit->currentIndex(); /*!< there is no "none" option in current clamp */
}

QComboBox * CurrentProtocolEditor::getCurrentRangeEdit() {
    return currentRangeEdit;
}

int CurrentProtocolEditor::getVoltageRangeIndex() {
    return voltageRangeEdit->currentIndex()-1; /*!< -1 is to remove the offset due to the "none" option */
}

QComboBox * CurrentProtocolEditor::getVoltageRangeEdit() {
    return voltageRangeEdit;
}

void CurrentProtocolEditor::setHoldingRange() {
    e4gcl::RangedMeasurement_t stimulusRange;
    commLib->getCurrentProtocolRange((unsigned int)(currentRangeEdit->currentIndex()), stimulusRange);
    holdEdit->setRangedMeasurement(stimulusRange, SteppedSpinBox::MinMaxRange);
}

void CurrentProtocolEditor::stimulusRangeSelected(int rangeIdx) {
    e4gcl::RangedMeasurement_t stimulusRange;
    commLib->getCurrentProtocolRange((unsigned int)rangeIdx, stimulusRange);
    holdEdit->setRangedMeasurement(stimulusRange, SteppedSpinBox::MinMaxRange);

    protocolPreview->setStimulusRange(stimulusRange);
    parentWidget->getProtocolPreview()->setStimulusRange(stimulusRange);
    phasesPidl->setStimulusRange(stimulusRange);
    ctrlPidl->setStimulusRange(stimulusRange);
}

GapfreeProtocolEditor::GapfreeProtocolEditor() {
    sweepsNumName->setVisible(false);
    sweepsNumEdit->setVisible(false);

    /*! Phases */
    phasesVl = new QVBoxLayout;
    editorHl->insertLayout(PTE_PHASES_COLUMN_IDX, phasesVl);

    QLabel * phasesTitle = new QLabel("Gap-free items");
    phasesTitle->setFont(titlesFont);
    phasesVl->addWidget(phasesTitle);
}

bool GapfreeProtocolEditor::exportEpml(EpmlManager * epmlManager, EpmlStatus_t &epmlStatus) {
    int depth = EPML_PROTOCOL_PARAM_DEPTH;
    QString parentTag = "protocol";

    QString tag = "type";
    epmlManager->addStringValue(tag, depth, "Gap Free");

    return ProtocolEditor::exportEpml(epmlManager, epmlStatus);
}

EpisodicProtocolEditor::EpisodicProtocolEditor() {
    /*! Phases */
    phasesVl = new QVBoxLayout;
    editorHl->insertLayout(PTE_PHASES_COLUMN_IDX, phasesVl);

    QLabel * phasesTitle = new QLabel("Sweeps items");
    phasesTitle->setFont(titlesFont);
    phasesVl->addWidget(phasesTitle);
}

bool EpisodicProtocolEditor::exportEpml(EpmlManager * epmlManager, EpmlStatus_t &epmlStatus) {
    int depth = EPML_PROTOCOL_PARAM_DEPTH;
    QString parentTag = "protocol";

    QString tag = "type";
    epmlManager->addStringValue(tag, depth, "Episodic");

    return ProtocolEditor::exportEpml(epmlManager, epmlStatus);
}

GapfreeVoltageProtocolEditor::GapfreeVoltageProtocolEditor(e4gcl::CommLib * commLib, ProtocolWidget * protocolWidget, QString name) :
    ProtocolEditor(commLib, protocolWidget, name),
    VoltageProtocolEditor(),
    GapfreeProtocolEditor() {

    phasesPidl = new GapfreeProtocolItemDropList(commLib, holdEdit, E4GCL_VOLTAGE_CLAMP_MODE);
    phasesVl->addWidget(phasesPidl);

    phasesPidl->setCtrlManager(protocolItemCtrlManager);
    connect(phasesPidl, &ProtocolItemDropList::updateProtocol, this, &ProtocolEditor::onUpdateProtocol);
}

EpisodicVoltageProtocolEditor::EpisodicVoltageProtocolEditor(e4gcl::CommLib * commLib, ProtocolWidget * protocolWidget, QString name) :
    ProtocolEditor(commLib, protocolWidget, name),
    VoltageProtocolEditor(),
    EpisodicProtocolEditor() {

    phasesPidl = new EpisodicProtocolItemDropList(commLib, holdEdit, E4GCL_VOLTAGE_CLAMP_MODE);
    phasesVl->addWidget(phasesPidl);

    phasesPidl->setCtrlManager(protocolItemCtrlManager);
    connect(phasesPidl, &ProtocolItemDropList::updateProtocol, this, &ProtocolEditor::onUpdateProtocol);
}

GapfreeCurrentProtocolEditor::GapfreeCurrentProtocolEditor(e4gcl::CommLib * commLib, ProtocolWidget * protocolWidget, QString name) :
    ProtocolEditor(commLib, protocolWidget, name),
    CurrentProtocolEditor(),
    GapfreeProtocolEditor() {

    phasesPidl = new GapfreeProtocolItemDropList(commLib, holdEdit, E4GCL_CURRENT_CLAMP_MODE);
    phasesVl->addWidget(phasesPidl);

    phasesPidl->setCtrlManager(protocolItemCtrlManager);
    connect(phasesPidl, &ProtocolItemDropList::updateProtocol, this, &ProtocolEditor::onUpdateProtocol);
}

EpisodicCurrentProtocolEditor::EpisodicCurrentProtocolEditor(e4gcl::CommLib * commLib, ProtocolWidget * protocolWidget, QString name) :
    ProtocolEditor(commLib, protocolWidget, name),
    CurrentProtocolEditor(),
    EpisodicProtocolEditor() {

    phasesPidl = new EpisodicProtocolItemDropList(commLib, holdEdit, E4GCL_CURRENT_CLAMP_MODE);
    phasesVl->addWidget(phasesPidl);

    phasesPidl->setCtrlManager(protocolItemCtrlManager);
    connect(phasesPidl, &ProtocolItemDropList::updateProtocol, this, &ProtocolEditor::onUpdateProtocol);
}
