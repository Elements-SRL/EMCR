#include "protocolitemdroplist.h"

#include <QtWidgets>

ProtocolItemDropList::ProtocolItemDropList(ModelDevice *  mDev, QDoubleSpinBox * holdEdit, ClampingModality_t clampingModality) :
    QListWidget(),
    mDev(mDev),
    holdEdit(holdEdit),
    clampingModality(clampingModality) {

    if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
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

void ProtocolItemDropList::setStimulusRange(RangedMeasurement_t &range) {
    for (int itemIdx = 0; itemIdx < items->size(); itemIdx++) {
        items->at(itemIdx)->setStimulusRange(range);
    }

    this->updateCtrlItemsList();
    for (int itemIdx = 0; itemIdx < ctrlItems->size(); itemIdx++) {
        ctrlItems->at(itemIdx)->setStimulusRange(range);
    }
}

std::vector <YAML::Control_t> ProtocolItemDropList::getYamlControls() {
    std::vector <YAML::Control_t> yamlControls;

    this->updateDropItemsList();

    for (int itemIdx = 0; itemIdx < items->size(); itemIdx++) {
        ProtocolDropItem * item = items->at(itemIdx);

        switch (item->type()) {
        case PROT_DROP_LIST_VOLTAGE_CONTROL_ITEM_TYPE:
            yamlControls.push_back(static_cast <ProtocolDropControlItem *> (item)->getYamlVoltageCtrl());
            break;

        case PROT_DROP_LIST_CURRENT_CONTROL_ITEM_TYPE:
            yamlControls.push_back(static_cast <ProtocolDropControlItem *> (item)->getYamlCurrentCtrl());
            break;

        case PROT_DROP_LIST_TIME_CONTROL_ITEM_TYPE:
            yamlControls.push_back(static_cast <ProtocolDropControlItem *> (item)->getYamlTimeCtrl());
            break;

        case PROT_DROP_LIST_FREQUENCY_CONTROL_ITEM_TYPE:
            yamlControls.push_back(static_cast <ProtocolDropControlItem *> (item)->getYamlFrequencyCtrl());
            break;

        case PROT_DROP_LIST_NATURAL_NUM_CONTROL_ITEM_TYPE:
            yamlControls.push_back(static_cast <ProtocolDropControlItem *> (item)->getYamlNaturalNumCtrl());
            break;

        default:
            break;
        }
    }
    return yamlControls;
}

std::vector <YAML::Phase_t> ProtocolItemDropList::getYamlPhases() {
    std::vector <YAML::Phase_t> yamlPhases;

    this->updateDropItemsList();

    for (int itemIdx = 0; itemIdx < items->size(); itemIdx++) {
        ProtocolDropItem * item = items->at(itemIdx);

        switch (item->type()) {
        case PROT_DROP_LIST_VSTEP_TSTEP_ITEM_TYPE:
            yamlPhases.push_back(static_cast <ProtocolDropXStepTStepItem *> (item)->getYamlVStepTStep());
            break;

        case PROT_DROP_LIST_ISTEP_TSTEP_ITEM_TYPE:
            yamlPhases.push_back(static_cast <ProtocolDropXStepTStepItem *> (item)->getYamlIStepTStep());
            break;

        case PROT_DROP_LIST_VCONST_ITEM_TYPE:
            yamlPhases.push_back(static_cast <ProtocolDropXStepTStepItem *> (item)->getYamlVConst());
            break;

        case PROT_DROP_LIST_ICONST_ITEM_TYPE:
            yamlPhases.push_back(static_cast <ProtocolDropXStepTStepItem *> (item)->getYamlIConst());
            break;

        case PROT_DROP_LIST_VHOLD_ITEM_TYPE:
            yamlPhases.push_back(static_cast <ProtocolDropXStepTStepItem *> (item)->getYamlVHold());
            break;

        case PROT_DROP_LIST_IHOLD_ITEM_TYPE:
            yamlPhases.push_back(static_cast <ProtocolDropXStepTStepItem *> (item)->getYamlIHold());
            break;

        case PROT_DROP_LIST_VREST_ITEM_TYPE:
            yamlPhases.push_back(static_cast <ProtocolDropXStepTStepItem *> (item)->getYamlVRest());
            break;

        case PROT_DROP_LIST_IREST_ITEM_TYPE:
            yamlPhases.push_back(static_cast <ProtocolDropXStepTStepItem *> (item)->getYamlIRest());
            break;

        case PROT_DROP_LIST_VRAMP_ITEM_TYPE:
            yamlPhases.push_back(static_cast <ProtocolDropXRampItem *> (item)->getYamlVRamp());
            break;

        case PROT_DROP_LIST_IRAMP_ITEM_TYPE:
            yamlPhases.push_back(static_cast <ProtocolDropXRampItem *> (item)->getYamlIRamp());
            break;

        case PROT_DROP_LIST_VSIN_ITEM_TYPE:
            yamlPhases.push_back(static_cast <ProtocolDropXSinItem *> (item)->getYamlVSin());
            break;

        case PROT_DROP_LIST_ISIN_ITEM_TYPE:
            yamlPhases.push_back(static_cast <ProtocolDropXSinItem *> (item)->getYamlISin());
            break;

        case PROT_DROP_LIST_VREP_SEQ_SCALED_ITEM_TYPE:
        case PROT_DROP_LIST_IREP_SEQ_SCALED_ITEM_TYPE:
            yamlPhases.push_back(static_cast <ProtocolDropIRepSeqScaledItem *> (item)->getYamlRepSeqScaled());
            break;

        case PROT_DROP_LIST_VREP_SEQ_ITEM_TYPE:
        case PROT_DROP_LIST_IREP_SEQ_ITEM_TYPE:
            yamlPhases.push_back(static_cast <ProtocolDropIRepSeqScaledItem *> (item)->getYamlRepSeq());
            break;

        case PROT_DROP_LIST_VREP_SEQ_WITH_STEPS_ITEM_TYPE:
        case PROT_DROP_LIST_IREP_SEQ_WITH_STEPS_ITEM_TYPE:
            yamlPhases.push_back(static_cast <ProtocolDropIRepSeqScaledItem *> (item)->getYamlRepSeqWithSteps());
            break;

        case PROT_DROP_LIST_VINF_REP_SEQ_ITEM_TYPE:
        case PROT_DROP_LIST_IINF_REP_SEQ_ITEM_TYPE:
            yamlPhases.push_back(static_cast <ProtocolDropIRepSeqScaledItem *> (item)->getYamlInfRepSeq());
            break;

        default:
            break;
        }
    }
    return yamlPhases;
}

std::vector <YAML::Analysis_t> ProtocolItemDropList::getYamlAnalyses() {
    std::vector <YAML::Analysis_t> yamlAnalyses;

    this->updateDropItemsList();

    for (int itemIdx = 0; itemIdx < items->size(); itemIdx++) {
        ProtocolDropItem * item = items->at(itemIdx);

        switch (item->type()) {
        case PROT_DROP_LIST_NOISE_REPORT_ITEM_TYPE:
        case PROT_DROP_LIST_HISTOGRAM_ITEM_TYPE:
        case PROT_DROP_LIST_SPECTRUM_ITEM_TYPE:
        case PROT_DROP_LIST_RESISTANCE_ESTIMATION_ITEM_TYPE:
        case PROT_DROP_LIST_MEMBRANE_TEST_ITEM_TYPE:
        case PROT_DROP_LIST_IV_GRAPH_ITEM_TYPE:
        case PROT_DROP_LIST_VOLTAGE_TRACKING_ITEM_TYPE:
        case PROT_DROP_LIST_AP_THRESHOLD_ITEM_TYPE:
        case PROT_DROP_LIST_AP_STATISTICS_ITEM_TYPE:
            yamlAnalyses.push_back(static_cast <ProtocolDropAnalysisItem *> (item)->getYamlAnalysis());
            break;

        default:
            break;
        }
    }
    return yamlAnalyses;
}

void ProtocolItemDropList::setControlsFromYaml(const std::vector <YAML::Control_t> &yamlControls, int voltageRangeIdx, int currentRangeIdx) {
    /*! Get the simulus range and set it before importing numeric values into the items */
    RangedMeasurement_t stimulusRange;
    if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
        mDev->getMessageDispatcher()->getVoltageProtocolRangeFeature((unsigned int)voltageRangeIdx, stimulusRange);

    } else {
        mDev->getMessageDispatcher()->getCurrentProtocolRangeFeature((unsigned int)currentRangeIdx, stimulusRange);
    }

    for (auto yamlControl : yamlControls) {
        ProtocolDropItem * item = nullptr;
        switch (yamlControl.index()) {
        case YAML::VoltageCtrlIdx:
            item = new ProtocolDropVoltageControlItem(mDev, ctrlManager, holdEdit->value());
            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropControlItem *> (item)->setCtrlFromYaml(std::get <YAML::VoltageCtrl> (yamlControl));
            break;

        case YAML::CurrentCtrlIdx:
            item = new ProtocolDropCurrentControlItem(mDev, ctrlManager, holdEdit->value());
            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropControlItem *> (item)->setCtrlFromYaml(std::get <YAML::CurrentCtrl> (yamlControl));
            break;

        case YAML::TimeCtrlIdx:
            item = new ProtocolDropTimeControlItem(mDev, ctrlManager, holdEdit->value());
            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropControlItem *> (item)->setCtrlFromYaml(std::get <YAML::TimeCtrl> (yamlControl));
            break;

        case YAML::FrequencyCtrlIdx:
            item = new ProtocolDropFrequencyControlItem(mDev, ctrlManager, holdEdit->value());
            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropControlItem *> (item)->setCtrlFromYaml(std::get <YAML::FrequencyCtrl> (yamlControl));
            break;

        case YAML::NaturalNumCtrlIdx:
            item = new ProtocolDropNaturalNumControlItem(mDev, ctrlManager, holdEdit->value());
            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropControlItem *> (item)->setCtrlFromYaml(std::get <YAML::NaturalNumCtrl> (yamlControl));
            break;
        }

        if (item != nullptr) {
            this->addItem(item);
            connect(item, &ProtocolDropItem::updateItem, this, &ProtocolItemDropList::onUpdateItem);
            connect(item, &ProtocolDropItem::protocolDropItemDeleteRequest, this, &ProtocolItemDropList::onDropItemDelete);
        }
    }
}

