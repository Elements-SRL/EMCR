#include "protocolitemdroplist.h"

#include <QtWidgets>

ProtocolItemDropList::ProtocolItemDropList(e4gcl::CommLib * commLib, QDoubleSpinBox * holdEdit, int clampingModality) :
    QListWidget(),
    commLib(commLib),
    holdEdit(holdEdit),
    clampingModality(clampingModality) {

    if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
        stimulusAbbrName = "V";

    } else {
        stimulusAbbrName = "I";
    }

    this->setDragDropMode(QAbstractItemView::InternalMove);
    this->setIconSize(QSize(30, 30));
    connect(this, &ProtocolItemDropList::itemDoubleClicked, this, &ProtocolItemDropList::onItemDoubleClicked);
    items = new QVector <ProtocolDropItem *>;
    ctrlItems = new QVector <ProtocolDropControlItem *>;
    voltageCtrlItems = new QVector <ProtocolDropControlItem *>;
    currentCtrlItems = new QVector <ProtocolDropControlItem *>;
    timeCtrlItems = new QVector <ProtocolDropControlItem *>;
    frequencyCtrlItems = new QVector <ProtocolDropControlItem *>;
    naturalNumCtrlItems = new QVector <ProtocolDropControlItem *>;
    analysisItems = new QVector <ProtocolDropAnalysisItem *> (PTD_PROTOCOL_ANALYSIS_NUM);
    for (int analysisIdx = 0; analysisIdx < PTD_PROTOCOL_ANALYSIS_NUM; analysisIdx++) {
        analysisItems->replace(analysisIdx, nullptr);
    }

    this->installEventFilter(this);
    this->createActions();
}

ProtocolItemDropList::~ProtocolItemDropList() {
    for (int itemIdx = 0; itemIdx < this->count(); itemIdx++) {
        ProtocolDropItem * item = static_cast <ProtocolDropItem *> (this->item(itemIdx));
        delete item;
    }
}

