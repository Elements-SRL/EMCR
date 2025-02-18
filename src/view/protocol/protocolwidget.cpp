#include "protocolwidget.h"

#include <QSplitter>
#include <QMessageBox>

#include "protocolutils.h"

ProtocolWidget::ProtocolWidget(MessageDispatcher * msgDisp, QString name, ProtocolPropertyDialog * dialog, ProtocolType_t type, ClampingModality_t clampingModality) :
    QListWidgetItem(),
    msgDisp(msgDisp),
    name(name),
    dialog(dialog),
    type(type),
    clampingModality(clampingModality) {

    this->setBackground(PROT_WIDGET_VALID_COLOR);

    model = new ProtocolModel;

    if (type == ProtocolTypeGapfree) {
        if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
            protocolEditor = new GapfreeVoltageProtocolEditor(msgDisp, model, this, name);

        } else {
            protocolEditor = new GapfreeCurrentProtocolEditor(msgDisp, model, this, name);
        }

    } else {
        if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
            protocolEditor = new EpisodicVoltageProtocolEditor(msgDisp, model, this, name);

        } else {
            protocolEditor = new EpisodicCurrentProtocolEditor(msgDisp, model, this, name);
        }
    }

    holdEditOrig = protocolEditor->getHoldEdit();
    holdRefEditOrig = protocolEditor->getHoldRefEdit();
    sweepsNumEditOrig = protocolEditor->getSweepsNumEdit();
    currentRangeEditOrig = protocolEditor->getCurrentRangeEdit();
    voltageRangeEditOrig = protocolEditor->getVoltageRangeEdit();
    samplingRateEditOrig = protocolEditor->getSamplingRateEdit();

    ctrlDispatchers = new QVector <ProtocolCtrlDispatcher *>;

    this->updateText();

    /*! Creates a grid that contains the item properties */
    propertyCtrlLo = dialog->getControlsLayout();
    propertyBtnsHl = dialog->getButtonsLayout();

    consumerRequests.resize(ProtocolConsumerAnalysis);
    this->initConsumerRequests();

//    mDev->getMaxOutputTriggers(maxTriggerEvents);
}

ProtocolWidget::ProtocolWidget() {

}

ProtocolWidget::~ProtocolWidget() {
    if (protocolEditor != nullptr) {
        delete protocolEditor;
        protocolEditor = nullptr;
    }

    this->clearPropertyDialog();

    for (int idx = 0; idx < ctrlDispatchers->size(); idx++) {
        delete ctrlDispatchers->at(idx);
    }
}

void ProtocolWidget::initConsumerRequests() {
    for (int consumerIdx = 0; consumerIdx < ProtocolConsumerAnalysis; consumerIdx++) {
        consumerRequests[consumerIdx] = false;
    }
}

bool ProtocolWidget::consumerRequested(ProtocolConsumerType_t consumerType) {
    if (consumerType < ProtocolConsumerAnalysis) {
        return consumerRequests[consumerType];

    } else {
        return (protocolEditor->analysisRequested(consumerType) &&
                protocolEditor->analysisValid(consumerType));
    }
}

void ProtocolWidget::setConsumerRequest(ProtocolConsumerType_t consumerType) {
    consumerRequests[consumerType] = true;
}

void ProtocolWidget::resetConsumerRequests() {
    for (int consumerIdx = 0; consumerIdx < ProtocolConsumerAnalysis; consumerIdx++) {
        consumerRequests[consumerIdx] = false;
    }
}

QVector <int> ProtocolWidget::getAnalysisCursorsMapping(ProtocolConsumerType_t consumerType) {
    return protocolEditor->getAnalysisCursorsMapping(consumerType);
}

QString ProtocolWidget::getName() {
    return name;
}

void ProtocolWidget::setShortCutIdx(int value) {
    shortCutIdx = value;
    this->updateText();
}

int ProtocolWidget::getShortCutIdx() {
    return shortCutIdx;
}

ProtocolPreview * ProtocolWidget::getProtocolPreview() {
    return dialog->getProtocolPreview();
}

void ProtocolWidget::openPropertyDialog(QStringList pn) {
    protocolsNames = pn;
    this->populatePropertyDialog();

    dialog->show();
}

bool ProtocolWidget::isPropertyDialogOpened(QString &protocolName) {
    return dialog->isOpened(protocolName);
}

