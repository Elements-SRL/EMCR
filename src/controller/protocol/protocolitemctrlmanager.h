#ifndef PROTOCOLITEMCTRLMANAGER_H
#define PROTOCOLITEMCTRLMANAGER_H

#include <QObject>
#include <QMap>

class ProtocolItemDropList;
class ProtocolDropItem;
class ProtocolDropXStepTStepItem;
class ProtocolDropXRampItem;
class ProtocolDropXSinItem;
class ProtocolDropXRepSeqScaledItem;
class ProtocolDropControlItem;
class ProtocolDropItemParam;
class ProtocolDropItemIntParam;
class ProtocolDropItemDoubleParam;

using namespace std;

class ProtocolItemCtrlManager : public QObject {
    Q_OBJECT

public:
    ProtocolItemCtrlManager(ProtocolItemDropList * pidl);

    ProtocolDropControlItem * manageVoltageCtrlConnections(int cbxIdx, ProtocolDropItemDoubleParam * dest);
    ProtocolDropControlItem * manageCurrentCtrlConnections(int cbxIdx, ProtocolDropItemDoubleParam * dest);
    ProtocolDropControlItem * manageTimeCtrlConnections(int cbxIdx, ProtocolDropItemDoubleParam * dest);
    ProtocolDropControlItem * manageFrequencyCtrlConnections(int cbxIdx, ProtocolDropItemDoubleParam * dest);
    ProtocolDropControlItem * manageNaturalNumCtrlConnections(int cbxIdx, ProtocolDropItemIntParam * dest);

    double getVoltageCtrlValue(int cbxIdx);
    double getCurrentCtrlValue(int cbxIdx);
    double getTimeCtrlValue(int cbxIdx);
    double getFrequencyCtrlValue(int cbxIdx);
    int getNaturalNumCtrlValue(int cbxIdx);

    QVector <ProtocolDropControlItem *> * getVoltageCtrlItems();
    QVector <ProtocolDropControlItem *> * getCurrentCtrlItems();
    QVector <ProtocolDropControlItem *> * getTimeCtrlItems();
    QVector <ProtocolDropControlItem *> * getFrequencyCtrlItems();
    QVector <ProtocolDropControlItem *> * getNaturalNumCtrlItems();

    void manageDeletedXStepTStep(ProtocolDropXStepTStepItem * deletedItem);
    void manageDeletedXRamp(ProtocolDropXRampItem * deletedItem);
    void manageDeletedXSin(ProtocolDropXSinItem * deletedItem);
    void manageDeletedLoop(ProtocolDropXRepSeqScaledItem * deletedItem);

    void manageDeletedVoltageCtrl(ProtocolDropControlItem * deletedCtrl);
    void manageDeletedCurrentCtrl(ProtocolDropControlItem * deletedCtrl);
    void manageDeletedTimeCtrl(ProtocolDropControlItem * deletedCtrl);
    void manageDeletedFrequencyCtrl(ProtocolDropControlItem * deletedCtrl);
    void manageDeletedNaturalNumCtrl(ProtocolDropControlItem * deletedCtrl);

private:
    ProtocolItemDropList * ctrlPidl;

    QMap <ProtocolDropItemParam *, ProtocolDropControlItem *> * connections;

    QVector <ProtocolDropControlItem *> * voltageCtrlItems;
    QVector <ProtocolDropControlItem *> * currentCtrlItems;
    QVector <ProtocolDropControlItem *> * timeCtrlItems;
    QVector <ProtocolDropControlItem *> * frequencyCtrlItems;
    QVector <ProtocolDropControlItem *> * naturalNumCtrlItems;
};

#endif // PROTOCOLITEMCTRLMANAGER_H