bool ProtocolItemDropList::importEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus, int voltageRangeIdx, int currentRangeIdx) {
    QString tag;
    int depth = EPML_PROTOCOL_ITEM_DEPTH;

    /*! Get the simulus range and set it before importing numeric values into the items */
    e4gcl::RangedMeasurement_t stimulusRange;
    if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
        commLib->getVoltageProtocolRange((unsigned int)voltageRangeIdx, stimulusRange);

    } else {
        commLib->getCurrentProtocolRange((unsigned int)currentRangeIdx, stimulusRange);
    }

    while (epmlManager->getNextAny(tag, depth, parentTag, epmlStatus)) {
        ProtocolDropItem * item = nullptr;

        if (tag == stimulusAbbrName.toLower() + "steptstep") {
            if (commLib->hasProtocolStep() == e4gcl::Success) {
                if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                    item = new ProtocolDropVStepTStepItem(commLib, ctrlManager, holdEdit->value());

                } else {
                    item = new ProtocolDropIStepTStepItem(commLib, ctrlManager, holdEdit->value());
                }

            } else {
                return false;
            }

        } else if (tag == stimulusAbbrName.toLower() + "step") {
            if (commLib->hasProtocolStep() == e4gcl::Success) {
                if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                    item = new ProtocolDropVStepItem(commLib, ctrlManager, holdEdit->value());

                } else {
                    item = new ProtocolDropIStepItem(commLib, ctrlManager, holdEdit->value());
                }

            } else {
                return false;
            }

        } else if (tag == "tstep") {
            if (commLib->hasProtocolStep() == e4gcl::Success) {
                if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                    item = new ProtocolDropVTStepItem(commLib, ctrlManager, holdEdit->value());

                } else {
                    item = new ProtocolDropITStepItem(commLib, ctrlManager, holdEdit->value());
                }

            } else {
                return false;
            }

        } else if (tag == stimulusAbbrName.toLower() + "const") {
            if (commLib->hasProtocolStep() == e4gcl::Success) {
                if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                    item = new ProtocolDropVConstItem(commLib, ctrlManager, holdEdit->value());

                } else {
                    item = new ProtocolDropIConstItem(commLib, ctrlManager, holdEdit->value());
                }

            } else {
                return false;
            }

        } else if (tag == stimulusAbbrName.toLower() + "hold") {
            if (commLib->hasProtocolStep() == e4gcl::Success) {
                if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                    item = new ProtocolDropVHoldItem(commLib, ctrlManager, holdEdit->value());

                } else {
                    item = new ProtocolDropIHoldItem(commLib, ctrlManager, holdEdit->value());
                }
                connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);

            } else {
                return false;
            }

        } else if (tag == "rest") {
            if (commLib->hasProtocolStep() == e4gcl::Success) {
                if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                    item = new ProtocolDropVRestItem(commLib, ctrlManager, holdEdit->value());

                } else {
                    item = new ProtocolDropIRestItem(commLib, ctrlManager, holdEdit->value());
                }

            } else {
                return false;
            }

        } else if (tag == stimulusAbbrName.toLower() + "ramp") {
            if (commLib->hasProtocolRamp() == e4gcl::Success) {
                if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                    item = new ProtocolDropVRampItem(commLib, ctrlManager, holdEdit->value());

                } else {
                    item = new ProtocolDropIRampItem(commLib, ctrlManager, holdEdit->value());
                }

            } else {
                return false;
            }

        } else if (tag == stimulusAbbrName.toLower() + "sin") {
            if (commLib->hasProtocolSin() == e4gcl::Success) {
                if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                    item = new ProtocolDropVSinItem(commLib, ctrlManager, holdEdit->value());

                } else {
                    item = new ProtocolDropISinItem(commLib, ctrlManager, holdEdit->value());
                }

            } else {
                return false;
            }

        } else if (tag == "repseqscaled") {
            if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                item = new ProtocolDropVRepSeqScaledItem(commLib, ctrlManager, holdEdit->value());

            } else {
                item = new ProtocolDropIRepSeqScaledItem(commLib, ctrlManager, holdEdit->value());
            }

        } else if (tag == "repseq") {
            if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                item = new ProtocolDropVRepSeqItem(commLib, ctrlManager, holdEdit->value());

            } else {
                item = new ProtocolDropIRepSeqItem(commLib, ctrlManager, holdEdit->value());
            }
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);

        } else if (tag == "repseqwithsteps") {
            if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                item = new ProtocolDropVRepSeqWithStepsItem(commLib, ctrlManager, holdEdit->value());

            } else {
                item = new ProtocolDropIRepSeqWithStepsItem(commLib, ctrlManager, holdEdit->value());
            }
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);

        } else if (tag == "infrepseq") {
            if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                item = new ProtocolDropVInfRepSeqItem(commLib, ctrlManager, holdEdit->value());

            } else {
                item = new ProtocolDropIInfRepSeqItem(commLib, ctrlManager, holdEdit->value());
            }
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);

        } else if (tag == "voltagectrl") {
            item = new ProtocolDropVoltageControlItem(commLib, ctrlManager, holdEdit->value());

        } else if (tag == "currentctrl") {
            item = new ProtocolDropCurrentControlItem(commLib, ctrlManager, holdEdit->value());

        } else if (tag == "timectrl") {
            item = new ProtocolDropTimeControlItem(commLib, ctrlManager, holdEdit->value());

        } else if (tag == "frequencyctrl") {
            item = new ProtocolDropFrequencyControlItem(commLib, ctrlManager, holdEdit->value());

        } else if (tag == "naturalnumctrl") {
            item = new ProtocolDropNaturalNumControlItem(commLib, ctrlManager, holdEdit->value());

        } else if (tag == "noisereport") {
            if (!(this->analysisRequested(ProtocolConsumerNoiseReport))) {
                item = new ProtocolDropNoiseReportItem(commLib, ctrlManager, holdEdit->value());
                ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                this->setAnalysis(ProtocolConsumerNoiseReport, castItem);
                emit requestCursors(castItem);

                connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                emit analysisChanged();
            }

        } else if (tag == "histogram") {
            if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                if (!(this->analysisRequested(ProtocolConsumerHistogram))) {
                    item = new ProtocolDropHistogramItem(commLib, ctrlManager, holdEdit->value());
                    ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                    this->setAnalysis(ProtocolConsumerHistogram, castItem);
                    emit requestCursors(castItem);

                    connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                    emit analysisChanged();
                }

            } else {
                /*! \todo FCON da implementare se serve una analisi diversa in current clamp */
            }

        } else if (tag == "spectrum") {
            if (!(this->analysisRequested(ProtocolConsumerSpectrum))) {
                item = new ProtocolDropSpectrumItem(commLib, ctrlManager, holdEdit->value());
                ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                this->setAnalysis(ProtocolConsumerSpectrum, castItem);
                emit requestCursors(castItem);

                connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                emit analysisChanged();
            }

        } else if (tag == "resistanceestimation") {
            if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                /*! \todo FCON da implementare se serve una analisi diversa in voltage clamp */

            } else {
                if (!(this->analysisRequested(ProtocolConsumerResistanceEstimation))) {
                    item = new ProtocolDropResistanceEstimationItem(commLib, ctrlManager, holdEdit->value());
                    ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                    this->setAnalysis(ProtocolConsumerResistanceEstimation, castItem);
                    emit requestCursors(castItem);

                    connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                    emit analysisChanged();
                }
            }

        } else if (tag == "membranetest") {
            if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                if (!(this->analysisRequested(ProtocolConsumerMembraneTest))) {
                    item = new ProtocolDropMembraneTestItem(commLib, ctrlManager, holdEdit->value());
                    ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                    this->setAnalysis(ProtocolConsumerMembraneTest, castItem);
                    emit requestCursors(castItem);

                    connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                    emit analysisChanged();
                }

            } else {
                /*! \todo FCON da implementare se serve una analisi diversa in current clamp */
            }

        } else if (tag == "ivgraph") {
            if (!(this->analysisRequested(ProtocolConsumerIvGraph))) {
                item = new ProtocolDropIvGraphItem(commLib, ctrlManager, holdEdit->value());
                ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                this->setAnalysis(ProtocolConsumerIvGraph, castItem);
                emit requestCursors(castItem);

                connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                emit analysisChanged();
            }

        } else if (tag == "voltagetracking") {
            if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                /*! \todo FCON da implementare se serve una analisi diversa in voltage clamp */

            } else {
                if (!(this->analysisRequested(ProtocolConsumerVoltageTracking))) {
                    item = new ProtocolDropVoltageTrackingItem(commLib, ctrlManager, holdEdit->value());
                    ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                    this->setAnalysis(ProtocolConsumerVoltageTracking, castItem);
                    emit requestCursors(castItem);

                    connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                    emit analysisChanged();
                }
            }

        } else if (tag == "apthreshold") {
            if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                /*! \todo FCON da implementare se serve una analisi diversa in voltage clamp */

            } else {
                if (!(this->analysisRequested(ProtocolConsumerApThreshold))) {
                    item = new ProtocolDropApThresholdItem(commLib, ctrlManager, holdEdit->value());
                    ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                    this->setAnalysis(ProtocolConsumerApThreshold, castItem);
                    emit requestCursors(castItem);

                    connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                    emit analysisChanged();
                }
            }

        } else if (tag == "apstatistics") {
            if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                /*! \todo FCON da implementare se serve una analisi diversa in voltage clamp */

            } else {
                if (!(this->analysisRequested(ProtocolConsumerApStatistics))) {
                    item = new ProtocolDropApStatisticsItem(commLib, ctrlManager, holdEdit->value());
                    ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                    this->setAnalysis(ProtocolConsumerApStatistics, castItem);
                    emit requestCursors(castItem);

                    connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                    emit analysisChanged();
                }
            }
        }

        if (item != nullptr) {
            item->setStimulusRange(stimulusRange);
            item->importEpml(epmlManager, tag, epmlStatus);

            this->addItem(item);
            connect(item, &ProtocolDropItem::updateItem, this, &ProtocolItemDropList::onUpdateItem);
            connect(item, &ProtocolDropItem::protocolDropItemDeleteRequest, this, &ProtocolItemDropList::onDropItemDelete);
        }
    }

    if ((epmlStatus == EpmlParentTagFound) || (epmlStatus == EpmlEof)) {
        return true;

    } else {
        return false;
    }
}