void ProtocolWidget::populatePropertyDialog() {
    if (!dialogPopulated) {
        /*! Protocol name */
        QLabel * nameLbl = new QLabel("Protocol ID");
        name = protocolEditor->getName();
        nameEdit = new QLineEdit(name);
        nameEdit->setMinimumWidth(50);
        propertyCtrlLo->addWidget(nameLbl, PPD_NAME_ROW, 0, 1, 1, Qt::AlignRight);
        propertyCtrlLo->addWidget(nameEdit, PPD_NAME_ROW, 1, 1, -1);
        connect(nameEdit, &QLineEdit::textChanged, this, &ProtocolWidget::onCheckNameAvailability);
        connect(nameEdit, &QLineEdit::textChanged, this, &ProtocolWidget::onPropertyChanged);

        /*! Hold control */
        QLabel * holdLbl = new QLabel(holdName);
        holdEdit = new QDoubleSpinBox();
        holdEdit->setMinimumWidth(50);
        double hold = holdEditOrig->value();
        copy(holdEditOrig, holdEdit);
        QString unitString;
        unitString = QString::fromStdString(model->getStimulusRange().getFullUnit());

        QLabel * holdUnitLbl = new QLabel(unitString);
        ProtocolDoubleSpinBoxCtrlDispatcher * holdDispatcher = new ProtocolDoubleSpinBoxCtrlDispatcher();
        holdDispatcher->setEdit(holdEditOrig);
        holdDispatcher->setOrigValue(hold);
        ctrlDispatchers->push_back(holdDispatcher);
        connect(holdEdit, QOverload <double> ::of(&QDoubleSpinBox::valueChanged), holdDispatcher, &ProtocolDoubleSpinBoxCtrlDispatcher::onCtrlChanged);
        connect(holdEdit, QOverload <double> ::of(&QDoubleSpinBox::valueChanged), this, &ProtocolWidget::onPropertyChanged);
        propertyCtrlLo->addWidget(holdLbl, PPD_HOLD_ROW, 0, 1, 1, Qt::AlignRight);
        propertyCtrlLo->addWidget(holdEdit, PPD_HOLD_ROW, 1, 1, 1);
        propertyCtrlLo->addWidget(holdUnitLbl, PPD_HOLD_ROW, 2, 1, 1);

        /*! Hold referenced control */
        QLabel * holdRefLbl = new QLabel(holdName + " referenced");
        holdRefEdit = new QCheckBox("");
        holdRefEdit->setMinimumWidth(50);
        bool holdRef = holdRefEditOrig->isChecked();
        holdRefEdit->setChecked(holdRef);
        ProtocolCheckBoxCtrlDispatcher * holdRefDispatcher = new ProtocolCheckBoxCtrlDispatcher();
        holdRefDispatcher->setEdit(holdRefEditOrig);
        holdRefDispatcher->setOrigValue(holdRef);
        ctrlDispatchers->push_back(holdRefDispatcher);
        connect(holdRefEdit, &QPushButton::clicked, holdRefDispatcher, &ProtocolCheckBoxCtrlDispatcher::onCtrlChanged);
        connect(holdRefEdit, &QPushButton::clicked, this, &ProtocolWidget::onPropertyChanged);
        propertyCtrlLo->addWidget(holdRefLbl, PPD_HOLD_REF_ROW, 0, 1, 1, Qt::AlignRight);
        propertyCtrlLo->addWidget(holdRefEdit, PPD_HOLD_REF_ROW, 1, 1, 1);

        /*! Sweeps number control */
        sweepsNumLbl = new QLabel("Sweeps");
        sweepsNumEdit = new QSpinBox();
        sweepsNumEdit->setMinimumWidth(50);
        int sweepsNum = sweepsNumEditOrig->value();
        sweepsNumEdit->setRange(sweepsNumEditOrig->minimum(), sweepsNumEditOrig->maximum());
        sweepsNumEdit->setValue(sweepsNum);
        ProtocolSpinBoxCtrlDispatcher * sweepsNumDispatcher = new ProtocolSpinBoxCtrlDispatcher();
        sweepsNumDispatcher->setEdit(sweepsNumEditOrig);
        sweepsNumDispatcher->setOrigValue(sweepsNum);
        ctrlDispatchers->push_back(sweepsNumDispatcher);
        connect(sweepsNumEdit, QOverload <int> ::of(&QSpinBox::valueChanged), sweepsNumDispatcher, &ProtocolSpinBoxCtrlDispatcher::onCtrlChanged);
        connect(sweepsNumEdit, QOverload <int> ::of(&QSpinBox::valueChanged), this, &ProtocolWidget::onPropertyChanged);
        propertyCtrlLo->addWidget(sweepsNumLbl, PPD_SWEEPS_ROW, 0, 1, 1, Qt::AlignRight);
        propertyCtrlLo->addWidget(sweepsNumEdit, PPD_SWEEPS_ROW, 1, 1, 1);

        sweepsNumLbl->setVisible(type == ProtocolTypeEpisodic);
        sweepsNumEdit->setVisible(type == ProtocolTypeEpisodic);

        /*! Current range control */
        if (currentRangeEditOrig != nullptr) {
            QLabel * currentRangeLbl = new QLabel("Set current range");
            currentRangeEdit = new QComboBox();
            currentRangeEdit->setMinimumWidth(50);
            for (int idx = 0; idx < currentRangeEditOrig->count(); idx++) {
                currentRangeEdit->addItem(currentRangeEditOrig->itemText(idx));
            }
            int currentRangeIdx = currentRangeEditOrig->currentIndex();
            currentRangeEdit->setCurrentIndex(currentRangeIdx);
            ProtocolComboBoxCtrlDispatcher * currentRangeDispatcher = new ProtocolComboBoxCtrlDispatcher();
            currentRangeDispatcher->setEdit(currentRangeEditOrig);
            currentRangeDispatcher->setOrigValue(currentRangeIdx);
            ctrlDispatchers->push_back(currentRangeDispatcher);

            connect(currentRangeEdit, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, [=] (int rangeIdx) {
                currentRangeDispatcher->onCtrlChanged(rangeIdx);
                this->onPropertyChanged();
                if (clampingModality == ClampingModality_t::CURRENT_CLAMP) {
                    protocolEditor->onStimulusRangeSelected(rangeIdx);
                    RangedMeasurement_t stimulusRange;
                    msgDisp->getCurrentProtocolRangeFeature((unsigned int)rangeIdx, stimulusRange);
                    initQdoubleSpinBox(holdEdit, stimulusRange, RangedQDoubleSpinBox_t::MIN_MAX);
//                    holdEdit->setRangedMeasurement(stimulusRange, QDoubleSpinBox::MinMaxRange);
                    holdUnitLbl->setText(QString::fromStdString(stimulusRange.getFullUnit()));
                }
            });

            propertyCtrlLo->addWidget(currentRangeLbl, PPD_CURRENT_RANGE_ROW, 0, 1, 1, Qt::AlignRight);
            propertyCtrlLo->addWidget(currentRangeEdit, PPD_CURRENT_RANGE_ROW, 1, 1, 1);

            if (((clampingModality == ClampingModality_t::VOLTAGE_CLAMP) && (currentRangeEditOrig->count() < 3)) ||
                    ((clampingModality == ClampingModality_t::CURRENT_CLAMP) && (currentRangeEditOrig->count() < 2))) {
                currentRangeLbl->setVisible(false);
                currentRangeEdit->setVisible(false);
            }
        }

        /*! Voltage range control */
        if (voltageRangeEditOrig != nullptr) {
            QLabel * voltageRangeLbl = new QLabel("Set voltage range");
            voltageRangeEdit = new QComboBox();
            voltageRangeEdit->setMinimumWidth(50);
            for (int idx = 0; idx < voltageRangeEditOrig->count(); idx++) {
                voltageRangeEdit->addItem(voltageRangeEditOrig->itemText(idx));
            }
            int voltageRangeIdx = voltageRangeEditOrig->currentIndex();
            voltageRangeEdit->setCurrentIndex(voltageRangeIdx);
            ProtocolComboBoxCtrlDispatcher * voltageRangeDispatcher = new ProtocolComboBoxCtrlDispatcher();
            voltageRangeDispatcher->setEdit(voltageRangeEditOrig);
            voltageRangeDispatcher->setOrigValue(voltageRangeIdx);
            ctrlDispatchers->push_back(voltageRangeDispatcher);

            connect(voltageRangeEdit, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, [=] (int rangeIdx) {
                voltageRangeDispatcher->onCtrlChanged(rangeIdx);
                this->onPropertyChanged();
                if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                    protocolEditor->onStimulusRangeSelected(rangeIdx);
                    RangedMeasurement_t stimulusRange;
                    msgDisp->getVoltageProtocolRangeFeature((unsigned int)rangeIdx, stimulusRange);
                    initQdoubleSpinBox(holdEdit, stimulusRange, RangedQDoubleSpinBox_t::MIN_MAX);
//                    holdEdit->setRangedMeasurement(stimulusRange, QDoubleSpinBox::MinMaxRange);
                    holdUnitLbl->setText(QString::fromStdString(stimulusRange.getFullUnit()));
                }
            });

            propertyCtrlLo->addWidget(voltageRangeLbl, PPD_VOLTAGE_RANGE_ROW, 0, 1, 1, Qt::AlignRight);
            propertyCtrlLo->addWidget(voltageRangeEdit, PPD_VOLTAGE_RANGE_ROW, 1, 1, 1);

            if (((clampingModality == ClampingModality_t::VOLTAGE_CLAMP) && (voltageRangeEditOrig->count() < 2)) ||
                    ((clampingModality == ClampingModality_t::CURRENT_CLAMP) && (voltageRangeEditOrig->count() < 3))) {
                voltageRangeLbl->setVisible(false);
                voltageRangeEdit->setVisible(false);
            }
        }

        /*! Sampling Rate control */
        if (samplingRateEditOrig != nullptr) {
            QLabel * samplingRateLbl = new QLabel("Set sampling rate");
            samplingRateEdit = new QComboBox();
            samplingRateEdit->setMinimumWidth(50);
            for (int idx = 0; idx < samplingRateEditOrig->count(); idx++) {
                samplingRateEdit->addItem(samplingRateEditOrig->itemText(idx));
            }
            int samplingRateIdx = samplingRateEditOrig->currentIndex();
            samplingRateEdit->setCurrentIndex(samplingRateIdx);
            ProtocolComboBoxCtrlDispatcher * samplingRateDispatcher = new ProtocolComboBoxCtrlDispatcher();
            samplingRateDispatcher->setEdit(samplingRateEditOrig);
            samplingRateDispatcher->setOrigValue(samplingRateIdx);
            ctrlDispatchers->push_back(samplingRateDispatcher);
            connect(samplingRateEdit, QOverload <int> ::of(&QComboBox::currentIndexChanged), samplingRateDispatcher, &ProtocolComboBoxCtrlDispatcher::onCtrlChanged);
            connect(samplingRateEdit, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, &ProtocolWidget::onPropertyChanged);
            propertyCtrlLo->addWidget(samplingRateLbl, PPD_SAMPLING_RATE_ROW, 0, 1, 1, Qt::AlignRight);
            propertyCtrlLo->addWidget(samplingRateEdit, PPD_SAMPLING_RATE_ROW, 1, 1, 1);
        }

        ctrlItems = protocolEditor->getCtrlItems();
        int rowIdx = PPD_CTRL_FIRST_ROW;

        for (int ctrlIdx = 0; ctrlIdx < ctrlItems->size(); ctrlIdx++) {
            ProtocolDropControlItem * ctrlItem = ctrlItems->at(ctrlIdx);

            propertyCtrlLo->addWidget(new QLabel(ctrlItem->getName()), rowIdx, 0, Qt::AlignRight);

            switch (ctrlItem->getItemCtrlType()) {
            case ProtocolItemCtrlVoltage:
            case ProtocolItemCtrlCurrent:
            case ProtocolItemCtrlTime:
            case ProtocolItemCtrlFrequency: {
                QDoubleSpinBox * doubleEditOrig = ctrlItem->getDoubleEdit();
                QDoubleSpinBox * doubleEdit = new QDoubleSpinBox();
                doubleEdit->setMinimumWidth(50);
                copy(doubleEditOrig, doubleEdit);

                ProtocolDoubleSpinBoxCtrlDispatcher * dispatcher = new ProtocolDoubleSpinBoxCtrlDispatcher();
                dispatcher->setCtrl(ctrlItem);
                dispatcher->setEdit(doubleEditOrig);
                ctrlDispatchers->push_back(dispatcher);
                connect(doubleEdit, QOverload <double> ::of(&QDoubleSpinBox::valueChanged), dispatcher, &ProtocolDoubleSpinBoxCtrlDispatcher::onCtrlChanged);
                connect(doubleEdit, QOverload <double> ::of(&QDoubleSpinBox::valueChanged), this, &ProtocolWidget::onPropertyChanged);

                QLabel * unitLbl = new QLabel(ctrlItem->getUnit());

                propertyCtrlLo->addWidget(doubleEdit, rowIdx, 1);
                propertyCtrlLo->addWidget(unitLbl, rowIdx++, 2);

                if (ctrlItem->getItemCtrlType() == ProtocolItemCtrlVoltage) {
                    connect(voltageRangeEdit, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, [=] (int) {
                        copy(doubleEditOrig, doubleEdit);
                        unitLbl->setText(ctrlItem->getUnit());
                    });
                }

                if (ctrlItem->getItemCtrlType() == ProtocolItemCtrlCurrent) {
                    connect(currentRangeEdit, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, [=] (int) {
                        copy(doubleEditOrig, doubleEdit);
                        unitLbl->setText(ctrlItem->getUnit());
                    });
                }

                break;
            }

            case ProtocolItemCtrlNaturalNum: {
                QSpinBox * intEditOrig = ctrlItem->getIntEdit();
                QSpinBox * intEdit = new QSpinBox();
                intEdit->setMinimumWidth(50);
                intEdit->setRange(intEditOrig->minimum(), intEditOrig->maximum());
                intEdit->setValue(intEditOrig->value());

                ProtocolSpinBoxCtrlDispatcher * dispatcher = new ProtocolSpinBoxCtrlDispatcher();
                dispatcher->setCtrl(ctrlItem);
                dispatcher->setEdit(intEditOrig);
                ctrlDispatchers->push_back(dispatcher);
                connect(intEdit, QOverload <int> ::of(&QSpinBox::valueChanged), dispatcher, &ProtocolSpinBoxCtrlDispatcher::onCtrlChanged);
                connect(intEdit, QOverload <int> ::of(&QSpinBox::valueChanged), this, &ProtocolWidget::onPropertyChanged);

                propertyCtrlLo->addWidget(intEdit, rowIdx, 1);
                propertyCtrlLo->addWidget(new QLabel(ctrlItem->getUnit()), rowIdx++, 2);
                break;
            }

            case ProtocolItemCtrlNone:
                break;
            }
        }

        QWidget * spacer = new QWidget;
        spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
        propertyCtrlLo->addWidget(spacer);

        if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
            RangedMeasurement_t stimulusRange;
            msgDisp->getVoltageProtocolRangeFeature((unsigned int)voltageRangeEditOrig->currentIndex(), stimulusRange);
            dialog->getProtocolPreview()->setStimulusRange(stimulusRange);

        } else {
            RangedMeasurement_t stimulusRange;
            msgDisp->getCurrentProtocolRangeFeature((unsigned int)currentRangeEditOrig->currentIndex(), stimulusRange);
            dialog->getProtocolPreview()->setStimulusRange(stimulusRange);
        }

        /*! Lower buttons */
        propertyOkButton = new QPushButton("OK");
        propertyOkButton->setCheckable(false);
        propertyBtnsHl->addWidget(propertyOkButton);
        connect(propertyOkButton, &QPushButton::clicked, this, &ProtocolWidget::onAcceptPropertyDialog);
        propertyOkButton->setDefault(true);

        propertyCancButton = new QPushButton("Cancel");
        propertyCancButton->setCheckable(false);
        propertyBtnsHl->addWidget(propertyCancButton);
        connect(propertyCancButton, &QPushButton::clicked, this, &ProtocolWidget::onRejectPropertyDialog);

        dialog->setOwner(this);
        dialog->setVisible(true);
        dialogPopulated = true;
        propertyChangedFlag = false;

        protocolEditor->onUpdateProtocol();
    }
}

