#include "protocolitemctrlmanager.h"
#include "protocolitemdroplist.h"
#include "protocoldropitem.h"
#include "protocoldropitemparam.h"

ProtocolItemCtrlManager::ProtocolItemCtrlManager(ProtocolItemDropList * pidl) {
    ctrlPidl = pidl;
    connections = new QMap <ProtocolDropItemParam *, ProtocolDropControlItem *>;
}

double ProtocolItemCtrlManager::getVoltageCtrlValue(int cbxIdx) {
    if (cbxIdx > 0) {
        unsigned int ctrlIdx = (cbxIdx-1)/2;
        ProtocolDropControlItem * ctrl = voltageCtrlItems->at(ctrlIdx);
        double sign = ((cbxIdx % 2) == 0 ? -1.0 : 1.0);
        return sign*ctrl->getDoubleValue();

    } else {
        return 0.0;
    }
}

double ProtocolItemCtrlManager::getCurrentCtrlValue(int cbxIdx) {
    if (cbxIdx > 0) {
        unsigned int ctrlIdx = (cbxIdx-1)/2;
        ProtocolDropControlItem * ctrl = currentCtrlItems->at(ctrlIdx);
        double sign = ((cbxIdx % 2) == 0 ? -1.0 : 1.0);
        return sign*ctrl->getDoubleValue();

    } else {
        return 0.0;
    }
}

double ProtocolItemCtrlManager::getTimeCtrlValue(int cbxIdx) {
    if (cbxIdx > 0) {
        unsigned int ctrlIdx = (cbxIdx-1)/2;
        ProtocolDropControlItem * ctrl = timeCtrlItems->at(ctrlIdx);
        double sign = ((cbxIdx % 2) == 0 ? -1.0 : 1.0);
        return sign*ctrl->getDoubleValue();

    } else {
        return 0.0;
    }
}

double ProtocolItemCtrlManager::getFrequencyCtrlValue(int cbxIdx) {
    if (cbxIdx > 0) {
        unsigned int ctrlIdx = (cbxIdx-1)/2;
        ProtocolDropControlItem * ctrl = frequencyCtrlItems->at(ctrlIdx);
        double sign = ((cbxIdx % 2) == 0 ? -1.0 : 1.0);
        return sign*ctrl->getDoubleValue();

    } else {
        return 0.0;
    }
}

int ProtocolItemCtrlManager::getNaturalNumCtrlValue(int cbxIdx) {
    if (cbxIdx > 0) {
        unsigned int ctrlIdx = (cbxIdx-1)/2;
        ProtocolDropControlItem * ctrl = naturalNumCtrlItems->at(ctrlIdx);
        double sign = ((cbxIdx % 2) == 0 ? -1 : 1);
        return sign*ctrl->getIntValue();

    } else {
        return 0.0;
    }
}

QVector <ProtocolDropControlItem *> * ProtocolItemCtrlManager::getVoltageCtrlItems() {
    voltageCtrlItems = ctrlPidl->getVoltageCtrlItems();
    return voltageCtrlItems;
}

QVector <ProtocolDropControlItem *> * ProtocolItemCtrlManager::getCurrentCtrlItems() {
    currentCtrlItems = ctrlPidl->getCurrentCtrlItems();
    return currentCtrlItems;
}

QVector <ProtocolDropControlItem *> * ProtocolItemCtrlManager::getTimeCtrlItems() {
    timeCtrlItems = ctrlPidl->getTimeCtrlItems();
    return timeCtrlItems;
}

QVector <ProtocolDropControlItem *> * ProtocolItemCtrlManager::getFrequencyCtrlItems() {
    frequencyCtrlItems = ctrlPidl->getFrequencyCtrlItems();
    return frequencyCtrlItems;
}

QVector <ProtocolDropControlItem *> * ProtocolItemCtrlManager::getNaturalNumCtrlItems() {
    naturalNumCtrlItems = ctrlPidl->getNaturalNumCtrlItems();
    return naturalNumCtrlItems;
}

