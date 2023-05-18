#include "protocolmanager.h"

ProtocolManager::ProtocolManager(e4gcl::CommLib * commLib) :
    QObject(),
    commLib(commLib) {

}

ProtocolManager::ProtocolApplicationStatus_t ProtocolManager::startProtocol(ProtocolWidget * protocol, bool recordFlag) {
    this->protocol = protocol;
    this->recordFlag = recordFlag;
    e4gcl::Measurement_t hold = protocol->getHold();
    int sweepsNum = protocol->getSweepsNum();
    int clampingModality = protocol->getClampingModality();

    /*! Preprocess protocol items */
    protocol->setProtocolItems();
    protocol->setProcessingStatus();
    protocol->setAnalysisCursors();
    protocol->setTriggerCursors();
    QVector <ProtocolItem *> protocolItems = protocol->getProtocolItems();
    ProtocolApplicationStatus_t status = this->toProtocolApplicationStatus(protocol->getProcessingStatus());

    if (status != Success) {
        return status;
    }

    if (protocol->isInhibited()) {
        return ErrorProtocolInhibited;
    }

    /*! Send the commands to commLib */
    QVector <TriggerCursor *> triggerCursors = protocol->getTriggerCursors();
    TriggerCursor * cursor;
    e4gcl::Measurement_t delay = {0.0, e4gcl::UnitPfxNone, "s"};
    for (int cursorIdx = 0; cursorIdx < triggerCursors.size(); cursorIdx++) {
        cursor = triggerCursors[cursorIdx];
        delay.value = cursor->delay;
        commLib->setDigitalTriggerOutput((unsigned short)cursorIdx, cursor->terminator, cursor->high, (unsigned short)cursor->id, delay);
    }

    if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
        commLib->setVoltageProtocolStructure(protocolId, (unsigned short)(protocolItems.size()), (unsigned short)sweepsNum, hold);

    } else {
        emit currentApplied();
        commLib->setCurrentProtocolStructure(protocolId, (unsigned short)(protocolItems.size()), (unsigned short)sweepsNum, hold);
    }

    e4gcl::UnitPfx_t stimulusPrefix = protocol->getStimulusPrefix();

    x0.prefix = stimulusPrefix;
    xStep.prefix = stimulusPrefix;
    xFinal.prefix = stimulusPrefix;
    xAmp.prefix = stimulusPrefix;

    for (int itemIdx = 0; itemIdx < protocolItems.size(); itemIdx++) {
        ProtocolItem * protocolItem = protocolItems[itemIdx];
        switch (protocolItem->type) {
        case ProtocolItemXStepTStep: {
            ProtocolXStepTStepItem * castItem = static_cast <ProtocolXStepTStepItem *> (protocolItem);
            x0.value = castItem->x0;
            xStep.value = castItem->xStep;
            t0.value = castItem->t0;
            tStep.value = castItem->tStep;

            if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                commLib->voltStepTimeStep(x0, xStep, t0, tStep, (uint16_t)itemIdx, (uint16_t)protocolItem->nextItem, (uint16_t)protocolItem->repsNum, (uint16_t)(protocolItem->applySteps ? 1 : 0));

            } else {
                commLib->currStepTimeStep(x0, xStep, t0, tStep, (uint16_t)itemIdx, (uint16_t)protocolItem->nextItem, (uint16_t)protocolItem->repsNum, (uint16_t)(protocolItem->applySteps ? 1 : 0));
            }
            break;
        }

        case ProtocolItemXRamp: {
            ProtocolXRampItem * castItem = static_cast <ProtocolXRampItem *> (protocolItem);
            x0.value = castItem->x0;
            xFinal.value = castItem->xFinal;
            t0.value = castItem->t0;

            if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                commLib->voltRamp(x0, xFinal, t0, (uint16_t)itemIdx, (uint16_t)protocolItem->nextItem, (uint16_t)protocolItem->repsNum, (uint16_t)(protocolItem->applySteps ? 1 : 0));

            } else {
                commLib->currRamp(x0, xFinal, t0, (uint16_t)itemIdx, (uint16_t)protocolItem->nextItem, (uint16_t)protocolItem->repsNum, (uint16_t)(protocolItem->applySteps ? 1 : 0));
            }
            break;
        }

        case ProtocolItemXSin: {
            ProtocolXSinItem * castItem = static_cast <ProtocolXSinItem *> (protocolItem);
            x0.value = castItem->x0;
            xAmp.value = castItem->xAmp;
            freq.value = castItem->freq;

            if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
                commLib->voltSin(x0, xAmp, freq, (uint16_t)itemIdx, (uint16_t)protocolItem->nextItem, (uint16_t)protocolItem->repsNum, (uint16_t)(protocolItem->applySteps ? 1 : 0));

            } else {
                commLib->currSin(x0, xAmp, freq, (uint16_t)itemIdx, (uint16_t)protocolItem->nextItem, (uint16_t)protocolItem->repsNum, (uint16_t)(protocolItem->applySteps ? 1 : 0));
            }
            break;
        }
        }
    }

    if (!secondaryDeviceFlag) {
        commLib->startProtocol();
    }

    lastStartedType = protocol->getType();

    protocol->resetConsumerRequests();
    if (recordFlag) {
        protocol->setConsumerRequest(ProtocolConsumerDataWriter);
    }

    emit protocolStarted(protocolId, protocol);

    if (!(protocol->isNullProtocol()) && !recordFlag) {
        lastRunProtocolId = protocolId;
    }

    /*! Increment protocol ID: this is used to discriminate data coming for the current protocol from spurious data from the previous procotocol */
    protocolId = (protocolId+1) & GLB_MAX_PROT_ID;

    return status;
}