ProtocolWidget::ClosureRequestReply_t ProtocolWidget::askForPropertyDialogClosure() {
    ClosureRequestReply_t ret = ClosureRequestDontSave;
    if (propertyChangedFlag) {

        QMessageBox msgBox;
        msgBox.setText("A protocol property panel with unsaved changes is currently open.\n"
                       "Do you want to save or discard the changes?");
        QAbstractButton * saveBtn = msgBox.addButton("Save", QMessageBox::NoRole);
        QAbstractButton * discardBtn = msgBox.addButton("Discard", QMessageBox::NoRole);
        QAbstractButton * cancelBtn = msgBox.addButton("Cancel", QMessageBox::NoRole);

        msgBox.exec();
        QAbstractButton * clickedBtn = msgBox.clickedButton();

        if (clickedBtn == saveBtn) {
            ret = ClosureRequestSave;

        } else if (clickedBtn == discardBtn) {
            ret = ClosureRequestDontSave;

        } else if (clickedBtn == cancelBtn) {
            ret = ClosureRequestRejected;
        }
    }

    if (ret == ClosureRequestSave) {
        this->onAcceptPropertyDialog();

    } else if (ret == ClosureRequestDontSave) {
        this->onRejectPropertyDialog();
    }

    return ret;
}

void ProtocolWidget::clearPropertyDialog() {
    if (dialogPopulated) {
        QLayoutItem * item;
        int rowIdx;
        for (rowIdx = 0; rowIdx < PPD_CTRL_FIRST_ROW; rowIdx++) {
            for (int colIdx = 0; colIdx < PPD_COLUMNS_NUM; colIdx++) {
                item = propertyCtrlLo->itemAtPosition(rowIdx, colIdx);
                if (item != nullptr) {
                    propertyCtrlLo->removeItem(item);
                    delete item->widget();
                }
            }
            if (rowIdx >= PPD_HOLD_ROW) {
                delete ctrlDispatchers->at(rowIdx-PPD_HOLD_ROW);
            }
        }

        if (ctrlItems != nullptr) {
            for (int ctrlIdx = 0; ctrlIdx < ctrlItems->size(); ctrlIdx++) {
                for (int colIdx = 0; colIdx < PPD_COLUMNS_NUM; colIdx++) {
                    item = propertyCtrlLo->itemAtPosition(rowIdx+ctrlIdx, colIdx);
                    propertyCtrlLo->removeItem(item);
                    delete item->widget();
                }
                delete ctrlDispatchers->at(ctrlIdx+PPD_CTRL_DISPATCHER_IDX);
            }

            for (int colIdx = 0; colIdx < PPD_COLUMNS_NUM; colIdx++) {
                item = propertyCtrlLo->itemAtPosition(rowIdx+ctrlItems->size(), colIdx);
                if (item != nullptr) {
                    propertyCtrlLo->removeItem(item);
                    delete item->widget();
                }
            }
            ctrlItems = nullptr;
        }

        ctrlDispatchers->clear();

        delete propertyOkButton;
        propertyOkButton = nullptr;
        delete propertyCancButton;
        propertyCancButton = nullptr;

        dialogPopulated = false;
    }
}