void ProtocolItemDropList::setPhasesFromYaml(const std::vector <YAML::Phase_t> &yamlPhases, int voltageRangeIdx, int currentRangeIdx) {
    /*! Get the simulus range and set it before importing numeric values into the items */
    RangedMeasurement_t stimulusRange;
    if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
        mDev->getMessageDispatcher()->getVoltageProtocolRangeFeature((unsigned int)voltageRangeIdx, stimulusRange);

    } else {
        mDev->getMessageDispatcher()->getCurrentProtocolRangeFeature((unsigned int)currentRangeIdx, stimulusRange);
    }

    for (auto yamlPhase : yamlPhases) {
        ProtocolDropItem * item = nullptr;
        switch (yamlPhase.index()) {
        case YAML::VHoldIdx:
            if (mDev->getMessageDispatcher()->hasProtocolStepFeature() == Success) {
                item = new ProtocolDropVHoldItem(mDev, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXStepTStepItem *> (item)->setPhaseFromYaml(std::get <YAML::VHold_t> (yamlPhase));
            }
            break;

        case YAML::VConstIdx:
            if (mDev->getMessageDispatcher()->hasProtocolStepFeature() == Success) {
                item = new ProtocolDropVConstItem(mDev, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXStepTStepItem *> (item)->setPhaseFromYaml(std::get <YAML::VConst_t> (yamlPhase));
            }
            break;

        case YAML::VStepTStepIdx:
            if (mDev->getMessageDispatcher()->hasProtocolStepFeature() == Success) {
                item = new ProtocolDropVStepTStepItem(mDev, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXStepTStepItem *> (item)->setPhaseFromYaml(std::get <YAML::VStepTStep_t> (yamlPhase));
            }
            break;

        case YAML::VRestIdx:
            if (mDev->getMessageDispatcher()->hasProtocolStepFeature() == Success) {
                item = new ProtocolDropVRestItem(mDev, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXStepTStepItem *> (item)->setPhaseFromYaml(std::get <YAML::VRest_t> (yamlPhase));
            }
            break;

        case YAML::VRampIdx:
            if (mDev->getMessageDispatcher()->hasProtocolRampFeature() == Success) {
                item = new ProtocolDropVRampItem(mDev, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXRampItem *> (item)->setPhaseFromYaml(std::get <YAML::VRamp_t> (yamlPhase));
            }
            break;

        case YAML::VSinIdx:
            if (mDev->getMessageDispatcher()->hasProtocolSinFeature() == Success) {
                item = new ProtocolDropVSinItem(mDev, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXSinItem *> (item)->setPhaseFromYaml(std::get <YAML::VSin_t> (yamlPhase));
            }
            break;

        case YAML::IHoldIdx:
            if (mDev->getMessageDispatcher()->hasProtocolStepFeature() == Success) {
                item = new ProtocolDropIHoldItem(mDev, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXStepTStepItem *> (item)->setPhaseFromYaml(std::get <YAML::IHold_t> (yamlPhase));
            }
            break;

        case YAML::IConstIdx:
            if (mDev->getMessageDispatcher()->hasProtocolStepFeature() == Success) {
                item = new ProtocolDropIConstItem(mDev, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXStepTStepItem *> (item)->setPhaseFromYaml(std::get <YAML::IConst_t> (yamlPhase));
            }
            break;

        case YAML::IStepTStepIdx:
            if (mDev->getMessageDispatcher()->hasProtocolStepFeature() == Success) {
                item = new ProtocolDropIStepTStepItem(mDev, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXStepTStepItem *> (item)->setPhaseFromYaml(std::get <YAML::IStepTStep_t> (yamlPhase));
            }
            break;

        case YAML::IRestIdx:
            if (mDev->getMessageDispatcher()->hasProtocolStepFeature() == Success) {
                item = new ProtocolDropIRestItem(mDev, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXStepTStepItem *> (item)->setPhaseFromYaml(std::get <YAML::IRest_t> (yamlPhase));
            }
            break;

        case YAML::IRampIdx:
            if (mDev->getMessageDispatcher()->hasProtocolRampFeature() == Success) {
                item = new ProtocolDropIRampItem(mDev, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXRampItem *> (item)->setPhaseFromYaml(std::get <YAML::VRamp_t> (yamlPhase));
            }
            break;

        case YAML::ISinIdx:
            if (mDev->getMessageDispatcher()->hasProtocolSinFeature() == Success) {
                item = new ProtocolDropISinItem(mDev, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXSinItem *> (item)->setPhaseFromYaml(std::get <YAML::ISin_t> (yamlPhase));
            }
            break;

        case YAML::RepSeqIdx:
            if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                item = new ProtocolDropVRepSeqItem(mDev, ctrlManager, holdEdit->value());

            } else {
                item = new ProtocolDropIRepSeqItem(mDev, ctrlManager, holdEdit->value());
            }

            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropXRepSeqScaledItem *> (item)->setPhaseFromYaml(std::get <YAML::RepSeq_t> (yamlPhase));
            break;

        case YAML::RepSeqWithStepsIdx:
            if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                item = new ProtocolDropVRepSeqWithStepsItem(mDev, ctrlManager, holdEdit->value());

            } else {
                item = new ProtocolDropIRepSeqWithStepsItem(mDev, ctrlManager, holdEdit->value());
            }

            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropXRepSeqScaledItem *> (item)->setPhaseFromYaml(std::get <YAML::RepSeqWithSteps_t> (yamlPhase));
            break;

        case YAML::RepSeqScaledIdx:
            if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                item = new ProtocolDropVRepSeqScaledItem(mDev, ctrlManager, holdEdit->value());

            } else {
                item = new ProtocolDropIRepSeqScaledItem(mDev, ctrlManager, holdEdit->value());
            }

            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropXRepSeqScaledItem *> (item)->setPhaseFromYaml(std::get <YAML::RepSeqScaled_t> (yamlPhase));
            break;

        case YAML::InfRepSeqIdx:
            if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                item = new ProtocolDropVInfRepSeqItem(mDev, ctrlManager, holdEdit->value());

            } else {
                item = new ProtocolDropIInfRepSeqItem(mDev, ctrlManager, holdEdit->value());
            }

            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropXRepSeqScaledItem *> (item)->setPhaseFromYaml(std::get <YAML::InfRepSeq_t> (yamlPhase));
            break;
        }

        if (item != nullptr) {
            this->addItem(item);
            connect(item, &ProtocolDropItem::updateItem, this, &ProtocolItemDropList::onUpdateItem);
            connect(item, &ProtocolDropItem::protocolDropItemDeleteRequest, this, &ProtocolItemDropList::onDropItemDelete);
        }
    }
}

void ProtocolItemDropList::setAnalysesFromYaml(const std::vector <YAML::Analysis_t> &yamlAnalyses) {
    for (auto yamlAnalysis : yamlAnalyses) {
        ProtocolDropItem * item = nullptr;
        switch (yamlAnalysis.type) {
        case YAML::NoiseReport: {
            item = new ProtocolDropNoiseReportItem(mDev, ctrlManager, holdEdit->value());
            ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
            this->setAnalysis(ProtocolConsumerNoiseReport, castItem);
            emit requestCursors(castItem);

            connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
            emit analysisChanged();
            break;
        }

        case YAML::Histogram: {
            if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                item = new ProtocolDropNoiseReportItem(mDev, ctrlManager, holdEdit->value());
                ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                this->setAnalysis(ProtocolConsumerNoiseReport, castItem);
                emit requestCursors(castItem);

                connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                emit analysisChanged();
            }
            break;
        }

        case YAML::Spectrum: {
            if (!(this->analysisRequested(ProtocolConsumerSpectrum))) {
                item = new ProtocolDropSpectrumItem(mDev, ctrlManager, holdEdit->value());
                ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                this->setAnalysis(ProtocolConsumerSpectrum, castItem);
                emit requestCursors(castItem);

                connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                emit analysisChanged();
            }
            break;
        }

        case YAML::MembraneTest: {
            if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                if (!(this->analysisRequested(ProtocolConsumerMembraneTest))) {
                    item = new ProtocolDropMembraneTestItem(mDev, ctrlManager, holdEdit->value());
                    ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                    this->setAnalysis(ProtocolConsumerMembraneTest, castItem);
                    emit requestCursors(castItem);

                    connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                    emit analysisChanged();
                }

            } else {
                /*! \todo FCON da implementare se serve una analisi diversa in current clamp */
            }
            break;
        }

        case YAML::IVGraph: {
            if (!(this->analysisRequested(ProtocolConsumerIvGraph))) {
                item = new ProtocolDropIvGraphItem(mDev, ctrlManager, holdEdit->value());
                ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                this->setAnalysis(ProtocolConsumerIvGraph, castItem);
                emit requestCursors(castItem);

                connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                emit analysisChanged();
            }
            break;
        }

        case YAML::ResistanceEstimation: {
            if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                /*! \todo FCON da implementare se serve una analisi diversa in voltage clamp */

            } else {
                if (!(this->analysisRequested(ProtocolConsumerResistanceEstimation))) {
                    item = new ProtocolDropResistanceEstimationItem(mDev, ctrlManager, holdEdit->value());
                    ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                    this->setAnalysis(ProtocolConsumerResistanceEstimation, castItem);
                    emit requestCursors(castItem);

                    connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                    emit analysisChanged();
                }
            }
            break;
        }

        case YAML::APThreshold: {
            if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                /*! \todo FCON da implementare se serve una analisi diversa in voltage clamp */

            } else {
                if (!(this->analysisRequested(ProtocolConsumerApThreshold))) {
                    item = new ProtocolDropApThresholdItem(mDev, ctrlManager, holdEdit->value());
                    ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                    this->setAnalysis(ProtocolConsumerApThreshold, castItem);
                    emit requestCursors(castItem);

                    connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                    emit analysisChanged();
                }
            }
            break;
        }

        case YAML::APStatistics: {
            if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                /*! \todo FCON da implementare se serve una analisi diversa in voltage clamp */

            } else {
                if (!(this->analysisRequested(ProtocolConsumerApStatistics))) {
                    item = new ProtocolDropApStatisticsItem(mDev, ctrlManager, holdEdit->value());
                    ProtocolDropAnalysisItem * castItem = static_cast <ProtocolDropAnalysisItem *> (item);
                    this->setAnalysis(ProtocolConsumerApStatistics, castItem);
                    emit requestCursors(castItem);

                    connect(item, &ProtocolDropItem::analysisChanged, this, &ProtocolItemDropList::analysisChanged);
                    emit analysisChanged();
                }
            }
            break;
        }
        }

        if (item != nullptr) {
            static_cast <ProtocolDropAnalysisItem *> (item)->setAnalysisFromYaml(yamlAnalysis);

            this->addItem(item);
            connect(item, &ProtocolDropItem::updateItem, this, &ProtocolItemDropList::onUpdateItem);
            connect(item, &ProtocolDropItem::protocolDropItemDeleteRequest, this, &ProtocolItemDropList::onDropItemDelete);
        }
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
            item = new ProtocolDropVStepTStepItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_ISTEP_TSTEP_ITEM_TYPE:
            item = new ProtocolDropIStepTStepItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VSTEP_ITEM_TYPE:
            item = new ProtocolDropVStepItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_ISTEP_ITEM_TYPE:
            item = new ProtocolDropIStepItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VTSTEP_ITEM_TYPE:
            item = new ProtocolDropVTStepItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_ITSTEP_ITEM_TYPE:
            item = new ProtocolDropITStepItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VCONST_ITEM_TYPE:
            item = new ProtocolDropVConstItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_ICONST_ITEM_TYPE:
            item = new ProtocolDropIConstItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VHOLD_ITEM_TYPE:
            item = new ProtocolDropVHoldItem(mDev, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_IHOLD_ITEM_TYPE:
            item = new ProtocolDropIHoldItem(mDev, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_VREST_ITEM_TYPE:
            item = new ProtocolDropVRestItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_IREST_ITEM_TYPE:
            item = new ProtocolDropIRestItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VRAMP_ITEM_TYPE:
            item = new ProtocolDropVRampItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_IRAMP_ITEM_TYPE:
            item = new ProtocolDropIRampItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VSIN_ITEM_TYPE:
            item = new ProtocolDropVSinItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_ISIN_ITEM_TYPE:
            item = new ProtocolDropISinItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VREP_SEQ_SCALED_ITEM_TYPE:
            item = new ProtocolDropVRepSeqScaledItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_IREP_SEQ_SCALED_ITEM_TYPE:
            item = new ProtocolDropIRepSeqScaledItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VREP_SEQ_ITEM_TYPE:
            item = new ProtocolDropVRepSeqItem(mDev, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_IREP_SEQ_ITEM_TYPE:
            item = new ProtocolDropIRepSeqItem(mDev, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_VREP_SEQ_WITH_STEPS_ITEM_TYPE:
            item = new ProtocolDropVRepSeqWithStepsItem(mDev, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_IREP_SEQ_WITH_STEPS_ITEM_TYPE:
            item = new ProtocolDropIRepSeqWithStepsItem(mDev, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_VINF_REP_SEQ_ITEM_TYPE:
            item = new ProtocolDropVInfRepSeqItem(mDev, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_IINF_REP_SEQ_ITEM_TYPE:
            item = new ProtocolDropIInfRepSeqItem(mDev, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_VOLTAGE_CONTROL_ITEM_TYPE:
            item = new ProtocolDropVoltageControlItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_CURRENT_CONTROL_ITEM_TYPE:
            item = new ProtocolDropCurrentControlItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_TIME_CONTROL_ITEM_TYPE:
            item = new ProtocolDropTimeControlItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_FREQUENCY_CONTROL_ITEM_TYPE:
            item = new ProtocolDropFrequencyControlItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_NATURAL_NUM_CONTROL_ITEM_TYPE:
            item = new ProtocolDropNaturalNumControlItem(mDev, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_NOISE_REPORT_ITEM_TYPE:
            if (!(this->analysisRequested(ProtocolConsumerNoiseReport))) {
                item = new ProtocolDropNoiseReportItem(mDev, ctrlManager, holdEdit->value());
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
                item = new ProtocolDropHistogramItem(mDev, ctrlManager, holdEdit->value());
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
                item = new ProtocolDropSpectrumItem(mDev, ctrlManager, holdEdit->value());
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
                item = new ProtocolDropResistanceEstimationItem(mDev, ctrlManager, holdEdit->value());
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
                item = new ProtocolDropMembraneTestItem(mDev, ctrlManager, holdEdit->value());
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
                item = new ProtocolDropIvGraphItem(mDev, ctrlManager, holdEdit->value());
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
                item = new ProtocolDropVoltageTrackingItem(mDev, ctrlManager, holdEdit->value());
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
                item = new ProtocolDropApThresholdItem(mDev, ctrlManager, holdEdit->value());
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
                item = new ProtocolDropApStatisticsItem(mDev, ctrlManager, holdEdit->value());
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

GapfreeProtocolItemDropList::GapfreeProtocolItemDropList(ModelDevice *  mDev, QDoubleSpinBox * holdEdit, ClampingModality_t clampingModality) :
    ProtocolItemDropList(mDev, holdEdit, clampingModality) {

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

EpisodicProtocolItemDropList::EpisodicProtocolItemDropList(ModelDevice *  mDev, QDoubleSpinBox * holdEdit, ClampingModality_t clampingModality) :
    ProtocolItemDropList(mDev, holdEdit, clampingModality) {

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

CtrlProtocolItemDropList::CtrlProtocolItemDropList(ModelDevice *  mDev, QDoubleSpinBox * holdEdit, ClampingModality_t clampingModality) :
    ProtocolItemDropList(mDev, holdEdit, clampingModality) {

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

AnalysisProtocolItemDropList::AnalysisProtocolItemDropList(ModelDevice *  mDev, QDoubleSpinBox * holdEdit, ClampingModality_t clampingModality) :
    ProtocolItemDropList(mDev, holdEdit, clampingModality) {

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
