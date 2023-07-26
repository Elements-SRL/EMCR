#include "protocolmanager.h"

#include "globaldefines.h"

ProtocolManager::ProtocolManager(MessageDispatcher * msgDisp) :
    QObject(),
    msgDisp(msgDisp) {

}

void ProtocolManager::onStartProtocolRequest(ProtocolWidget * protocol) {
    emit protocolRequestOutcome(this->startProtocol(protocol));
}

ProtocolApplicationStatus_t ProtocolManager::startProtocol(ProtocolWidget * protocol, bool recordFlag) {
    this->protocol = protocol;
    this->recordFlag = recordFlag;
    Measurement_t hold = protocol->getHold();
    int sweepsNum = protocol->getSweepsNum();
    e384CommLib::ClampingModality_t clampingModality = protocol->getClampingModality();

    /*! Preprocess protocol items */
    protocol->setProtocolItems();
    protocol->setProcessingStatus();
    protocol->setAnalysisCursors();
    protocol->setTriggerCursors();
    QVector <ProtocolItem *> protocolItems = protocol->getProtocolItems();
    ProtocolApplicationStatus_t status = this->toProtocolApplicationStatus(protocol->getProcessingStatus());

    if (status != ProtocolApplicationSuccess) {
        return status;
    }

    if (protocol->isInhibited()) {
        return ErrorProtocolInhibited;
    }

    /*! Send the commands to mDev */
    if (clampingModality == e384CommLib::ClampingModality_t::VOLTAGE_CLAMP) {
        msgDisp->setVoltageProtocolStructure(protocolId, (unsigned short)(protocolItems.size()), (unsigned short)sweepsNum, hold);

    } else {
        emit currentApplied();
        msgDisp->setCurrentProtocolStructure(protocolId, (unsigned short)(protocolItems.size()), (unsigned short)sweepsNum, hold);
    }

    UnitPfx_t stimulusPrefix = protocol->getStimulusPrefix();

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
            t0Step.value = castItem->tStep;

            if (clampingModality == e384CommLib::ClampingModality_t::VOLTAGE_CLAMP) {
                msgDisp->setVoltageProtocolStep((uint16_t)itemIdx, (uint16_t)protocolItem->nextItem, (uint16_t)protocolItem->repsNum, protocolItem->applySteps, x0, xStep, t0, t0Step);

            } else {
                msgDisp->setCurrentProtocolStep((uint16_t)itemIdx, (uint16_t)protocolItem->nextItem, (uint16_t)protocolItem->repsNum, protocolItem->applySteps, x0, xStep, t0, t0Step);
            }
            break;
        }

        case ProtocolItemXRamp: {
            ProtocolXRampItem * castItem = static_cast <ProtocolXRampItem *> (protocolItem);
            x0.value = castItem->x0;
            xStep.value = 0.0;
            xFinal.value = castItem->xFinal;
            xFinalStep.value = 0.0;
            t0.value = castItem->t0;
            t0Step.value = 0.0;

            if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
                msgDisp->setVoltageProtocolRamp((uint16_t)itemIdx, (uint16_t)protocolItem->nextItem, (uint16_t)protocolItem->repsNum, protocolItem->applySteps, x0, xStep, xFinal, xFinalStep, t0, t0Step);

            } else {
                msgDisp->setCurrentProtocolRamp((uint16_t)itemIdx, (uint16_t)protocolItem->nextItem, (uint16_t)protocolItem->repsNum, protocolItem->applySteps, x0, xStep, xFinal, xFinalStep, t0, t0Step);
            }
            break;
        }

        case ProtocolItemXSin: {
            ProtocolXSinItem * castItem = static_cast <ProtocolXSinItem *> (protocolItem);
            x0.value = castItem->x0;
            xStep.value = 0.0;
            xAmp.value = castItem->xAmp;
            xAmpStep.value = 0.0;
            f0.value = castItem->freq;
            f0Step.value = 0.0;

            if (clampingModality == e384CommLib::ClampingModality_t::VOLTAGE_CLAMP) {
                msgDisp->setVoltageProtocolSin((uint16_t)itemIdx, (uint16_t)protocolItem->nextItem, (uint16_t)protocolItem->repsNum, protocolItem->applySteps, x0, xStep, xAmp, xAmpStep, f0, f0Step);

            } else {
                msgDisp->setCurrentProtocolSin((uint16_t)itemIdx, (uint16_t)protocolItem->nextItem, (uint16_t)protocolItem->repsNum, protocolItem->applySteps, x0, xStep, xAmp, xAmpStep, f0, f0Step);
            }
            break;
        }
        }
    }

    msgDisp->startProtocol();

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

void ProtocolManager::onIncreaseProtocolId() {
    /*! Increment protocol ID: this is used to discriminate data coming for the current protocol from spurious data from the previous procotocol */
    /*! This is done here in this slot too so that the protocol manager of a given clamping modality increases the protocol ID of all other protocol managers as well */
    protocolId = (protocolId+1) & GLB_MAX_PROT_ID;
}

ProtocolApplicationStatus_t ProtocolManager::toProtocolApplicationStatus(ItemsProcStatus_t status) {
    switch (status) {
    case ItemsProcOk:
        return ProtocolApplicationSuccess;

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

    return ProtocolApplicationSuccess;
}