int ProtocolWidget::openProtocolEditor() {
    return protocolEditor->exec();
}

ProtocolType_t ProtocolWidget::getType() {
    return type;
}

e384CommLib::ClampingModality_t ProtocolWidget::getClampingModality() {
    return clampingModality;
}

QVector <ProtocolDropItem *> * ProtocolWidget::getProtocolDropItems() {
    return protocolEditor->getDropItems();
}

void ProtocolWidget::pushProtocolItems(QVector <ProtocolItem *> items) {
    pushedProtocolItems = items;
}

void ProtocolWidget::setProtocolItems() {
    this->setProcessingStatus();

    if (processingStatus == ItemsProcOk) {
        /*! Delete previous protocol */
        /*! \todo FCON qui converrebbe cancellare gli elementi del vecchio protocollo, ma se li sta ancora usando qualcun'altro (tipo il plot o il datawriter)
         * gli si cancellano i dati sotto i piedi. Questa cosa è da verificare */
//        for (int idx = 0; idx < protocolItems.size(); idx++) {

//        }
        protocolItems.clear();

        for (int idx = 0; idx < pushedProtocolItems.size(); idx++) {
            switch (pushedProtocolItems[idx]->type) {
            case ProtocolItemXStepTStep:
                protocolItems.push_back(new ProtocolXStepTStepItem(pushedProtocolItems[idx]));
                break;

            case ProtocolItemXRamp:
                protocolItems.push_back(new ProtocolXRampItem(pushedProtocolItems[idx]));
                break;

            case ProtocolItemXSin:
                protocolItems.push_back(new ProtocolXSinItem(pushedProtocolItems[idx]));
                break;
            }
        }
    }
}

QVector <ProtocolItem *> ProtocolWidget::getProtocolItems() {
    return protocolItems;
}

void ProtocolWidget::pushAnalysisCursors(QVector <AnalysisCursor *> cursors) {
    pushedAnalysisCursors = cursors;
}

void ProtocolWidget::pushTriggerCursors(QVector <TriggerCursor *> cursors) {
    pushedTriggerCursors = cursors;
    cursorsValid = (pushedTriggerCursors.size() > (int)maxTriggerEvents ? false : true);
    protocolEditor->setTooManyTriggersWarning(!cursorsValid);
    this->setValidityColor();
}

QVector <AnalysisCursor *> ProtocolWidget::getAnalysisCursors() {
    return analysisCursors;
}

