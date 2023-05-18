#ifndef PROTOCOLEDITOR_H
#define PROTOCOLEDITOR_H

#include <QDialog>

#include "protocoldragitem.h"
#include "protocoldropitem.h"
#include "protocolitemdraglist.h"
#include "protocolitemdroplist.h"
#include "protocolpreview.h"
#include "protocolitemctrlmanager.h"
#include "voltageprotocol.h"
#include "currentprotocol.h"

#define PTE_HOLD_ROW 0
#define PTE_HOLDREF_ROW (PTE_HOLD_ROW+1)
#define PTE_SWEEPS_ROW (PTE_HOLDREF_ROW+1)
#define PTE_CURRENT_RANGE_ROW (PTE_SWEEPS_ROW+1)
#define PTE_VOLTAGE_RANGE_ROW (PTE_CURRENT_RANGE_ROW+1)
#define PTE_SAMPLING_RATE_ROW (PTE_VOLTAGE_RANGE_ROW+1)

#define PTE_PHASES_COLUMN_IDX 1

class ProtocolWidget;

class ProtocolEditor : public QDialog {
    Q_OBJECT

public:
    ProtocolEditor(ModelDevice *  mDev, ProtocolWidget * protocolWidget, QString name);
    ProtocolEditor();
    virtual ~ProtocolEditor();

    QVector <ProtocolDropControlItem *> * getCtrlItems();
    bool analysisRequested(ProtocolConsumerType_t consumerType);
    bool analysisValid(ProtocolConsumerType_t consumerType);
    bool allAnalysisValid();
    QVector <int> getAnalysisCursorsMapping(ProtocolConsumerType_t consumerType);

    void setName(QString value);
    void setHold(double value);
    void setSweepsNum(int value);
    QString getName();
    double getHold();
    QDoubleSpinBox * getHoldEdit();
    void setHoldingDelta(Measurement_t &holdingDelta);
    bool getHoldRef();
    QCheckBox * getHoldRefEdit();
    int getSweepsNum();
    QSpinBox * getSweepsNumEdit();
    virtual int getCurrentRangeIndex() = 0;
    virtual QComboBox * getCurrentRangeEdit() = 0;
    virtual int getVoltageRangeIndex() = 0;
    virtual QComboBox * getVoltageRangeEdit() = 0;
    virtual void setHoldingRange() = 0;
    int getSamplingRateIndex();
    QComboBox * getSamplingRateEdit();
    QVector <ProtocolDropItem *> * getDropItems();
    void setTooManyTriggersWarning(bool flag);

    YAML::VoltageProtocol_t getYamlVoltageProtocol();
    YAML::CurrentProtocol_t getYamlCurrentProtocol();

    void setProtocolFromYaml(const YAML::VoltageProtocol_t &yamlProtocol);
    void setProtocolFromYaml(const YAML::CurrentProtocol_t &yamlProtocol);

public slots:
    void onUpdateProtocol();
    void onUpdateCtrlItem();
    void onUpdateHold(double value);
    void onUpdateHoldRef();
    void onStimulusRangeSelected(int rangeIdx);

protected:
    virtual void stimulusRangeSelected(int rangeIdx) = 0;

    ModelDevice *  mDev;
    ProtocolWidget * parentWidget;
    QVBoxLayout * mainVl;
    QHBoxLayout * editorHl;
    QVBoxLayout * phasesVl;
    QGridLayout * protocolWideCtrlsGl;
    QFont titlesFont;
    ProtocolItemDragList * libraryPidl = nullptr;
    ProtocolItemDropList * phasesPidl = nullptr;
    ProtocolItemDropList * ctrlPidl = nullptr;
    ProtocolItemDropList * analysisPidl = nullptr;
    ProtocolItemCtrlManager * protocolItemCtrlManager = nullptr;
    ProtocolPreview * protocolPreview = nullptr;

    QString name;
    QString type;
    QString stimulusAbbrName;

    QDoubleSpinBox * holdEdit;
    QCheckBox * holdRefEdit;
    QLabel * sweepsNumName;
    QSpinBox * sweepsNumEdit;
    QComboBox * currentRangeEdit = nullptr;
    QComboBox * voltageRangeEdit = nullptr;
    QComboBox * samplingRateEdit = nullptr;
};

class VoltageProtocolEditor : virtual public ProtocolEditor {
public:
    VoltageProtocolEditor();

    int getCurrentRangeIndex() override;
    QComboBox * getCurrentRangeEdit() override;
    int getVoltageRangeIndex() override;
    QComboBox * getVoltageRangeEdit() override;
    void setHoldingRange() override;

protected:
    void stimulusRangeSelected(int rangeIdx) override;
};

class CurrentProtocolEditor : virtual public ProtocolEditor {
public:
    CurrentProtocolEditor();

    int getCurrentRangeIndex() override;
    QComboBox * getCurrentRangeEdit() override;
    int getVoltageRangeIndex() override;
    QComboBox * getVoltageRangeEdit() override;
    void setHoldingRange() override;

protected:
    void stimulusRangeSelected(int rangeIdx) override;
};

class GapfreeProtocolEditor : virtual public ProtocolEditor {
public:
    GapfreeProtocolEditor();
};

class EpisodicProtocolEditor : virtual public ProtocolEditor {
public:
    EpisodicProtocolEditor();
};

class GapfreeVoltageProtocolEditor : public VoltageProtocolEditor, public GapfreeProtocolEditor {
    Q_OBJECT

public:
    GapfreeVoltageProtocolEditor(ModelDevice *  mDev, ProtocolWidget * protocolWidget, QString name);
};

class EpisodicVoltageProtocolEditor : public VoltageProtocolEditor, public EpisodicProtocolEditor {
    Q_OBJECT

public:
    EpisodicVoltageProtocolEditor(ModelDevice *  mDev, ProtocolWidget * protocolWidget, QString name);
};

class GapfreeCurrentProtocolEditor : public CurrentProtocolEditor, public GapfreeProtocolEditor {
    Q_OBJECT

public:
    GapfreeCurrentProtocolEditor(ModelDevice *  mDev, ProtocolWidget * protocolWidget, QString name);
};

class EpisodicCurrentProtocolEditor : public CurrentProtocolEditor, public EpisodicProtocolEditor {
    Q_OBJECT

public:
    EpisodicCurrentProtocolEditor(ModelDevice *  mDev, ProtocolWidget * protocolWidget, QString name);
};

#endif // PROTOCOLEDITOR_H