bool ProtocolItemDropList::exportEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus) {
    QString tag;
    int depth = EPML_PROTOCOL_ITEM_DEPTH;

    this->updateDropItemsList();

    for (int itemIdx = 0; itemIdx < items->size(); itemIdx++) {
        ProtocolDropItem * item = items->at(itemIdx);

        switch (item->type()) {
        case PROT_DROP_LIST_VSTEP_TSTEP_ITEM_TYPE:
            tag = "vsteptstep";
            break;

        case PROT_DROP_LIST_ISTEP_TSTEP_ITEM_TYPE:
            tag = "isteptstep";
            break;

        case PROT_DROP_LIST_VSTEP_ITEM_TYPE:
            tag = "vstep";
            break;

        case PROT_DROP_LIST_ISTEP_ITEM_TYPE:
            tag = "istep";
            break;

        case PROT_DROP_LIST_VTSTEP_ITEM_TYPE:
        case PROT_DROP_LIST_ITSTEP_ITEM_TYPE:
            tag = "tstep";
            break;

        case PROT_DROP_LIST_VCONST_ITEM_TYPE:
            tag = "vconst";
            break;

        case PROT_DROP_LIST_ICONST_ITEM_TYPE:
            tag = "iconst";
            break;

        case PROT_DROP_LIST_VHOLD_ITEM_TYPE:
            tag = "vhold";
            break;

        case PROT_DROP_LIST_IHOLD_ITEM_TYPE:
            tag = "ihold";
            break;

        case PROT_DROP_LIST_VREST_ITEM_TYPE:
        case PROT_DROP_LIST_IREST_ITEM_TYPE:
            tag = "rest";
            break;

        case PROT_DROP_LIST_VRAMP_ITEM_TYPE:
            tag = "vramp";
            break;

        case PROT_DROP_LIST_IRAMP_ITEM_TYPE:
            tag = "iramp";
            break;

        case PROT_DROP_LIST_VSIN_ITEM_TYPE:
            tag = "vsin";
            break;

        case PROT_DROP_LIST_ISIN_ITEM_TYPE:
            tag = "isin";
            break;

        case PROT_DROP_LIST_VREP_SEQ_SCALED_ITEM_TYPE:
        case PROT_DROP_LIST_IREP_SEQ_SCALED_ITEM_TYPE:
            tag = "repseqscaled";
            break;

        case PROT_DROP_LIST_VREP_SEQ_ITEM_TYPE:
        case PROT_DROP_LIST_IREP_SEQ_ITEM_TYPE:
            tag = "repseq";
            break;

        case PROT_DROP_LIST_VREP_SEQ_WITH_STEPS_ITEM_TYPE:
        case PROT_DROP_LIST_IREP_SEQ_WITH_STEPS_ITEM_TYPE:
            tag = "repseqwithsteps";
            break;

        case PROT_DROP_LIST_VINF_REP_SEQ_ITEM_TYPE:
        case PROT_DROP_LIST_IINF_REP_SEQ_ITEM_TYPE:
            tag = "infrepseq";
            break;

        case PROT_DROP_LIST_VOLTAGE_CONTROL_ITEM_TYPE:
            tag = "voltagectrl";
            break;

        case PROT_DROP_LIST_CURRENT_CONTROL_ITEM_TYPE:
            tag = "currentctrl";
            break;

        case PROT_DROP_LIST_TIME_CONTROL_ITEM_TYPE:
            tag = "timectrl";
            break;

        case PROT_DROP_LIST_FREQUENCY_CONTROL_ITEM_TYPE:
            tag = "frequencyctrl";
            break;

        case PROT_DROP_LIST_NATURAL_NUM_CONTROL_ITEM_TYPE:
            tag = "naturalnumctrl";
            break;

        case PROT_DROP_LIST_NOISE_REPORT_ITEM_TYPE:
            tag = "noisereport";
            break;

        case PROT_DROP_LIST_HISTOGRAM_ITEM_TYPE:
            tag = "histogram";
            break;

        case PROT_DROP_LIST_SPECTRUM_ITEM_TYPE:
            tag = "spectrum";
            break;

        case PROT_DROP_LIST_RESISTANCE_ESTIMATION_ITEM_TYPE:
            tag = "resistanceestimation";
            break;

        case PROT_DROP_LIST_MEMBRANE_TEST_ITEM_TYPE:
            tag = "membranetest";
            break;

        case PROT_DROP_LIST_IV_GRAPH_ITEM_TYPE:
            tag = "ivgraph";
            break;

        case PROT_DROP_LIST_VOLTAGE_TRACKING_ITEM_TYPE:
            tag = "voltagetracking";
            break;

        case PROT_DROP_LIST_AP_THRESHOLD_ITEM_TYPE:
            tag = "apthreshold";
            break;

        case PROT_DROP_LIST_AP_STATISTICS_ITEM_TYPE:
            tag = "apstatistics";
            break;
        }

        if (!(epmlManager->createSection(tag, depth, parentTag, epmlStatus))) {
            /*! \todo FCON gestire l'uscita */
            return false;
        }

        if (!(item->exportEpml(epmlManager, epmlStatus))) {
            /*! \todo FCON gestire l'uscita */
            return false;
        }
    }
    return true;
}