void ProtocolWidget::setTriggerCursors() {
    if (processingStatus == ItemsProcOk) {
        /*! Delete previous cursors */
        /*! \todo FCON qui converrebbe cancellare gli elementi del vecchio protocollo, ma se li sta ancora usando qualche analisi
         * gli si cancellano i dati sotto i piedi. Questa cosa è da verificare */
//        for (int idx = 0; idx < triggerCursors.size(); idx++) {

//        }
        triggerCursors.clear();

        if (pushedTriggerCursors.size() <= (int)maxTriggerEvents) {
            for (int idx = 0; idx < pushedTriggerCursors.size(); idx++) {
                triggerCursors.push_back(new TriggerCursor(pushedTriggerCursors[idx]));
            }

        } else {
            /*! If there are too many triggers set, select only the available ones plus the terminator */
            for (int idx = 0; idx < (int)maxTriggerEvents-1; idx++) {
                triggerCursors.push_back(new TriggerCursor(pushedTriggerCursors[idx]));
            }
            triggerCursors.push_back(new TriggerCursor(pushedTriggerCursors.back()));
        }
    }
}

QVector <TriggerCursor *> ProtocolWidget::getTriggerCursors() {
    return triggerCursors;
}

void ProtocolWidget::pushProcessingStatus(ItemsProcStatus_t status) {
    pushedProcessingStatus = status;
}

void ProtocolWidget::setProcessingStatus() {
    processingStatus = pushedProcessingStatus;
}

ItemsProcStatus_t ProtocolWidget::getProcessingStatus() {
    return processingStatus;
}

void ProtocolWidget::setInhibition(bool inhibitFlag) {
    protocolInhibited = inhibitFlag;
    this->setValidityColor();
}

bool ProtocolWidget::isInhibited() {
    return protocolInhibited;
}

void ProtocolWidget::setProtocolSections(ProtocolSections * sections) {
    protocolSections = sections;
}

ProtocolSection * ProtocolWidget::getSectionByItem(int itemIdx, int repsIdx, int sweepIdx) {
    return protocolSections->getSectionByItem(itemIdx, repsIdx, sweepIdx);
}

void ProtocolWidget::setHold(Measurement_t hold) {
    holdEditOrig->setValue(hold.value);
}

Measurement_t ProtocolWidget::getHold() {
    hold.value = holdEditOrig->value();
    hold.prefix = model->getStimulusRange().prefix;
    return hold;
}

void ProtocolWidget::setHoldingDelta(Measurement_t &holdingDelta) {
    protocolEditor->setHoldingDelta(holdingDelta);
}

UnitPfx_t ProtocolWidget::getStimulusPrefix() {
    return model->getStimulusRange().prefix;
}

bool ProtocolWidget::getHoldRef() {
    return holdRefEditOrig->isChecked();
}

int ProtocolWidget::getSweepsNum() {
    return sweepsNumEditOrig->value();
}

int ProtocolWidget::getSamplingRateIndex() {
    return samplingRateEditOrig->currentIndex()-1; /*!< -1 is to remove the offset due to the "none" option */
}

void ProtocolWidget::setProtocolValid(bool valid) {
    protocolValid = valid;
    this->setValidityColor();
}

void ProtocolWidget::setAppliedRange(RangedMeasurement_t &newAppliedRange) {
    appliedRange = newAppliedRange;
}

RangedMeasurement_t ProtocolWidget::getAppliedRange() {
    return appliedRange;
}

void ProtocolWidget::getSweepTrigger(double &value, bool &rising) {
    value = 0;
    rising = true;
    int firstItem = -1;
    int lastItem = -2;

    for (int itemIdx = 0; itemIdx < protocolItems.size(); itemIdx++) {
        if (protocolItems[itemIdx]->visible) {
            firstItem = itemIdx;
            break;
        }
    }

    for (int itemIdx = protocolItems.size()-1; itemIdx >= 0; itemIdx--) {
        if (protocolItems[itemIdx]->visible) {
            lastItem = itemIdx;
            break;
        }
    }

    if (lastItem >= firstItem) {
        double firstValue = 0.0;
        double lastValue = 0.0;

        switch (protocolItems[firstItem]->type) {
        case ProtocolItemXStepTStep: {
            ProtocolXStepTStepItem * item = static_cast <ProtocolXStepTStepItem *> (protocolItems[firstItem]);
            firstValue = item->x0;
            break;
        }

        case ProtocolItemXRamp: {
            ProtocolXRampItem * item = static_cast <ProtocolXRampItem *> (protocolItems[firstItem]);
            firstValue = item->x0;
            if (item->xFinal > item->x0) {
                rising = true;

            } else if (item->x0 > item->xFinal) {
                rising = false;
            }
            break;
        }

        case ProtocolItemXSin: {
            ProtocolXSinItem * item = static_cast <ProtocolXSinItem *> (protocolItems[firstItem]);
            firstValue = item->x0;
            rising = true;
            break;
        }
        }

        switch (protocolItems[firstItem]->type) {
        case ProtocolItemXStepTStep: {
            ProtocolXStepTStepItem * item = static_cast <ProtocolXStepTStepItem *> (protocolItems[lastItem]);
            if (sweepsNumEditOrig->value() > 1) {
                lastValue = item->x0+item->xStep*(double)(sweepsNumEditOrig->value()-1);

            } else if ((item->repsNum > 0) && (item->applySteps)) {
                lastValue = item->x0+item->xStep*(double)(item->repsNum-1);

            } else {
                lastValue = item->x0;
            }
            break;
        }

        case ProtocolItemXRamp: {
            ProtocolXRampItem * item = static_cast <ProtocolXRampItem *> (protocolItems[lastItem]);
            lastValue = item->xFinal;
            if (item->xFinal > item->x0) {
                rising = true;

            } else if (item->x0 > item->xFinal) {
                rising = false;
            }
            break;
        }

        case ProtocolItemXSin: {
            ProtocolXSinItem * item = static_cast <ProtocolXSinItem *> (protocolItems[lastItem]);
            lastValue = item->x0;
            rising = true;
            break;
        }
        }

        if (firstValue > lastValue) {
            rising = true;

        } else if (lastValue > firstValue) {
            rising = false;
        }
        value = 0.5*(firstValue+lastValue);
    }
}

bool ProtocolWidget::hasInfiniteRepetition() {
    return (protocolItems.back()->repsNum == 0 ? true : false);
}

YAML::VoltageProtocol ProtocolWidget::getYamlVoltageProtocol() {
    YAML::VoltageProtocol yamlProtocol = protocolEditor->getYamlVoltageProtocol();
    yamlProtocol.shortcutindex = shortCutIdx;
    return yamlProtocol;
}

