#include "protocolitemdroplist.h"

#include <QtWidgets>

ProtocolItemDropList::ProtocolItemDropList(MessageDispatcher * msgDisp, QDoubleSpinBox * holdEdit, ClampingModality_t clampingModality) :
    QListWidget(),
    msgDisp(msgDisp),
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

void ProtocolItemDropList::setControlsFromYaml(const std::vector <YAML::Control_t> &yamlControls, int voltageRangeIdx, int currentRangeIdx) {
    /*! Get the simulus range and set it before importing numeric values into the items */
    RangedMeasurement_t stimulusRange;
    if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
        msgDisp->getVoltageProtocolRangeFeature((unsigned int)voltageRangeIdx, stimulusRange);

    } else {
        msgDisp->getCurrentProtocolRangeFeature((unsigned int)currentRangeIdx, stimulusRange);
    }

    for (auto yamlControl : yamlControls) {
        ProtocolDropItem * item = nullptr;
        switch (yamlControl.index()) {
        case YAML::VoltageCtrlIdx:
            item = new ProtocolDropVoltageControlItem(msgDisp, ctrlManager, holdEdit->value());
            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropControlItem *> (item)->setCtrlFromYaml(std::get <YAML::VoltageCtrl> (yamlControl));
            break;

        case YAML::CurrentCtrlIdx:
            item = new ProtocolDropCurrentControlItem(msgDisp, ctrlManager, holdEdit->value());
            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropControlItem *> (item)->setCtrlFromYaml(std::get <YAML::CurrentCtrl> (yamlControl));
            break;

        case YAML::TimeCtrlIdx:
            item = new ProtocolDropTimeControlItem(msgDisp, ctrlManager, holdEdit->value());
            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropControlItem *> (item)->setCtrlFromYaml(std::get <YAML::TimeCtrl> (yamlControl));
            break;

        case YAML::FrequencyCtrlIdx:
            item = new ProtocolDropFrequencyControlItem(msgDisp, ctrlManager, holdEdit->value());
            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropControlItem *> (item)->setCtrlFromYaml(std::get <YAML::FrequencyCtrl> (yamlControl));
            break;

        case YAML::NaturalNumCtrlIdx:
            item = new ProtocolDropNaturalNumControlItem(msgDisp, ctrlManager, holdEdit->value());
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
        msgDisp->getVoltageProtocolRangeFeature((unsigned int)voltageRangeIdx, stimulusRange);

    } else {
        msgDisp->getCurrentProtocolRangeFeature((unsigned int)currentRangeIdx, stimulusRange);
    }

    for (auto yamlPhase : yamlPhases) {
        ProtocolDropItem * item = nullptr;
        switch (yamlPhase.index()) {
        case YAML::VHoldIdx:
            if (msgDisp->hasProtocolStepFeature() == Success) {
                item = new ProtocolDropVHoldItem(msgDisp, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXStepTStepItem *> (item)->setPhaseFromYaml(std::get <YAML::VHold_t> (yamlPhase));
                connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            }
            break;

        case YAML::VConstIdx:
            if (msgDisp->hasProtocolStepFeature() == Success) {
                item = new ProtocolDropVConstItem(msgDisp, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXStepTStepItem *> (item)->setPhaseFromYaml(std::get <YAML::VConst_t> (yamlPhase));
            }
            break;

        case YAML::VStepTStepIdx:
            if (msgDisp->hasProtocolStepFeature() == Success) {
                item = new ProtocolDropVStepTStepItem(msgDisp, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXStepTStepItem *> (item)->setPhaseFromYaml(std::get <YAML::VStepTStep_t> (yamlPhase));
            }
            break;

        case YAML::VRestIdx:
            if (msgDisp->hasProtocolStepFeature() == Success) {
                item = new ProtocolDropVRestItem(msgDisp, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXStepTStepItem *> (item)->setPhaseFromYaml(std::get <YAML::VRest_t> (yamlPhase));
            }
            break;

        case YAML::VRampIdx:
            if (msgDisp->hasProtocolRampFeature() == Success) {
                item = new ProtocolDropVRampItem(msgDisp, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXRampItem *> (item)->setPhaseFromYaml(std::get <YAML::VRamp_t> (yamlPhase));
            }
            break;

        case YAML::VSinIdx:
            if (msgDisp->hasProtocolSinFeature() == Success) {
                item = new ProtocolDropVSinItem(msgDisp, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXSinItem *> (item)->setPhaseFromYaml(std::get <YAML::VSin_t> (yamlPhase));
            }
            break;

        case YAML::IHoldIdx:
            if (msgDisp->hasProtocolStepFeature() == Success) {
                item = new ProtocolDropIHoldItem(msgDisp, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXStepTStepItem *> (item)->setPhaseFromYaml(std::get <YAML::IHold_t> (yamlPhase));
                connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            }
            break;

        case YAML::IConstIdx:
            if (msgDisp->hasProtocolStepFeature() == Success) {
                item = new ProtocolDropIConstItem(msgDisp, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXStepTStepItem *> (item)->setPhaseFromYaml(std::get <YAML::IConst_t> (yamlPhase));
            }
            break;

        case YAML::IStepTStepIdx:
            if (msgDisp->hasProtocolStepFeature() == Success) {
                item = new ProtocolDropIStepTStepItem(msgDisp, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXStepTStepItem *> (item)->setPhaseFromYaml(std::get <YAML::IStepTStep_t> (yamlPhase));
            }
            break;

        case YAML::IRestIdx:
            if (msgDisp->hasProtocolStepFeature() == Success) {
                item = new ProtocolDropIRestItem(msgDisp, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXStepTStepItem *> (item)->setPhaseFromYaml(std::get <YAML::IRest_t> (yamlPhase));
            }
            break;

        case YAML::IRampIdx:
            if (msgDisp->hasProtocolRampFeature() == Success) {
                item = new ProtocolDropIRampItem(msgDisp, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXRampItem *> (item)->setPhaseFromYaml(std::get <YAML::IRamp_t> (yamlPhase));
            }
            break;

        case YAML::ISinIdx:
            if (msgDisp->hasProtocolSinFeature() == Success) {
                item = new ProtocolDropISinItem(msgDisp, ctrlManager, holdEdit->value());
                item->setStimulusRange(stimulusRange);
                static_cast <ProtocolDropXSinItem *> (item)->setPhaseFromYaml(std::get <YAML::ISin_t> (yamlPhase));
            }
            break;

        case YAML::RepSeqIdx:
            if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                item = new ProtocolDropVRepSeqItem(msgDisp, ctrlManager, holdEdit->value());

            } else {
                item = new ProtocolDropIRepSeqItem(msgDisp, ctrlManager, holdEdit->value());
            }

            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropXRepSeqScaledItem *> (item)->setPhaseFromYaml(std::get <YAML::RepSeq_t> (yamlPhase));
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case YAML::RepSeqWithStepsIdx:
            if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                item = new ProtocolDropVRepSeqWithStepsItem(msgDisp, ctrlManager, holdEdit->value());

            } else {
                item = new ProtocolDropIRepSeqWithStepsItem(msgDisp, ctrlManager, holdEdit->value());
            }

            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropXRepSeqScaledItem *> (item)->setPhaseFromYaml(std::get <YAML::RepSeqWithSteps_t> (yamlPhase));
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case YAML::RepSeqScaledIdx:
            if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                item = new ProtocolDropVRepSeqScaledItem(msgDisp, ctrlManager, holdEdit->value());

            } else {
                item = new ProtocolDropIRepSeqScaledItem(msgDisp, ctrlManager, holdEdit->value());
            }

            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropXRepSeqScaledItem *> (item)->setPhaseFromYaml(std::get <YAML::RepSeqScaled_t> (yamlPhase));
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case YAML::InfRepSeqIdx:
            if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                item = new ProtocolDropVInfRepSeqItem(msgDisp, ctrlManager, holdEdit->value());

            } else {
                item = new ProtocolDropIInfRepSeqItem(msgDisp, ctrlManager, holdEdit->value());
            }

            item->setStimulusRange(stimulusRange);
            static_cast <ProtocolDropXRepSeqScaledItem *> (item)->setPhaseFromYaml(std::get <YAML::InfRepSeq_t> (yamlPhase));
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;
        }

        if (item != nullptr) {
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
            item = new ProtocolDropVStepTStepItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_ISTEP_TSTEP_ITEM_TYPE:
            item = new ProtocolDropIStepTStepItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VSTEP_ITEM_TYPE:
            item = new ProtocolDropVStepItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_ISTEP_ITEM_TYPE:
            item = new ProtocolDropIStepItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VTSTEP_ITEM_TYPE:
            item = new ProtocolDropVTStepItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_ITSTEP_ITEM_TYPE:
            item = new ProtocolDropITStepItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VCONST_ITEM_TYPE:
            item = new ProtocolDropVConstItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_ICONST_ITEM_TYPE:
            item = new ProtocolDropIConstItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VHOLD_ITEM_TYPE:
            item = new ProtocolDropVHoldItem(msgDisp, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_IHOLD_ITEM_TYPE:
            item = new ProtocolDropIHoldItem(msgDisp, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_VREST_ITEM_TYPE:
            item = new ProtocolDropVRestItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_IREST_ITEM_TYPE:
            item = new ProtocolDropIRestItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VRAMP_ITEM_TYPE:
            item = new ProtocolDropVRampItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_IRAMP_ITEM_TYPE:
            item = new ProtocolDropIRampItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VSIN_ITEM_TYPE:
            item = new ProtocolDropVSinItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_ISIN_ITEM_TYPE:
            item = new ProtocolDropISinItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VREP_SEQ_SCALED_ITEM_TYPE:
            item = new ProtocolDropVRepSeqScaledItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_IREP_SEQ_SCALED_ITEM_TYPE:
            item = new ProtocolDropIRepSeqScaledItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_VREP_SEQ_ITEM_TYPE:
            item = new ProtocolDropVRepSeqItem(msgDisp, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_IREP_SEQ_ITEM_TYPE:
            item = new ProtocolDropIRepSeqItem(msgDisp, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_VREP_SEQ_WITH_STEPS_ITEM_TYPE:
            item = new ProtocolDropVRepSeqWithStepsItem(msgDisp, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_IREP_SEQ_WITH_STEPS_ITEM_TYPE:
            item = new ProtocolDropIRepSeqWithStepsItem(msgDisp, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_VINF_REP_SEQ_ITEM_TYPE:
            item = new ProtocolDropVInfRepSeqItem(msgDisp, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_IINF_REP_SEQ_ITEM_TYPE:
            item = new ProtocolDropIInfRepSeqItem(msgDisp, ctrlManager, holdEdit->value());
            connect(this, &ProtocolItemDropList::updateHold, item, &ProtocolDropItem::onUpdateHold);
            break;

        case PROT_DRAG_LIST_VOLTAGE_CONTROL_ITEM_TYPE:
            item = new ProtocolDropVoltageControlItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_CURRENT_CONTROL_ITEM_TYPE:
            item = new ProtocolDropCurrentControlItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_TIME_CONTROL_ITEM_TYPE:
            item = new ProtocolDropTimeControlItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_FREQUENCY_CONTROL_ITEM_TYPE:
            item = new ProtocolDropFrequencyControlItem(msgDisp, ctrlManager, holdEdit->value());
            break;

        case PROT_DRAG_LIST_NATURAL_NUM_CONTROL_ITEM_TYPE:
            item = new ProtocolDropNaturalNumControlItem(msgDisp, ctrlManager, holdEdit->value());
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
            break;
        }
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

GapfreeProtocolItemDropList::GapfreeProtocolItemDropList(MessageDispatcher * msgDisp, QDoubleSpinBox * holdEdit, ClampingModality_t clampingModality) :
    ProtocolItemDropList(msgDisp, holdEdit, clampingModality) {

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

EpisodicProtocolItemDropList::EpisodicProtocolItemDropList(MessageDispatcher * msgDisp, QDoubleSpinBox * holdEdit, ClampingModality_t clampingModality) :
    ProtocolItemDropList(msgDisp, holdEdit, clampingModality) {

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

CtrlProtocolItemDropList::CtrlProtocolItemDropList(MessageDispatcher * msgDisp, QDoubleSpinBox * holdEdit, ClampingModality_t clampingModality) :
    ProtocolItemDropList(msgDisp, holdEdit, clampingModality) {

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