void ProtocolItemDropList::setCtrlManager(ProtocolItemCtrlManager * cm) {
    ctrlManager = cm;
}

void ProtocolItemDropList::onUpdateHold(double value) {
    emit updateHold(value);
}

QVector <ProtocolDropItem *> * ProtocolItemDropList::getDropItems() {
    this->updateDropItemsList();
    return items;
}

QVector <ProtocolDropControlItem *> * ProtocolItemDropList::getCtrlItems() {
    this->updateCtrlItemsList();
    return ctrlItems;
}

QVector <ProtocolDropControlItem *> * ProtocolItemDropList::getVoltageCtrlItems() {
    this->updateVoltageCtrlItemsList();
    return voltageCtrlItems;
}

QVector <ProtocolDropControlItem *> * ProtocolItemDropList::getCurrentCtrlItems() {
    this->updateCurrentCtrlItemsList();
    return currentCtrlItems;
}

QVector <ProtocolDropControlItem *> * ProtocolItemDropList::getTimeCtrlItems() {
    this->updateTimeCtrlItemsList();
    return timeCtrlItems;
}

QVector <ProtocolDropControlItem *> * ProtocolItemDropList::getFrequencyCtrlItems() {
    this->updateFrequencyCtrlItemsList();
    return frequencyCtrlItems;
}

QVector <ProtocolDropControlItem *> * ProtocolItemDropList::getNaturalNumCtrlItems() {
    this->updateNaturalNumCtrlItemsList();
    return naturalNumCtrlItems;
}

bool ProtocolItemDropList::analysisRequested(ProtocolConsumerType_t consumerType) {
    return (analysisItems->at(consumerType-PTD_PROTOCOL_ANALYSIS_OFFSET) != nullptr);
}

bool ProtocolItemDropList::analysisValid(ProtocolConsumerType_t consumerType) {
    ProtocolDropAnalysisItem * analysis = analysisItems->at(consumerType-PTD_PROTOCOL_ANALYSIS_OFFSET);
    if (analysis != nullptr) {
        return (analysis->onCheckCursorsValidity() == "OK");

    } else {
        return false;
    }
}

QVector <int> ProtocolItemDropList::getAnalysisCursorsMapping(ProtocolConsumerType_t consumerType) {
    return analysisItems->at(consumerType-PTD_PROTOCOL_ANALYSIS_OFFSET)->getCursorMapping();
}

void ProtocolItemDropList::setStimulusRange(e4gcl::RangedMeasurement_t &range) {
    for (int itemIdx = 0; itemIdx < items->size(); itemIdx++) {
        items->at(itemIdx)->setStimulusRange(range);
    }

    this->updateCtrlItemsList();
    for (int itemIdx = 0; itemIdx < ctrlItems->size(); itemIdx++) {
        ctrlItems->at(itemIdx)->setStimulusRange(range);
    }
}

void ProtocolItemDropList::onItemDoubleClicked(QListWidgetItem * item) {
    static_cast <ProtocolDropItem *> (item)->openPropertyDialog();
}

void ProtocolItemDropList::onUpdateItem() {
    emit updateProtocol();
}

void ProtocolItemDropList::onDropItemDelete(ProtocolDropItem * item) {
    if (item != nullptr) {
        this->manageItemDelete(item);
        this->manageCtrlDelete(item);
        this->manageAnalysisDelete(item);

        delete item;
        item = nullptr;
        emit updateProtocol();
    }
}

void ProtocolItemDropList::updateDropItemsList() {
    items->clear();
    for (int itemIdx = 0; itemIdx < this->count(); itemIdx++) {
        ProtocolDropItem * item = static_cast <ProtocolDropItem *> (this->item(itemIdx));
        items->push_back(item);
    }
}

void ProtocolItemDropList::updateCtrlItemsList() {
    ctrlItems->clear();
    for (int itemIdx = 0; itemIdx < this->count(); itemIdx++) {
        ProtocolDropControlItem * item = dynamic_cast <ProtocolDropControlItem *> (this->item(itemIdx));
        if (item != nullptr) {
            if (item->getItemCtrlType() != ProtocolItemCtrlNone) {
                ctrlItems->push_back(item);
            }
        }
    }
}

void ProtocolItemDropList::updateVoltageCtrlItemsList() {
    voltageCtrlItems->clear();
    for (int itemIdx = 0; itemIdx < this->count(); itemIdx++) {
        ProtocolDropControlItem * item = dynamic_cast <ProtocolDropControlItem *> (this->item(itemIdx));
        if (item != nullptr) {
            if (item->getItemCtrlType() == ProtocolItemCtrlVoltage) {
                voltageCtrlItems->push_back(item);
            }
        }
    }
}

void ProtocolItemDropList::updateCurrentCtrlItemsList() {
    currentCtrlItems->clear();
    for (int itemIdx = 0; itemIdx < this->count(); itemIdx++) {
        ProtocolDropControlItem * item = dynamic_cast <ProtocolDropControlItem *> (this->item(itemIdx));
        if (item != nullptr) {
            if (item->getItemCtrlType() == ProtocolItemCtrlCurrent) {
                currentCtrlItems->push_back(item);
            }
        }
    }
}