YAML::CurrentProtocol ProtocolWidget::getYamlCurrentProtocol() {
    YAML::CurrentProtocol yamlProtocol = protocolEditor->getYamlCurrentProtocol();
    yamlProtocol.shortcutindex = shortCutIdx;
    return yamlProtocol;
}

void ProtocolWidget::setProtocolFromYaml(const YAML::VoltageProtocol &yamlProtocol) {
    protocolEditor->setProtocolFromYaml(yamlProtocol);
}

void ProtocolWidget::setProtocolFromYaml(const YAML::CurrentProtocol &yamlProtocol) {
    protocolEditor->setProtocolFromYaml(yamlProtocol);
}

void ProtocolWidget::onAcceptPropertyDialog() {
    name = nameEdit->text();
    emit nameChanged(protocolEditor->getName(), name);
    protocolEditor->setName(name);
    this->updateText();

    this->clearPropertyDialog();
    dialog->setOwner(nullptr);
    dialog->setVisible(false);
//    propertyDialog->accept();
}

void ProtocolWidget::onRejectPropertyDialog() {
    for (int dispatcherIdx = 0; dispatcherIdx < ctrlDispatchers->size(); dispatcherIdx++) {
        ctrlDispatchers->at(dispatcherIdx)->setOrigValueBack();
    }

    this->clearPropertyDialog();
    dialog->setOwner(nullptr);
    dialog->setVisible(false);
}

void ProtocolWidget::onCheckAnalysisValid() {
    analysisValid = protocolEditor->allAnalysisValid();
    this->setValidityColor();
}

void ProtocolWidget::updateText() {
    QString suffix = "";
    if (shortCutIdx >= 0) {
        suffix = QString(" (%1)").arg(shortCutIdx);
    }
    this->setText(name+suffix);
}

void ProtocolWidget::setValidityColor() {
    if (protocolValid && !protocolInhibited) {
        if (analysisValid && cursorsValid) {
            this->setBackground(PROT_WIDGET_VALID_COLOR);

        } else {
            this->setBackground(PROT_WIDGET_ANALYSIS_INVALID_COLOR);
        }

    } else {
        this->setBackground(PROT_WIDGET_INVALID_COLOR);
    }
}

void ProtocolWidget::onCheckNameAvailability(QString name) {
    if (protocolsNames.contains(name)) {
        propertyOkButton->setEnabled(false);

    } else if (name.isEmpty()) {
        propertyOkButton->setEnabled(false);

    } else {
        propertyOkButton->setEnabled(true);
    }
}

void ProtocolWidget::onPropertyChanged() {
    propertyChangedFlag = true;
}

VoltageProtocolWidget::VoltageProtocolWidget() {
    holdName = "V-Hold";
    hold.unit = "V";
}

void VoltageProtocolWidget::setStimulusRangeIndex(int idx) {
    voltageRangeEditOrig->setCurrentIndex(idx);
}

int VoltageProtocolWidget::getStimulusRangeIndex() {
    return voltageRangeEditOrig->currentIndex(); /*!< "none" option is not present in stimulus range */
}

int VoltageProtocolWidget::getCurrentRangeIndex() {
    return currentRangeEditOrig->currentIndex()-1; /*!< -1 is to remove the offset due to the "none" option */
}

int VoltageProtocolWidget::getVoltageRangeIndex() {
    return voltageRangeEditOrig->currentIndex(); /*!< "none" option is not present in voltage clamp */
}

CurrentProtocolWidget::CurrentProtocolWidget() {
    holdName = "I-Hold";
    hold.unit = "A";
}

void CurrentProtocolWidget::setStimulusRangeIndex(int idx) {
    currentRangeEditOrig->setCurrentIndex(idx);
}

int CurrentProtocolWidget::getStimulusRangeIndex() {
    return currentRangeEditOrig->currentIndex(); /*!< "none" option is not present in stimulus range */
}

int CurrentProtocolWidget::getCurrentRangeIndex() {
    return currentRangeEditOrig->currentIndex(); /*!< "none" option is not present in current clamp */
}

int CurrentProtocolWidget::getVoltageRangeIndex() {
    return voltageRangeEditOrig->currentIndex()-1; /*!< -1 is to remove the offset due to the "none" option */
}

GapfreeProtocolWidget::GapfreeProtocolWidget() {
    QString iconString = ":imgs/gap free icon.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);
}

void GapfreeProtocolWidget::setAnalysisCursors() {
    if (processingStatus == ItemsProcOk) {
        /*! Delete previous cursors */
        /*! \todo FCON qui converrebbe cancellare gli elementi del vecchio protocollo, ma se li sta ancora usando qualche analisi
         * gli si cancellano i dati sotto i piedi. Questa cosa è da verificare */
//        for (int idx = 0; idx < analysisCursors.size(); idx++) {

//        }
        analysisCursors.clear();

        for (int idx = 0; idx < pushedAnalysisCursors.size(); idx++) {
            switch (pushedAnalysisCursors[idx]->type) {
            case AnalysisCursorGapFree:
                analysisCursors.push_back(new AnalysisGapFreeCursor(pushedAnalysisCursors[idx]));
                break;

            case AnalysisCursorGapFreeAllReps:
                analysisCursors.push_back(new AnalysisGapFreeAllRepsCursor(pushedAnalysisCursors[idx]));
                break;

            case AnalysisCursorGapFreeInfReps:
                analysisCursors.push_back(new AnalysisGapFreeInfRepsCursor(pushedAnalysisCursors[idx]));
                break;

            case AnalysisCursorGapFreeInfAllReps:
                analysisCursors.push_back(new AnalysisGapFreeInfAllRepsCursor(pushedAnalysisCursors[idx]));
                break;

            default:
                /*! Nothing to do */
                break;
            }
        }
    }
}

double GapfreeProtocolWidget::getTotalDuration() {
    double totalDuration = 0.0;
    for (int itemIdx = 0; itemIdx < protocolItems.size(); itemIdx++) {
        if (protocolItems[itemIdx]->nextItem > itemIdx) {
            totalDuration += protocolItems[itemIdx]->duration();

        } else {
            /*! Last item of a loop is not added before, so it is added here */
            totalDuration += protocolItems[itemIdx]->duration();

            /*! In loops the item duration is subtracted once cause it has already been added when the item was processed in the first place */
            for (int loopItemIdx = protocolItems[itemIdx]->nextItem; loopItemIdx <= itemIdx; loopItemIdx++) {
                totalDuration += protocolItems[loopItemIdx]->totalDuration(protocolItems[itemIdx]->repsNum)-
                        protocolItems[loopItemIdx]->duration();
            }
        }
    }
    return totalDuration;
}