ProtocolDropControlItem * ProtocolItemCtrlManager::manageVoltageCtrlConnections(int cbxIdx, ProtocolDropItemDoubleParam * dest) {
    ProtocolDropControlItem * newCtrl = nullptr;
    if (cbxIdx > 0) {
        unsigned int ctrlIdx = (cbxIdx-1)/2;
        newCtrl = voltageCtrlItems->at(ctrlIdx);
        double sign = ((cbxIdx % 2) == 0 ? -1.0 : 1.0);
        dest->setCtrlSign(sign);
        dest->onSetCtrlValue(newCtrl->getDoubleValue());

        if (connections->contains(dest)) {
            ProtocolDropControlItem * oldCtrl = connections->value(dest);
            if (newCtrl != oldCtrl) {
                disconnect(oldCtrl, QOverload <double> ::of(&ProtocolDropControlItem::valueChanged), dest, &ProtocolDropItemDoubleParam::onSetCtrlValue);
                connections->remove(dest);

                connect(newCtrl, QOverload <double> ::of(&ProtocolDropControlItem::valueChanged), dest, &ProtocolDropItemDoubleParam::onSetCtrlValue);
                connections->insert(dest, newCtrl);
            }

        } else {
            connect(newCtrl, QOverload <double> ::of(&ProtocolDropControlItem::valueChanged), dest, &ProtocolDropItemDoubleParam::onSetCtrlValue);
            connections->insert(dest, newCtrl);
        }
    }
    return newCtrl;
}

ProtocolDropControlItem * ProtocolItemCtrlManager::manageCurrentCtrlConnections(int cbxIdx, ProtocolDropItemDoubleParam * dest) {
    ProtocolDropControlItem * newCtrl = nullptr;
    if (cbxIdx > 0) {
        unsigned int ctrlIdx = (cbxIdx-1)/2;
        newCtrl = currentCtrlItems->at(ctrlIdx);
        double sign = ((cbxIdx % 2) == 0 ? -1.0 : 1.0);
        dest->setCtrlSign(sign);
        dest->onSetCtrlValue(newCtrl->getDoubleValue());

        if (connections->contains(dest)) {
            ProtocolDropControlItem * oldCtrl = connections->value(dest);
            if (newCtrl != oldCtrl) {
                disconnect(oldCtrl, QOverload <double> ::of(&ProtocolDropControlItem::valueChanged), dest, &ProtocolDropItemDoubleParam::onSetCtrlValue);
                connections->remove(dest);

                connect(newCtrl, QOverload <double> ::of(&ProtocolDropControlItem::valueChanged), dest, &ProtocolDropItemDoubleParam::onSetCtrlValue);
                connections->insert(dest, newCtrl);
            }

        } else {
            connect(newCtrl, QOverload <double> ::of(&ProtocolDropControlItem::valueChanged), dest, &ProtocolDropItemDoubleParam::onSetCtrlValue);
            connections->insert(dest, newCtrl);
        }
    }
    return newCtrl;
}

ProtocolDropControlItem * ProtocolItemCtrlManager::manageTimeCtrlConnections(int cbxIdx, ProtocolDropItemDoubleParam * dest) {
    ProtocolDropControlItem * newCtrl = nullptr;
    if (cbxIdx > 0) {
        unsigned int ctrlIdx = (cbxIdx-1)/2;
        newCtrl = timeCtrlItems->at(ctrlIdx);
        double sign = ((cbxIdx % 2) == 0 ? -1.0 : 1.0);
        dest->setCtrlSign(sign);
        dest->onSetCtrlValue(newCtrl->getDoubleValue());

        if (connections->contains(dest)) {
            ProtocolDropControlItem * oldCtrl = connections->value(dest);
            if (newCtrl != oldCtrl) {
                disconnect(oldCtrl, QOverload <double> ::of(&ProtocolDropControlItem::valueChanged), dest, &ProtocolDropItemDoubleParam::onSetCtrlValue);
                connections->remove(dest);

                connect(newCtrl, QOverload <double> ::of(&ProtocolDropControlItem::valueChanged), dest, &ProtocolDropItemDoubleParam::onSetCtrlValue);
                connections->insert(dest, newCtrl);
            }

        } else {
            connect(newCtrl, QOverload <double> ::of(&ProtocolDropControlItem::valueChanged), dest, &ProtocolDropItemDoubleParam::onSetCtrlValue);
            connections->insert(dest, newCtrl);
        }
    }
    return newCtrl;
}