void ProtocolItemDropList::updateTimeCtrlItemsList() {
    timeCtrlItems->clear();
    for (int itemIdx = 0; itemIdx < this->count(); itemIdx++) {
        ProtocolDropControlItem * item = dynamic_cast <ProtocolDropControlItem *> (this->item(itemIdx));
        if (item != nullptr) {
            if (item->getItemCtrlType() == ProtocolItemCtrlTime) {
                timeCtrlItems->push_back(item);
            }
        }
    }
}

void ProtocolItemDropList::updateFrequencyCtrlItemsList() {
    frequencyCtrlItems->clear();
    for (int itemIdx = 0; itemIdx < this->count(); itemIdx++) {
        ProtocolDropControlItem * item = dynamic_cast <ProtocolDropControlItem *> (this->item(itemIdx));
        if (item != nullptr) {
            if (item->getItemCtrlType() == ProtocolItemCtrlFrequency) {
                frequencyCtrlItems->push_back(item);
            }
        }
    }
}

void ProtocolItemDropList::updateNaturalNumCtrlItemsList() {
    naturalNumCtrlItems->clear();
    for (int itemIdx = 0; itemIdx < this->count(); itemIdx++) {
        ProtocolDropControlItem * item = dynamic_cast <ProtocolDropControlItem *> (this->item(itemIdx));
        if (item != nullptr) {
            if (item->getItemCtrlType() == ProtocolItemCtrlNaturalNum) {
                naturalNumCtrlItems->push_back(item);
            }
        }
    }
}

void ProtocolItemDropList::setAnalysis(ProtocolConsumerType_t consumerType, ProtocolDropAnalysisItem * item) {
    analysisItems->replace(consumerType-PTD_PROTOCOL_ANALYSIS_OFFSET, item);
}

void ProtocolItemDropList::resetAnalysis(ProtocolConsumerType_t consumerType) {
    this->setAnalysis(consumerType, nullptr);
}

void ProtocolItemDropList::dragEnterEvent(QDragEnterEvent * event) {
    if (event->source() == this) {
        event->acceptProposedAction();

    } else if (this->acceptedMimeDataFormat(event->mimeData())) {
        event->acceptProposedAction();

    } else {
        event->ignore();
    }
}

void ProtocolItemDropList::dragMoveEvent(QDragMoveEvent * event) {
    if (event->source() == this) {
        event->acceptProposedAction();

    } else if (this->acceptedMimeDataFormat(event->mimeData())) {
        event->acceptProposedAction();

    } else {
        event->ignore();
    }
}