ProtocolSection * GapfreeProtocolWidget::getItemAtTime(double time, int, double &offset) {
    if (protocolSections->size() == 0) {
        return nullptr;
    }

    int sectionIdx = 0;

    if (time < 0.0) {
        offset = 0.0;
        return protocolSections->at(sectionIdx);
    }

    while (sectionIdx < protocolSections->size()) {
        if (time < protocolSections->at(sectionIdx)->endingTime) {
            offset = time-protocolSections->at(sectionIdx)->startingTime;
            return protocolSections->at(sectionIdx);
        }
        sectionIdx++;
    }

    sectionIdx--;
    offset = protocolSections->at(sectionIdx)->duration();
    return protocolSections->at(sectionIdx);
}

ProtocolSection * GapfreeProtocolWidget::getItemAtTime(double time, int itemIdx, int repsIdx, int, double &offset) {
    if (protocolSections->size() == 0) {
        return nullptr;
    }

    int sectionIdx = 0;

    if (time < 0.0) {
        offset = 0.0;
        return protocolSections->at(sectionIdx);
    }

    while (sectionIdx < protocolSections->size()) {
        if (time < protocolSections->at(sectionIdx)->endingTime) {
            if ((itemIdx == protocolSections->at(sectionIdx)->itemIdx) &&
                    (repsIdx == protocolSections->at(sectionIdx)->repsIdx)) {
                offset = time-protocolSections->at(sectionIdx)->startingTime;
                return protocolSections->at(sectionIdx);

            } else if (((itemIdx > protocolSections->at(sectionIdx)->itemIdx) &&
                       repsIdx == protocolSections->at(sectionIdx)->repsIdx) ||
                       (repsIdx > protocolSections->at(sectionIdx)->repsIdx)) {
                /*! If the itemIdx and repsIdx do not match it means that the cursor is on the borderline between 2 items:
                 *  this is covers the case in which the previous section was obtained, so increase the sectiondIdx by 1 and set
                 *  0.0 as offset */
                sectionIdx++;
                offset = 0.0;
                return protocolSections->at(sectionIdx);

            } else if (((itemIdx < protocolSections->at(sectionIdx)->itemIdx) &&
                        repsIdx == protocolSections->at(sectionIdx)->repsIdx) ||
                        (repsIdx < protocolSections->at(sectionIdx)->repsIdx)) {
                /*! If the itemIdx and repsIdx do not match it means that the cursor is on the borderline between 2 items:
                 *  this if covers the case in which the following section, so decrease the sectiondIdx by 1 and set
                 *  the whole section duration as offset */
                sectionIdx--;
                offset = protocolSections->at(sectionIdx)->duration();
                return protocolSections->at(sectionIdx);
            }
        }
        sectionIdx++;
    }

    sectionIdx--;
    offset = protocolSections->at(sectionIdx)->duration();
    return protocolSections->at(sectionIdx);
}

EpisodicProtocolWidget::EpisodicProtocolWidget() {
    QString iconString = ":imgs/episodic icon.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);
}

void EpisodicProtocolWidget::setAnalysisCursors() {
    if (processingStatus == ItemsProcOk) {
        /*! Delete previous cursors */
        /*! \todo FCON qui converrebbe cancellare gli elementi del vecchio protocollo, ma se li sta ancora usando qualche analisi
         * gli si cancellano i dati sotto i piedi. Questa cosa è da verificare */
//        for (int idx = 0; idx < analysisCursors.size(); idx++) {

//        }
        analysisCursors.clear();

        for (int idx = 0; idx < pushedAnalysisCursors.size(); idx++) {
            switch (pushedAnalysisCursors[idx]->type) {
            case AnalysisCursorEpisodic:
                analysisCursors.push_back(new AnalysisEpisodicCursor(pushedAnalysisCursors[idx]));
                break;

            case AnalysisCursorEpisodicAllReps:
                analysisCursors.push_back(new AnalysisEpisodicAllRepsCursor(pushedAnalysisCursors[idx]));
                break;

            case AnalysisCursorEpisodicAllSweeps:
                analysisCursors.push_back(new AnalysisEpisodicAllSweepsCursor(pushedAnalysisCursors[idx]));
                break;

            case AnalysisCursorEpisodicAllRepsAllSweeps:
                analysisCursors.push_back(new AnalysisEpisodicAllRepsAllSweepsCursor(pushedAnalysisCursors[idx]));
                break;

            default:
                /*! Nothing to do */
                break;
            }
        }
    }
}

double EpisodicProtocolWidget::getTotalDuration() {
    double totalDuration = 0.0;
    int sweepsNum = sweepsNumEditOrig->value();
    for (int itemIdx = 0; itemIdx < protocolItems.size(); itemIdx++) {
        if (protocolItems[itemIdx]->nextItem > itemIdx) {
            totalDuration += protocolItems[itemIdx]->maxDuration(sweepsNum);

        } else {
            /*! Last item of a loop is not added before, so it is added here */
            totalDuration += protocolItems[itemIdx]->maxDuration(sweepsNum);

            /*! In loops the item duration is subtracted once cause it has already been added when the item was processed in the first place */
            for (int loopItemIdx = protocolItems[itemIdx]->nextItem; loopItemIdx <= itemIdx; loopItemIdx++) {
                totalDuration += protocolItems[loopItemIdx]->maxDuration(sweepsNum)*(double)(protocolItems[itemIdx]->repsNum-1);
            }
        }
    }
    return totalDuration;
}

ProtocolSection * EpisodicProtocolWidget::getItemAtTime(double time, int sweepIdx, double &offset) {
    if (protocolSections->size() == 0) {
        return nullptr;
    }

    int sectionIdx = 0;

    if (time < 0.0) {
        offset = 0.0;
        return protocolSections->at(sectionIdx);
    }

    while (sectionIdx < protocolSections->size()) {
        if (sweepIdx < protocolSections->at(sectionIdx)->sweepIdx) {
            break;

        } else if (sweepIdx == protocolSections->at(sectionIdx)->sweepIdx) {
            if (time < protocolSections->at(sectionIdx)->endingTime) {
                offset = time-protocolSections->at(sectionIdx)->startingTime;
                return protocolSections->at(sectionIdx);
            }
        }

        sectionIdx++;
    }

    sectionIdx--;
    offset = protocolSections->at(sectionIdx)->duration();
    return protocolSections->at(sectionIdx);
}

