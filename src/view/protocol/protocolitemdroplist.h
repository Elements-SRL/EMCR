#ifndef PROTOCOITEMDROPLIST_H
#define PROTOCOITEMDROPLIST_H

#include <QListWidget>

#include "protocoldropitem.h"
#include "protocoldragitem.h"
#include "protocolitemctrlmanager.h"
#include "protocolcursor.h"
#include "globaldefines.h"
#include "control.h"
#include "phase.h"
#include "cursor.h"
#include "messagedispatcher.h"

class ProtocolItemDropList : public QListWidget {
    Q_OBJECT

public:
    ProtocolItemDropList(MessageDispatcher * msgDisp, QDoubleSpinBox * holdEdit, ClampingModality_t clampingModality);
    virtual ~ProtocolItemDropList();

    void setCtrlManager(ProtocolItemCtrlManager * cm);
    void onUpdateHold(double value);
    QVector <ProtocolDropItem *> * getDropItems();
    QVector <ProtocolDropControlItem *> * getCtrlItems();
    QVector <ProtocolDropControlItem *> * getVoltageCtrlItems();
    QVector <ProtocolDropControlItem *> * getCurrentCtrlItems();
    QVector <ProtocolDropControlItem *> * getTimeCtrlItems();
    QVector <ProtocolDropControlItem *> * getFrequencyCtrlItems();
    QVector <ProtocolDropControlItem *> * getNaturalNumCtrlItems();
    void setStimulusRange(RangedMeasurement_t &range);

    std::vector <YAML::Control_t> getYamlControls();
    std::vector <YAML::Phase_t> getYamlPhases();

    void setControlsFromYaml(const std::vector <YAML::Control_t> &yamlControls, int voltageRangeIdx, int currentRangeIdx);
    void setPhasesFromYaml(const std::vector <YAML::Phase_t> &yamlPhases, int voltageRangeIdx, int currentRangeIdx);

public slots:
    void onItemDoubleClicked(QListWidgetItem * item);
    void onUpdateItem();
    void onDropItemDelete(ProtocolDropItem * item);

protected:
    void updateDropItemsList();
    void updateCtrlItemsList();
    void updateVoltageCtrlItemsList();
    void updateCurrentCtrlItemsList();
    void updateTimeCtrlItemsList();
    void updateFrequencyCtrlItemsList();
    void updateNaturalNumCtrlItemsList();

    virtual void dragEnterEvent(QDragEnterEvent * event) override;
    virtual void dragMoveEvent(QDragMoveEvent * event) override;
    virtual void dropEvent(QDropEvent * event) override;
    virtual bool eventFilter(QObject * obj, QEvent * event) override;
    void contextMenuEvent(QContextMenuEvent * event) override;
    void manageItemDelete(ProtocolDropItem * item);
    void manageCtrlDelete(ProtocolDropItem * item);
    virtual bool acceptedMimeDataFormat(const QMimeData * mimeData) = 0;

    MessageDispatcher * msgDisp = nullptr;
    QDoubleSpinBox * holdEdit;
    ClampingModality_t clampingModality;
    QVector <ProtocolDropItem *> * items;
    QVector <ProtocolDropControlItem *> * ctrlItems;
    QVector <ProtocolDropControlItem *> * voltageCtrlItems;
    QVector <ProtocolDropControlItem *> * currentCtrlItems;
    QVector <ProtocolDropControlItem *> * timeCtrlItems;
    QVector <ProtocolDropControlItem *> * frequencyCtrlItems;
    QVector <ProtocolDropControlItem *> * naturalNumCtrlItems;
    ProtocolItemCtrlManager * ctrlManager;
    QString stimulusAbbrName;

    QAction * editAct;
    QAction * deleteAct;

private:
    void createActions();

signals:
    void updateProtocol();
    void updateHold(double);
};

class GapfreeProtocolItemDropList : public ProtocolItemDropList {
    Q_OBJECT

public:
    GapfreeProtocolItemDropList(MessageDispatcher * msgDisp, QDoubleSpinBox * holdEdit, ClampingModality_t clampingModality);

protected:
    virtual bool acceptedMimeDataFormat(const QMimeData * mimeData) override;
};

class EpisodicProtocolItemDropList : public ProtocolItemDropList {
    Q_OBJECT

public:
    EpisodicProtocolItemDropList(MessageDispatcher * msgDisp, QDoubleSpinBox * holdEdit, ClampingModality_t clampingModality);

protected:
    virtual bool acceptedMimeDataFormat(const QMimeData * mimeData) override;
};

class CtrlProtocolItemDropList : public ProtocolItemDropList {
    Q_OBJECT

public:
    CtrlProtocolItemDropList(MessageDispatcher * msgDisp, QDoubleSpinBox * holdEdit, ClampingModality_t clampingModality);

protected:
    virtual bool acceptedMimeDataFormat(const QMimeData * mimeData) override;
};

#endif // PROTOCOITEMDROPLIST_H