void ProtocolItemDropList::dropEvent(QDropEvent * event) {
    if (event->source() == this) {
        QListWidget::dropEvent(event);
        this->onUpdateItem();

    } else if (this->acceptedMimeDataFormat(event->mimeData())) {
        const QMimeData * mime = event->mimeData();
        QByteArray itemData = mime->data(PROT_DRAG_LIST_WIDGET_ITEM_MIME_FORMAT);
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);
        int type = 0;

        dataStream >> type;

        ProtocolDropItem * item = nullptr;

        switch (type) {
        case PROT_DRAG_LIST_VSTEP_TSTEP_ITEM_TYPE:
            item = new ProtocolDropVStepTStepItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_ISTEP_TSTEP_ITEM_TYPE:
            item = new ProtocolDropIStepTStepItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VSTEP_ITEM_TYPE:
            item = new ProtocolDropVStepItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_ISTEP_ITEM_TYPE:
            item = new ProtocolDropIStepItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VTSTEP_ITEM_TYPE:
            item = new ProtocolDropVTStepItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_ITSTEP_ITEM_TYPE:
            item = new ProtocolDropITStepItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VCONST_ITEM_TYPE:
            item = new ProtocolDropVConstItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_ICONST_ITEM_TYPE:
            item = new ProtocolDropIConstItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VHOLD_ITEM_TYPE:
            item = new ProtocolDropVHoldItem(commLib, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_IHOLD_ITEM_TYPE:
            item = new ProtocolDropIHoldItem(commLib, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_VREST_ITEM_TYPE:
            item = new ProtocolDropVRestItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_IREST_ITEM_TYPE:
            item = new ProtocolDropIRestItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VRAMP_ITEM_TYPE:
            item = new ProtocolDropVRampItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_IRAMP_ITEM_TYPE:
            item = new ProtocolDropIRampItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VSIN_ITEM_TYPE:
            item = new ProtocolDropVSinItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_ISIN_ITEM_TYPE:
            item = new ProtocolDropISinItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VREP_SEQ_SCALED_ITEM_TYPE:
            item = new ProtocolDropVRepSeqScaledItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_IREP_SEQ_SCALED_ITEM_TYPE:
            item = new ProtocolDropIRepSeqScaledItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VREP_SEQ_ITEM_TYPE:
            item = new ProtocolDropVRepSeqItem(commLib, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_IREP_SEQ_ITEM_TYPE:
            item = new ProtocolDropIRepSeqItem(commLib, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_VREP_SEQ_WITH_STEPS_ITEM_TYPE:
            item = new ProtocolDropVRepSeqWithStepsItem(commLib, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_IREP_SEQ_WITH_STEPS_ITEM_TYPE:
            item = new ProtocolDropIRepSeqWithStepsItem(commLib, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_VINF_REP_SEQ_ITEM_TYPE:
            item = new ProtocolDropVInfRepSeqItem(commLib, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_IINF_REP_SEQ_ITEM_TYPE:
            item = new ProtocolDropIInfRepSeqItem(commLib, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_VOLTAGE_CONTROL_ITEM_TYPE:
            item = new ProtocolDropVoltageControlItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_CURRENT_CONTROL_ITEM_TYPE:
            item = new ProtocolDropCurrentControlItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_TIME_CONTROL_ITEM_TYPE:
            item = new ProtocolDropTimeControlItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_FREQUENCY_CONTROL_ITEM_TYPE:
            item = new ProtocolDropFrequencyControlItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_NATURAL_NUM_CONTROL_ITEM_TYPE:
            item = new ProtocolDropNaturalNumControlItem(commLib, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_NOISE_REPORT_ITEM_TYPE:
            if (!(this->analysisRequested(ProtocolConsumerNoiseReport))) {
                item = new ProtocolDropNoiseReportItem(commLib, ctrlManager, holdEdit->value());
                ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                this->setAnalysis(ProtocolConsumerNoiseReport, castItem);
                emit requestCursors(castItem);
                connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                emit analysisChanged();

            } else {
                event->ignore();
            }
            break;

        case PROT_DRAG_LIST_HISTOGRAM_ITEM_TYPE:
            if (!(this->analysisRequested(ProtocolConsumerHistogram))) {
                item = new ProtocolDropHistogramItem(commLib, ctrlManager, holdEdit->value());
                ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                this->setAnalysis(ProtocolConsumerHistogram, castItem);
                emit requestCursors(castItem);
                connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                emit analysisChanged();

            } else {
                event->ignore();
            }
            break;

        case PROT_DRAG_LIST_SPECTRUM_ITEM_TYPE:
            if (!(this->analysisRequested(ProtocolConsumerSpectrum))) {
                item = new ProtocolDropSpectrumItem(commLib, ctrlManager, holdEdit->value());
                ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                this->setAnalysis(ProtocolConsumerSpectrum, castItem);
                emit requestCursors(castItem);
                connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                emit analysisChanged();

            } else {
                event->ignore();
            }
            break;

        case PROT_DRAG_LIST_RESISTANCE_ESTIMATION_ITEM_TYPE:
            if (!(this->analysisRequested(ProtocolConsumerResistanceEstimation))) {
                item = new ProtocolDropResistanceEstimationItem(commLib, ctrlManager, holdEdit->value());
                ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                this->setAnalysis(ProtocolConsumerResistanceEstimation, castItem);
                emit requestCursors(castItem);
                connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                emit analysisChanged();

            } else {
                event->ignore();
            }
            break;

        case PROT_DRAG_LIST_MEMBRANE_TEST_ITEM_TYPE:
            if (!(this->analysisRequested(ProtocolConsumerMembraneTest))) {
                item = new ProtocolDropMembraneTestItem(commLib, ctrlManager, holdEdit->value());
                ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                this->setAnalysis(ProtocolConsumerMembraneTest, castItem);
                emit requestCursors(castItem);
                connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                emit analysisChanged();

            } else {
                event->ignore();
            }
            break;

        case PROT_DRAG_LIST_IV_GRAPH_ITEM_TYPE:
            if (!(this->analysisRequested(ProtocolConsumerIvGraph))) {
                item = new ProtocolDropIvGraphItem(commLib, ctrlManager, holdEdit->value());
                ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                this->setAnalysis(ProtocolConsumerIvGraph, castItem);
                emit requestCursors(castItem);
                connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                emit analysisChanged();

            } else {
                event->ignore();
            }
            break;

        case PROT_DRAG_LIST_VOLTAGE_TRACKING_ITEM_TYPE:
            if (!(this->analysisRequested(ProtocolConsumerVoltageTracking))) {
                item = new ProtocolDropVoltageTrackingItem(commLib, ctrlManager, holdEdit->value());
                ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                this->setAnalysis(ProtocolConsumerVoltageTracking, castItem);
                emit requestCursors(castItem);
                connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                emit analysisChanged();

            } else {
                event->ignore();
            }
            break;

        case PROT_DRAG_LIST_AP_THRESHOLD_ITEM_TYPE:
            if (!(this->analysisRequested(ProtocolConsumerApThreshold))) {
                item = new ProtocolDropApThresholdItem(commLib, ctrlManager, holdEdit->value());
                ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                this->setAnalysis(ProtocolConsumerApThreshold, castItem);
                emit requestCursors(castItem);
                connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                emit analysisChanged();

            } else {
                event->ignore();
            }
            break;

        case PROT_DRAG_LIST_AP_STATISTICS_ITEM_TYPE:
            if (!(this->analysisRequested(ProtocolConsumerApStatistics))) {
                item = new ProtocolDropApStatisticsItem(commLib, ctrlManager, holdEdit->value());
                ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                this->setAnalysis(ProtocolConsumerApStatistics, castItem);
                emit requestCursors(castItem);
                connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                emit analysisChanged();

            } else {
                event->ignore();
            }
            break;

        case PROT_DRAG_LIST_SEPARATOR_TYPE:
            event->ignore();
            return;
        }

        if (item != nullptr) {
            this->addItem(item);
            connect(item, &ProtocolDropItem::updateItem, this, &ProtocolItemDropList::onUpdateItem);
            connect(item, &ProtocolDropItem::protocolDropItemDeleteRequest, this, &ProtocolItemDropList::onDropItemDelete);
            this->onUpdateItem();
        }

    } else {
        event->ignore();
    }
}

bool ProtocolItemDropList::eventFilter(QObject * obj, QEvent * event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent * keyEvent = static_cast <QKeyEvent *> (event);
        if (keyEvent->key() == Qt::Key_Delete) {
            ProtocolDropItem * item = dynamic_cast <ProtocolDropItem *> (this->currentItem());
            this->onDropItemDelete(item);
        }
    }
    return QObject::eventFilter(obj, event);
}

void ProtocolItemDropList::contextMenuEvent(QContextMenuEvent * event) {
    QMenu menu(this);
    menu.addAction(editAct);
    menu.addSeparator();
    menu.addAction(deleteAct);
    menu.exec(event->globalPos());
}

void ProtocolItemDropList::manageItemDelete(ProtocolDropItem * item) {
    if (item != nullptr) {
        switch (item->type()) {
        case PROT_DROP_LIST_VSTEP_TSTEP_ITEM_TYPE:
        case PROT_DROP_LIST_ISTEP_TSTEP_ITEM_TYPE:
        case PROT_DROP_LIST_VSTEP_ITEM_TYPE:
        case PROT_DROP_LIST_ISTEP_ITEM_TYPE:
        case PROT_DROP_LIST_VTSTEP_ITEM_TYPE:
        case PROT_DROP_LIST_ITSTEP_ITEM_TYPE:
        case PROT_DROP_LIST_VCONST_ITEM_TYPE:
        case PROT_DROP_LIST_ICONST_ITEM_TYPE:
        case PROT_DROP_LIST_VHOLD_ITEM_TYPE:
        case PROT_DROP_LIST_IHOLD_ITEM_TYPE:
        case PROT_DROP_LIST_VREST_ITEM_TYPE:
        case PROT_DROP_LIST_IREST_ITEM_TYPE: {
            ProtocolDropXStepTStepItem * castItem = static_cast <ProtocolDropXStepTStepItem *> (item);
            ctrlManager->manageDeletedXStepTStep(castItem);
            break;
        }

        case PROT_DROP_LIST_VRAMP_ITEM_TYPE:
        case PROT_DROP_LIST_IRAMP_ITEM_TYPE: {
            ProtocolDropXRampItem * castItem = static_cast <ProtocolDropXRampItem *> (item);
            ctrlManager->manageDeletedXRamp(castItem);
            break;
        }

        case PROT_DROP_LIST_VSIN_ITEM_TYPE:
        case PROT_DROP_LIST_ISIN_ITEM_TYPE: {
            ProtocolDropXSinItem * castItem = static_cast <ProtocolDropXSinItem *> (item);
            ctrlManager->manageDeletedXSin(castItem);
            break;
        }

        case PROT_DROP_LIST_VREP_SEQ_SCALED_ITEM_TYPE:
        case PROT_DROP_LIST_IREP_SEQ_SCALED_ITEM_TYPE:
        case PROT_DROP_LIST_VREP_SEQ_ITEM_TYPE:
        case PROT_DROP_LIST_IREP_SEQ_ITEM_TYPE:
        case PROT_DROP_LIST_VREP_SEQ_WITH_STEPS_ITEM_TYPE:
        case PROT_DROP_LIST_IREP_SEQ_WITH_STEPS_ITEM_TYPE:
        case PROT_DROP_LIST_VINF_REP_SEQ_ITEM_TYPE:
        case PROT_DROP_LIST_IINF_REP_SEQ_ITEM_TYPE: {
            ProtocolDropXRepSeqScaledItem * castItem = static_cast <ProtocolDropXRepSeqScaledItem *> (item);
            ctrlManager->manageDeletedLoop(castItem);
            break;
        }

        default:
            break;
        }
    }
}

void ProtocolItemDropList::manageCtrlDelete(ProtocolDropItem * item) {
    ProtocolDropControlItem * ctrlItem = dynamic_cast <ProtocolDropControlItem *> (item);
    if (ctrlItem != nullptr) {
        switch (ctrlItem->getItemCtrlType()) {
        case ProtocolItemCtrlVoltage:
            ctrlManager->manageDeletedVoltageCtrl(ctrlItem);
            break;

        case ProtocolItemCtrlCurrent:
            ctrlManager->manageDeletedCurrentCtrl(ctrlItem);
            break;

        case ProtocolItemCtrlTime:
            ctrlManager->manageDeletedTimeCtrl(ctrlItem);
            break;

        case ProtocolItemCtrlFrequency:
            ctrlManager->manageDeletedFrequencyCtrl(ctrlItem);
            break;

        case ProtocolItemCtrlNaturalNum:
            ctrlManager->manageDeletedNaturalNumCtrl(ctrlItem);
            break;

        case ProtocolItemCtrlNone:
            GLB_HERE
            break;
        }
    }
}

void ProtocolItemDropList::manageAnalysisDelete(ProtocolDropItem * item) {
    ProtocolDropAnalysisItem * analysisItem = dynamic_cast <ProtocolDropAnalysisItem *> (item);
    if (analysisItem != nullptr) {
        this->resetAnalysis(analysisItem->getConsumerType());
        emit analysisChanged();
    }
}

void ProtocolItemDropList::createActions() {
    editAct = new QAction("Edit item", this);
    editAct->setIcon(QIcon(":/imgs/edit protocol.png"));
    connect(editAct, &QAction::triggered, this, [=] () {
        this->onItemDoubleClicked(this->currentItem());
    });

    deleteAct = new QAction("Delete item", this);
    deleteAct->setIcon(QIcon(":/imgs/remove protocol.png"));
    connect(deleteAct, &QAction::triggered, this, [=] () {
        this->onDropItemDelete(static_cast <ProtocolDropItem *> (this->currentItem()));
    });
}

GapfreeProtocolItemDropList::GapfreeProtocolItemDropList(e4gcl::CommLib * commLib, QDoubleSpinBox * holdEdit, int clampingModality) :
    ProtocolItemDropList(commLib, holdEdit, clampingModality) {

}

bool GapfreeProtocolItemDropList::acceptedMimeDataFormat(const QMimeData * mimeData) {
    if (mimeData->hasFormat(PROT_DRAG_LIST_VSTEP_TSTEP_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_ISTEP_TSTEP_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VSTEP_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_ISTEP_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VTSTEP_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_ITSTEP_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VCONST_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_ICONST_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VHOLD_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_IHOLD_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VREST_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_IREST_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VRAMP_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_IRAMP_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VSIN_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_ISIN_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VREP_SEQ_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_IREP_SEQ_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VREP_SEQ_WITH_STEPS_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_IREP_SEQ_WITH_STEPS_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VINF_REP_SEQ_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_IINF_REP_SEQ_ITEM_MIME_FORMAT)) {
        return true;

    } else {
        return false;
    }
}

EpisodicProtocolItemDropList::EpisodicProtocolItemDropList(e4gcl::CommLib * commLib, QDoubleSpinBox * holdEdit, int clampingModality) :
    ProtocolItemDropList(commLib, holdEdit, clampingModality) {

}

bool EpisodicProtocolItemDropList::acceptedMimeDataFormat(const QMimeData * mimeData) {
    if (mimeData->hasFormat(PROT_DRAG_LIST_VSTEP_TSTEP_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_ISTEP_TSTEP_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VSTEP_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_ISTEP_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VTSTEP_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_ITSTEP_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VCONST_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_ICONST_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VHOLD_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_IHOLD_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VREST_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_IREST_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VRAMP_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_IRAMP_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VSIN_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_ISIN_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VREP_SEQ_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_IREP_SEQ_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VREP_SEQ_SCALED_ITEM_MIME_FORMAT)) {
        return true;

    } else {
        return false;
    }
}

CtrlProtocolItemDropList::CtrlProtocolItemDropList(e4gcl::CommLib * commLib, QDoubleSpinBox * holdEdit, int clampingModality) :
    ProtocolItemDropList(commLib, holdEdit, clampingModality) {

}

bool CtrlProtocolItemDropList::acceptedMimeDataFormat(const QMimeData * mimeData) {
    if (mimeData->hasFormat(PROT_DRAG_LIST_CONTROL_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_VOLTAGE_CONTROL_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_CURRENT_CONTROL_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_TIME_CONTROL_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_FREQUENCY_CONTROL_ITEM_MIME_FORMAT) ||
            mimeData->hasFormat(PROT_DRAG_LIST_NATURAL_NUM_CONTROL_ITEM_MIME_FORMAT)) {
        return true;

    } else {
        return false;
    }
}

AnalysisProtocolItemDropList::AnalysisProtocolItemDropList(e4gcl::CommLib * commLib, QDoubleSpinBox * holdEdit, int clampingModality) :
    ProtocolItemDropList(commLib, holdEdit, clampingModality) {

}

void AnalysisProtocolItemDropList::addCursors(QVector <ProtocolCursor *> * cursors) {
    for (int itemIdx = 0; itemIdx < analysisItems->size(); itemIdx++) {
        if (analysisItems->at(itemIdx) != nullptr) {
            analysisItems->at(itemIdx)->addCursors(cursors);
        }
    }
}

void AnalysisProtocolItemDropList::removeCursors(QVector <ProtocolCursor *> * cursors, QVector <int> cursorsMap) {
    for (int itemIdx = 0; itemIdx < analysisItems->size(); itemIdx++) {
        if (analysisItems->at(itemIdx) != nullptr) {
            analysisItems->at(itemIdx)->removeCursors(cursors, cursorsMap);
        }
    }
}

void AnalysisProtocolItemDropList::updateCursors() {
    for (int itemIdx = 0; itemIdx < analysisItems->size(); itemIdx++) {
        if (analysisItems->at(itemIdx) != nullptr) {
            analysisItems->at(itemIdx)->onAcceptPropertyDialog();
        }
    }
}

void AnalysisProtocolItemDropList::enableAnalysis(bool enabled) {
    for (int itemIdx = 0; itemIdx < analysisItems->size(); itemIdx++) {
        if (analysisItems->at(itemIdx) != nullptr) {
            analysisItems->at(itemIdx)->setEnabled(enabled);
        }
    }
}

bool AnalysisProtocolItemDropList::acceptedMimeDataFormat(const QMimeData * mimeData) {
    if (((mimeData->hasFormat(PROT_DRAG_LIST_NOISE_REPORT_ITEM_MIME_FORMAT)) && (!(this->analysisRequested(ProtocolConsumerNoiseReport)))) ||
            ((mimeData->hasFormat(PROT_DRAG_LIST_HISTOGRAM_ITEM_MIME_FORMAT)) && (!(this->analysisRequested(ProtocolConsumerHistogram)))) ||
            ((mimeData->hasFormat(PROT_DRAG_LIST_SPECTRUM_ITEM_MIME_FORMAT)) && (!(this->analysisRequested(ProtocolConsumerSpectrum)))) ||
            ((mimeData->hasFormat(PROT_DRAG_LIST_RESISTANCE_ESTIMATION_ITEM_MIME_FORMAT)) && (!(this->analysisRequested(ProtocolConsumerResistanceEstimation)))) ||
            ((mimeData->hasFormat(PROT_DRAG_LIST_MEMBRANE_TEST_ITEM_MIME_FORMAT)) && (!(this->analysisRequested(ProtocolConsumerMembraneTest)))) ||
            ((mimeData->hasFormat(PROT_DRAG_LIST_IV_GRAPH_ITEM_MIME_FORMAT)) && (!(this->analysisRequested(ProtocolConsumerIvGraph)))) ||
            ((mimeData->hasFormat(PROT_DRAG_LIST_VOLTAGE_TRACKING_ITEM_MIME_FORMAT)) && (!(this->analysisRequested(ProtocolConsumerVoltageTracking)))) ||
            ((mimeData->hasFormat(PROT_DRAG_LIST_AP_THRESHOLD_ITEM_MIME_FORMAT)) && (!(this->analysisRequested(ProtocolConsumerApThreshold)))) ||
            ((mimeData->hasFormat(PROT_DRAG_LIST_AP_STATISTICS_ITEM_MIME_FORMAT)) && (!(this->analysisRequested(ProtocolConsumerApStatistics))))) {
        return true;

    } else {
        return false;
    }
}
