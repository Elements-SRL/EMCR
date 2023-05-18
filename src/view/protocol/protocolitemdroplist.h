#ifndef PROTOCOITEMDROPLIST_H
#define PROTOCOITEMDROPLIST_H

#include <QListWidget>

#include "protocoldropitem.h"
#include "protocoldragitem.h"
#include "protocolitemctrlmanager.h"
#include "protocolcursor.h"
#include "epmlmanager.h"
#include "globaldefines.h"
#include "control.h"
#include "phase.h"
#include "cursor.h"
#include "analysis.h"
#include "e4gcommlib.h"

namespace e4gcl = e4gCommLib;

class ProtocolItemDropList : public QListWidget {
    Q_OBJECT

public:
    ProtocolItemDropList(e4gcl::CommLib * commLib, QDoubleSpinBox * holdEdit, int clampingModality);
    virtual ~ProtocolItemDropList();

    bool importEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus, int voltageRangeIdx = 0, int currentRangeIdx = 0);
    bool exportEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus);

    void setCtrlManager(ProtocolItemCtrlManager * cm);
    void onUpdateHold(double value);
    QVector <ProtocolDropItem *> * getDropItems();
    QVector <ProtocolDropControlItem *> * getCtrlItems();
    QVector <ProtocolDropControlItem *> * getVoltageCtrlItems();
    QVector <ProtocolDropControlItem *> * getCurrentCtrlItems();
    QVector <ProtocolDropControlItem *> * getTimeCtrlItems();
    QVector <ProtocolDropControlItem *> * getFrequencyCtrlItems();
    QVector <ProtocolDropControlItem *> * getNaturalNumCtrlItems();
    bool analysisRequested(ProtocolConsumerType_t consumerType);
    bool analysisValid(ProtocolConsumerType_t consumerType);
    QVector <int> getAnalysisCursorsMapping(ProtocolConsumerType_t consumerType);
    void setStimulusRange(e4gcl::RangedMeasurement_t &range);

    std::vector <YAML::Control_t> getYamlControls();
    std::vector <YAML::Phase_t> getYamlPhases();
    std::vector <YAML::Analysis_t> getYamlAnalyses();

    void setControlsFromYaml(const std::vector <YAML::Control_t> &yamlControls, int voltageRangeIdx, int currentRangeIdx);
    void setPhasesFromYaml(const std::vector <YAML::Phase_t> &yamlPhases, int voltageRangeIdx, int currentRangeIdx);
    void setAnalysesFromYaml(const std::vector <YAML::Analysis_t> &yamlAnalyses);

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
    void setAnalysis(ProtocolConsumerType_t consumerType, ProtocolDropAnalysisItem * item);
    void resetAnalysis(ProtocolConsumerType_t consumerType);

    virtual void dragEnterEvent(QDragEnterEvent * event) override;
    virtual void dragMoveEvent(QDragMoveEvent * event) override;
    virtual void dropEvent(QDropEvent * event) override;
    virtual bool eventFilter(QObject * obj, QEvent * event) override;
    void contextMenuEvent(QContextMenuEvent * event) override;
    void manageItemDelete(ProtocolDropItem * item);
    void manageCtrlDelete(ProtocolDropItem * item);
    void manageAnalysisDelete(ProtocolDropItem * item);
    virtual bool acceptedMimeDataFormat(const QMimeData * mimeData) = 0;

    e4gcl::CommLib * commLib;
    QDoubleSpinBox * holdEdit;
    int clampingModality;
    QVector <ProtocolDropItem *> * items;
    QVector <ProtocolDropControlItem *> * ctrlItems;
    QVector <ProtocolDropControlItem *> * voltageCtrlItems;
    QVector <ProtocolDropControlItem *> * currentCtrlItems;
    QVector <ProtocolDropControlItem *> * timeCtrlItems;
    QVector <ProtocolDropControlItem *> * frequencyCtrlItems;
    QVector <ProtocolDropControlItem *> * naturalNumCtrlItems;
    QVector <ProtocolDropAnalysisItem *> * analysisItems;
    ProtocolItemCtrlManager * ctrlManager;
    QString stimulusAbbrName;

    QAction * editAct;
    QAction * deleteAct;

private:
    void createActions();

signals:
    void updateProtocol();
    void updateHold(double);
    void requestCursors(ProtocolDropAnalysisItem *);
    void analysisChanged();
};

class GapfreeProtocolItemDropList : public ProtocolItemDropList {
    Q_OBJECT

public:
    GapfreeProtocolItemDropList(e4gcl::CommLib * commLib, QDoubleSpinBox * holdEdit, int clampingModality);

protected:
    virtual bool acceptedMimeDataFormat(const QMimeData * mimeData) override;
};

class EpisodicProtocolItemDropList : public ProtocolItemDropList {
    Q_OBJECT

public:
    EpisodicProtocolItemDropList(e4gcl::CommLib * commLib, QDoubleSpinBox * holdEdit, int clampingModality);

protected:
    virtual bool acceptedMimeDataFormat(const QMimeData * mimeData) override;
};

class CtrlProtocolItemDropList : public ProtocolItemDropList {
    Q_OBJECT

public:
    CtrlProtocolItemDropList(e4gcl::CommLib * commLib, QDoubleSpinBox * holdEdit, int clampingModality);

protected:
    virtual bool acceptedMimeDataFormat(const QMimeData * mimeData) override;
};

class AnalysisProtocolItemDropList : public ProtocolItemDropList {
    Q_OBJECT

public:
    AnalysisProtocolItemDropList(e4gcl::CommLib * commLib, QDoubleSpinBox * holdEdit, int clampingModality);

    void addCursors(QVector <ProtocolCursor *> * cursors);
    void removeCursors(QVector <ProtocolCursor *> * cursors, QVector <int> cursorsMap);
    void updateCursors();
    void enableAnalysis(bool enabled);

protected:
    virtual bool acceptedMimeDataFormat(const QMimeData * mimeData) override;
};

#endif // PROTOCOITEMDROPLIST_H