ProtocolDropControlItem * ProtocolItemCtrlManager::manageFrequencyCtrlConnections(int cbxIdx, ProtocolDropItemDoubleParam * dest) {
    ProtocolDropControlItem * newCtrl = nullptr;
    if (cbxIdx > 0) {
        unsigned int ctrlIdx = (cbxIdx-1)/2;
        newCtrl = frequencyCtrlItems->at(ctrlIdx);
        double sign = ((cbxIdx % 2) == 0 ? -1.0 : 1.0);
        dest->setCtrlSign(sign);
        dest->onSetCtrlValue(newCtrl->getDoubleValue());

        if (connections->contains(dest)) {
            ProtocolDropControlItem * oldCtrl = connections->value(dest);
            if (newCtrl != oldCtrl) {
                disconnect(oldCtrl, QOverload <double> ::of(&ProtocolDropControlItem::valueChanged), dest, &ProtocolDropItemDoubleParam::onSetCtrlValue);
                connections->remove(dest);

                connect(newCtrl, QOverload <double> ::of(&ProtocolDropControlItem::valueChanged), dest, &ProtocolDropItemDoubleParam::onSetCtrlValue);
                connections->insert(dest, newCtrl);
            }

        } else {
            connect(newCtrl, QOverload <double> ::of(&ProtocolDropControlItem::valueChanged), dest, &ProtocolDropItemDoubleParam::onSetCtrlValue);
            connections->insert(dest, newCtrl);
        }
    }
    return newCtrl;
}

ProtocolDropControlItem * ProtocolItemCtrlManager::manageNaturalNumCtrlConnections(int cbxIdx, ProtocolDropItemIntParam * dest) {
    ProtocolDropControlItem * newCtrl = nullptr;
    if (cbxIdx > 0) {
        unsigned int ctrlIdx = (cbxIdx-1)/2;
        newCtrl = naturalNumCtrlItems->at(ctrlIdx);
        int sign = ((cbxIdx % 2) == 0 ? -1 : 1);
        dest->setCtrlSign(sign);
        dest->onSetCtrlValue(newCtrl->getIntValue());

        if (connections->contains(dest)) {
            ProtocolDropControlItem * oldCtrl = connections->value(dest);
            if (newCtrl != oldCtrl) {
                disconnect(oldCtrl, QOverload <int> ::of(&ProtocolDropControlItem::valueChanged), dest, &ProtocolDropItemIntParam::onSetCtrlValue);
                connections->remove(dest);

                connect(newCtrl, QOverload <int> ::of(&ProtocolDropControlItem::valueChanged), dest, &ProtocolDropItemIntParam::onSetCtrlValue);
                connections->insert(dest, newCtrl);
            }

        } else {
            connect(newCtrl, QOverload <int> ::of(&ProtocolDropControlItem::valueChanged), dest, &ProtocolDropItemIntParam::onSetCtrlValue);
            connections->insert(dest, newCtrl);
        }
    }
    return newCtrl;
}

void ProtocolItemCtrlManager::manageDeletedXStepTStep(ProtocolDropXStepTStepItem * deletedItem) {
    connections->remove(deletedItem->getX0Param());
    connections->remove(deletedItem->getXStepParam());
    connections->remove(deletedItem->getT0Param());
    connections->remove(deletedItem->getTStepParam());
}