ProtocolSection * EpisodicProtocolWidget::getItemAtTime(double time, int itemIdx, int repsIdx, int sweepIdx, double &offset) {
    if (protocolSections->size() == 0) {
        return nullptr;
    }

    int sectionIdx = 0;

    if (time < 0.0) {
        offset = 0.0;
        return protocolSections->at(sectionIdx);
    }

    while (sectionIdx < protocolSections->size()) {
        if (sweepIdx < protocolSections->at(sectionIdx)->sweepIdx) {
            break;

        } else if (sweepIdx == protocolSections->at(sectionIdx)->sweepIdx) {
            if (time < protocolSections->at(sectionIdx)->endingTime) {
                if ((itemIdx == protocolSections->at(sectionIdx)->itemIdx) &&
                        (repsIdx == protocolSections->at(sectionIdx)->repsIdx)) {
                    offset = time-protocolSections->at(sectionIdx)->startingTime;
                    return protocolSections->at(sectionIdx);

                } else {
                    /*! If the itemIdx and repsIdx do not match it means that the cursor is on the borderline between 2 items: in these cases the
                     *  time disequality always selects the following section, so it's enough to reduce the sectiondIdx by 1 and set
                     *  the whole section duration as offset */
                    if (sectionIdx > 0) {
                        sectionIdx--;
                        offset = protocolSections->at(sectionIdx)->duration();
                        return protocolSections->at(sectionIdx);

                    } else {
                        return nullptr;
                    }
                }
            }
        }
        sectionIdx++;
    }

    sectionIdx--;
    offset = protocolSections->at(sectionIdx)->duration();
    return protocolSections->at(sectionIdx);
}

GapfreeVoltageProtocolWidget::GapfreeVoltageProtocolWidget(MessageDispatcher * msgDisp, QString name, ProtocolPropertyDialog * dialog) :
    ProtocolWidget(msgDisp, name, dialog, ProtocolTypeGapfree, ClampingModality_t::VOLTAGE_CLAMP),
    VoltageProtocolWidget(),
    GapfreeProtocolWidget() {

}

EpisodicVoltageProtocolWidget::EpisodicVoltageProtocolWidget(MessageDispatcher * msgDisp, QString name, ProtocolPropertyDialog * dialog) :
    ProtocolWidget(msgDisp, name, dialog, ProtocolTypeEpisodic, ClampingModality_t::VOLTAGE_CLAMP),
    VoltageProtocolWidget(),
    EpisodicProtocolWidget() {

}

GapfreeCurrentProtocolWidget::GapfreeCurrentProtocolWidget(MessageDispatcher * msgDisp, QString name, ProtocolPropertyDialog * dialog) :
    ProtocolWidget(msgDisp, name, dialog, ProtocolTypeGapfree, ClampingModality_t::CURRENT_CLAMP),
    CurrentProtocolWidget(),
    GapfreeProtocolWidget() {

}

EpisodicCurrentProtocolWidget::EpisodicCurrentProtocolWidget(MessageDispatcher * msgDisp, QString name, ProtocolPropertyDialog * dialog) :
    ProtocolWidget(msgDisp, name, dialog, ProtocolTypeEpisodic, ClampingModality_t::CURRENT_CLAMP),
    CurrentProtocolWidget(),
    EpisodicProtocolWidget() {

}

ProtocolCtrlDispatcher::ProtocolCtrlDispatcher() {

}

void ProtocolCtrlDispatcher::setCtrl(ProtocolDropControlItem * c) {
    ctrl = c;
}

ProtocolBoolCtrlDispatcher::ProtocolBoolCtrlDispatcher() : ProtocolCtrlDispatcher() {

}

void ProtocolBoolCtrlDispatcher::setOrigValue(bool value) {
    origValue = value;
}

ProtocolCheckBoxCtrlDispatcher::ProtocolCheckBoxCtrlDispatcher() : ProtocolBoolCtrlDispatcher() {

}

void ProtocolCheckBoxCtrlDispatcher::setEdit(QCheckBox * e) {
    objEdit = e;
    origValue = objEdit->isChecked();
}

void ProtocolCheckBoxCtrlDispatcher::setOrigValueBack() {
    this->onCtrlChanged(origValue);
}

void ProtocolCheckBoxCtrlDispatcher::onCtrlChanged(bool value) {
    objEdit->setChecked(value);
    if (ctrl != nullptr) {
        ctrl->onAcceptPropertyDialog();
    }
}

ProtocolIntCtrlDispatcher::ProtocolIntCtrlDispatcher() : ProtocolCtrlDispatcher() {

}

void ProtocolIntCtrlDispatcher::setOrigValue(int value) {
    origValue = value;
}

ProtocolSpinBoxCtrlDispatcher::ProtocolSpinBoxCtrlDispatcher() : ProtocolIntCtrlDispatcher() {

}

void ProtocolSpinBoxCtrlDispatcher::setEdit(QSpinBox * e) {
    objEdit = e;
    origValue = objEdit->value();
}

void ProtocolSpinBoxCtrlDispatcher::setOrigValueBack() {
    this->onCtrlChanged(origValue);
}

void ProtocolSpinBoxCtrlDispatcher::onCtrlChanged(int value) {
    objEdit->setValue(value);
    if (ctrl != nullptr) {
        ctrl->onAcceptPropertyDialog();
    }
}

ProtocolComboBoxCtrlDispatcher::ProtocolComboBoxCtrlDispatcher() : ProtocolIntCtrlDispatcher() {

}

void ProtocolComboBoxCtrlDispatcher::setEdit(QComboBox * e) {
    objEdit = e;
    origValue = objEdit->currentIndex();
}

void ProtocolComboBoxCtrlDispatcher::setOrigValueBack() {
    this->onCtrlChanged(origValue);
}

void ProtocolComboBoxCtrlDispatcher::onCtrlChanged(int value) {
    objEdit->setCurrentIndex(value);
    if (ctrl != nullptr) {
        ctrl->onAcceptPropertyDialog();
    }
}

ProtocolDoubleCtrlDispatcher::ProtocolDoubleCtrlDispatcher() : ProtocolCtrlDispatcher() {

}

void ProtocolDoubleCtrlDispatcher::setOrigValue(double value) {
    origValue = value;
}

ProtocolDoubleSpinBoxCtrlDispatcher::ProtocolDoubleSpinBoxCtrlDispatcher() : ProtocolDoubleCtrlDispatcher() {

}

void ProtocolDoubleSpinBoxCtrlDispatcher::setEdit(QDoubleSpinBox * e) {
    objEdit = e;
    origValue = objEdit->value();
}

void ProtocolDoubleSpinBoxCtrlDispatcher::setOrigValueBack() {
    this->onCtrlChanged(origValue);
}

void ProtocolDoubleSpinBoxCtrlDispatcher::onCtrlChanged(double value) {
    objEdit->setValue(value);
    if (ctrl != nullptr) {
        ctrl->onAcceptPropertyDialog();
    }
}