void ProtocolManager::saveLast(ProtocolWidget * protocol) {
    /*! Preprocess protocol items */
    protocol->setProtocolItems();
    protocol->setProcessingStatus();
    protocol->setAnalysisCursors();
    protocol->resetConsumerRequests();
    protocol->setConsumerRequest(ProtocolConsumerDataWriter);

    emit protocolSaveRequest(lastRunProtocolId, protocol);
}

ProtocolType_t ProtocolManager::getLastStartedType() {
    return lastStartedType;
}

void ProtocolManager::setSecondaryDevice(bool flag) {
    secondaryDeviceFlag = flag;
}

void ProtocolManager::onIncreaseProtocolId() {
    /*! Increment protocol ID: this is used to discriminate data coming for the current protocol from spurious data from the previous procotocol */
    /*! This is done here in this slot too so that the protocol manager of a given clamping modality increases the protocol ID of all other protocol managers as well */
    protocolId = (protocolId+1) & GLB_MAX_PROT_ID;
}

void ProtocolManager::onProtocolEnded() {
    if (secondaryDeviceFlag) {
        /*! Ifthe device is secondary get ready for another trigger */
        this->startProtocol(protocol, recordFlag);
    }
}

ProtocolManager::ProtocolApplicationStatus_t ProtocolManager::toProtocolApplicationStatus(ItemsProcStatus_t status) {
    switch (status) {
    case ItemsProcOk:
        return Success;

    case ItemsProcErrorNotEnoughItemsForSequence:
        return ErrorNotEnoughItemsForSequence;

    case ItemsProcErrorOverlappingSequences:
        return ErrorOverlappingSequences;

    case ItemsProcErrorMidInfiniteSequence:
        return ErrorMidInfiniteSequence;

    case ItemsOverflow:
        return ErrorItemsOverflow;

    case ItemsNotFound:
        return ErrorItemsNotFound;

    case ItemsOverStimulus:
        return ErrorItemsOverStimulus;

    case ItemsUnderStimulus:
        return ErrorItemsUnderStimulus;

    case ItemsUnderDuration:
        return ErrorItemsUnderDuration;

    case ItemsNotProcessed:
        return ErrorItemsNotProcessed;
    }

    return Success;
}