void ProtocolItemCtrlManager::manageDeletedXRamp(ProtocolDropXRampItem * deletedItem) {
    connections->remove(deletedItem->getX0Param());
    connections->remove(deletedItem->getXFinalParam());
    connections->remove(deletedItem->getT0Param());
}

void ProtocolItemCtrlManager::manageDeletedXSin(ProtocolDropXSinItem * deletedItem) {
    connections->remove(deletedItem->getX0Param());
    connections->remove(deletedItem->getXAmpParam());
    connections->remove(deletedItem->getFreqParam());
}

void ProtocolItemCtrlManager::manageDeletedLoop(ProtocolDropXRepSeqScaledItem * deletedItem) {
    connections->remove(deletedItem->getHoldLeakParam());
    connections->remove(deletedItem->getScaleFactorParam());
    connections->remove(deletedItem->getRepNumParam());
    connections->remove(deletedItem->getItemNumParam());
    connections->remove(deletedItem->getRestStimulusParam());
    connections->remove(deletedItem->getRestTimeParam());
}

void ProtocolItemCtrlManager::manageDeletedVoltageCtrl(ProtocolDropControlItem * deletedCtrl) {
    ProtocolDropItemDoubleParam * dest;
    QMap <ProtocolDropItemParam *, ProtocolDropControlItem *>::Iterator it;
    for (it = connections->begin(); it != connections->end(); ) {
        if (it.value() == deletedCtrl) {
            dest = static_cast <ProtocolDropItemDoubleParam *> (it.key());
            dest->setValueToEditWidget();
            connections->remove(it++.key());

        } else {
            ++it;
        }
    }
}

void ProtocolItemCtrlManager::manageDeletedCurrentCtrl(ProtocolDropControlItem * deletedCtrl) {
    ProtocolDropItemDoubleParam * dest;
    QMap <ProtocolDropItemParam *, ProtocolDropControlItem *>::Iterator it;
    for (it = connections->begin(); it != connections->end(); ) {
        if (it.value() == deletedCtrl) {
            dest = static_cast <ProtocolDropItemDoubleParam *> (it.key());
            dest->setValueToEditWidget();
            connections->remove(it++.key());

        } else {
            ++it;
        }
    }
}

void ProtocolItemCtrlManager::manageDeletedTimeCtrl(ProtocolDropControlItem * deletedCtrl) {
    ProtocolDropItemDoubleParam * dest;
    QMap <ProtocolDropItemParam *, ProtocolDropControlItem *>::Iterator it;
    for (it = connections->begin(); it != connections->end(); ) {
        if (it.value() == deletedCtrl) {
            dest = static_cast <ProtocolDropItemDoubleParam *> (it.key());
            dest->setValueToEditWidget();
            connections->remove(it++.key());

        } else {
            ++it;
        }
    }
}

void ProtocolItemCtrlManager::manageDeletedFrequencyCtrl(ProtocolDropControlItem * deletedCtrl) {
    ProtocolDropItemDoubleParam * dest;
    QMap <ProtocolDropItemParam *, ProtocolDropControlItem *>::Iterator it;
    for (it = connections->begin(); it != connections->end(); ) {
        if (it.value() == deletedCtrl) {
            dest = static_cast <ProtocolDropItemDoubleParam *> (it.key());
            dest->setValueToEditWidget();
            connections->remove(it++.key());

        } else {
            ++it;
        }
    }
}

void ProtocolItemCtrlManager::manageDeletedNaturalNumCtrl(ProtocolDropControlItem * deletedCtrl) {
    ProtocolDropItemIntParam * dest;
    QMap <ProtocolDropItemParam *, ProtocolDropControlItem *>::Iterator it;
    for (it = connections->begin(); it != connections->end(); ) {
        if (it.value() == deletedCtrl) {
            dest = static_cast <ProtocolDropItemIntParam *> (it.key());
            dest->setValueToEditWidget();
            connections->remove(it++.key());

        } else {
            ++it;
        }
    }
}
