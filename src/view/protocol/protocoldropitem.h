#ifndef PROTOCOLDROPITEM_H
#define PROTOCOLDROPITEM_H

#include <QListWidgetItem>
#include <QGridLayout>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QDialog>
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>

#include "protocolitemctrlmanager.h"
#include "protocoldropitemparam.h"
#include "protocolcursor.h"
#include "globaldefines.h"
#include "control.h"
#include "phase.h"
#include "analysis.h"
#include "messagedispatcher.h"

#define PDI_MAX_PARAMS 16
#define PDI_DESCRIPTION_ROW 0
#define PDI_ANALYSIS_OPTION_ROW (PDI_DESCRIPTION_ROW+1)
#define PDI_FIRST_PARAM_ROW (PDI_ANALYSIS_OPTION_ROW+1)
#define PDI_VISIBLE_CHECK_ROW (PDI_ANALYSIS_OPTION_ROW+PDI_MAX_PARAMS)
#define PDI_OK_CANC_BTN_ROW (PDI_VISIBLE_CHECK_ROW+1)

class ProtocolDropItem : public QObject, public QListWidgetItem {
    Q_OBJECT

public:
    ProtocolDropItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type = PROT_DROP_LIST_WIDGET_ITEM_TYPE);
    virtual ~ProtocolDropItem();

    virtual void openPropertyDialog() = 0;
    bool hasVisibleData();
    ProtocolItemCtrlTypes_t getItemCtrlType();
    ProtocolConsumerType_t getConsumerType();
    virtual QString getName() = 0;
    virtual void setStimulusRange(RangedMeasurement_t &range) = 0;

protected:
    MessageDispatcher * msgDisp = nullptr;
    QDialog * propertyDialog = nullptr;
    QGridLayout * propertyLo;
    QLabel * descriptionName;
    QVector <QWidget *> editWidgets;
    QVector <ProtocolDropItemParam *> dropItemParams;

    QString stimulusAbbrName;
    QString stimulusName;
    ProtocolItemCtrlTypes_t stimulusCtrlType;

    bool visible;
    QCheckBox * visibleEdit;
    ProtocolItemCtrlManager * ctrlManager = nullptr;
    double hold = 0.0;

    RangedMeasurement_t stimulusRange;
    RangedMeasurement_t timeRange;
    int timeDecimals;
    RangedMeasurement_t frequencyRange;
    int frequencyDecimals;

    ProtocolItemCtrlTypes_t protocolItemCtrlType = ProtocolItemCtrlNone;
    ProtocolConsumerType_t protocolConsumerType = ProtocolConsumerNone;

public slots:
    virtual void onSetString() = 0;
    virtual void onAcceptPropertyDialog();
    virtual void onRejectPropertyDialog();
    virtual void onUpdateHold(double value) = 0;

signals:
    void updateItem();
    void analysisChanged();
    void protocolDropItemDeleteRequest(ProtocolDropItem *);
};

class ProtocolDropStimulusItem : public ProtocolDropItem {
public:
    ProtocolDropStimulusItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type = PROT_DROP_LIST_STIMULUS_ITEM_TYPE);

    QString getName() override;
};

class ProtocolDropXStepTStepItem : public ProtocolDropStimulusItem {
    Q_OBJECT

public:
    ProtocolDropXStepTStepItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type);

    void openPropertyDialog() override;
    void setStimulusRange(RangedMeasurement_t &range) override;

    double getX0();
    double getXStep();
    double getT0();
    double getTStep();
    ProtocolDropItemDoubleParam * getX0Param();
    ProtocolDropItemDoubleParam * getXStepParam();
    ProtocolDropItemDoubleParam * getT0Param();
    ProtocolDropItemDoubleParam * getTStepParam();

    YAML::Phase_t getYamlVStepTStep();
    YAML::Phase_t getYamlIStepTStep();
    YAML::Phase_t getYamlVConst();
    YAML::Phase_t getYamlIConst();
    YAML::Phase_t getYamlVHold();
    YAML::Phase_t getYamlIHold();
    YAML::Phase_t getYamlVRest();
    YAML::Phase_t getYamlIRest();

    void setPhaseFromYaml(const YAML::VStepTStep_t &yamlPhase);
    void setPhaseFromYaml(const YAML::IStepTStep_t &yamlPhase);
    void setPhaseFromYaml(const YAML::VConst_t &yamlPhase);
    void setPhaseFromYaml(const YAML::IConst_t &yamlPhase);
    void setPhaseFromYaml(const YAML::VHold_t &yamlPhase);
    void setPhaseFromYaml(const YAML::IHold_t &yamlPhase);
    void setPhaseFromYaml(const YAML::VRest_t &yamlPhase);
    void setPhaseFromYaml(const YAML::IRest_t &yamlPhase);

protected:
    ProtocolDropItemDoubleParam * x0Param = nullptr;
    ProtocolDropItemDoubleParam * xStepParam = nullptr;
    ProtocolDropItemDoubleParam * t0Param = nullptr;
    ProtocolDropItemDoubleParam * tStepParam = nullptr;
    QDoubleSpinBox * x0EditWidget;
    QDoubleSpinBox * xStepEditWidget;

public slots:
    virtual void onSetString() override;
    virtual void onAcceptPropertyDialog() override;
    virtual void onRejectPropertyDialog() override;
    virtual void onUpdateHold(double value) override;
};

class ProtocolDropVStepTStepItem : public ProtocolDropXStepTStepItem {
    Q_OBJECT

public:
    ProtocolDropVStepTStepItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VSTEP_TSTEP_ITEM_TYPE);
};

class ProtocolDropIStepTStepItem : public ProtocolDropXStepTStepItem {
    Q_OBJECT

public:
    ProtocolDropIStepTStepItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_ISTEP_TSTEP_ITEM_TYPE);
};

class ProtocolDropXStepItem : public ProtocolDropXStepTStepItem {
    Q_OBJECT

public:
    ProtocolDropXStepItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVStepItem : public ProtocolDropXStepItem {
    Q_OBJECT

public:
    ProtocolDropVStepItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VSTEP_ITEM_TYPE);
};

class ProtocolDropIStepItem : public ProtocolDropXStepItem {
    Q_OBJECT

public:
    ProtocolDropIStepItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_ISTEP_ITEM_TYPE);
};

class ProtocolDropXTStepItem : public ProtocolDropXStepTStepItem {
    Q_OBJECT

public:
    ProtocolDropXTStepItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVTStepItem : public ProtocolDropXTStepItem {
    Q_OBJECT

public:
    ProtocolDropVTStepItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VTSTEP_ITEM_TYPE);
};

class ProtocolDropITStepItem : public ProtocolDropXTStepItem {
    Q_OBJECT

public:
    ProtocolDropITStepItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_ITSTEP_ITEM_TYPE);
};

class ProtocolDropXConstItem : public ProtocolDropXStepTStepItem {
    Q_OBJECT

public:
    ProtocolDropXConstItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVConstItem : public ProtocolDropXConstItem {
    Q_OBJECT

public:
    ProtocolDropVConstItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VCONST_ITEM_TYPE);
};

class ProtocolDropIConstItem : public ProtocolDropXConstItem {
    Q_OBJECT

public:
    ProtocolDropIConstItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_ICONST_ITEM_TYPE);
};

class ProtocolDropXHoldItem : public ProtocolDropXStepTStepItem {
    Q_OBJECT

public:
    ProtocolDropXHoldItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVHoldItem : public ProtocolDropXHoldItem {
    Q_OBJECT

public:
    ProtocolDropVHoldItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VHOLD_ITEM_TYPE);
};

class ProtocolDropIHoldItem : public ProtocolDropXHoldItem {
    Q_OBJECT

public:
    ProtocolDropIHoldItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_IHOLD_ITEM_TYPE);
};

class ProtocolDropXRestItem : public ProtocolDropXStepTStepItem {
    Q_OBJECT

public:
    ProtocolDropXRestItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVRestItem : public ProtocolDropXRestItem {
    Q_OBJECT

public:
    ProtocolDropVRestItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VREST_ITEM_TYPE);
};

class ProtocolDropIRestItem : public ProtocolDropXRestItem {
    Q_OBJECT

public:
    ProtocolDropIRestItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_IREST_ITEM_TYPE);
};

class ProtocolDropXRampItem : public ProtocolDropStimulusItem {
    Q_OBJECT

public:
    ProtocolDropXRampItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type);

    void openPropertyDialog() override;
    void setStimulusRange(RangedMeasurement_t &range) override;

    double getX0();
    double getXFinal();
    double getT0();
    ProtocolDropItemDoubleParam * getX0Param();
    ProtocolDropItemDoubleParam * getXFinalParam();
    ProtocolDropItemDoubleParam * getT0Param();

    YAML::Phase_t getYamlVRamp();
    YAML::Phase_t getYamlIRamp();

    void setPhaseFromYaml(const YAML::VRamp_t &yamlPhase);
    void setPhaseFromYaml(const YAML::IRamp_t &yamlPhase);

protected:
    ProtocolDropItemDoubleParam * x0Param = nullptr;
    ProtocolDropItemDoubleParam * xFinalParam = nullptr;
    ProtocolDropItemDoubleParam * t0Param = nullptr;
    QDoubleSpinBox * x0EditWidget;
    QDoubleSpinBox * xFinalEditWidget;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVRampItem : public ProtocolDropXRampItem {
    Q_OBJECT

public:
    ProtocolDropVRampItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VRAMP_ITEM_TYPE);
};

class ProtocolDropIRampItem : public ProtocolDropXRampItem {
    Q_OBJECT

public:
    ProtocolDropIRampItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_IRAMP_ITEM_TYPE);
};

class ProtocolDropXSinItem : public ProtocolDropStimulusItem {
    Q_OBJECT

public:
    ProtocolDropXSinItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type = PROT_DROP_LIST_VSIN_ITEM_TYPE);

    void openPropertyDialog() override;
    void setStimulusRange(RangedMeasurement_t &range) override;

    double getX0();
    double getXAmp();
    double getFreq();
    ProtocolDropItemDoubleParam * getX0Param();
    ProtocolDropItemDoubleParam * getXAmpParam();
    ProtocolDropItemDoubleParam * getFreqParam();

    YAML::Phase_t getYamlVSin();
    YAML::Phase_t getYamlISin();

    void setPhaseFromYaml(const YAML::VSin_t &yamlPhase);
    void setPhaseFromYaml(const YAML::ISin_t &yamlPhase);

protected:
    ProtocolDropItemDoubleParam * x0Param = nullptr;
    ProtocolDropItemDoubleParam * xAmpParam = nullptr;
    ProtocolDropItemDoubleParam * freqParam = nullptr;
    QDoubleSpinBox * x0EditWidget;
    QDoubleSpinBox * xAmpEditWidget;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVSinItem : public ProtocolDropXSinItem {
    Q_OBJECT

public:
    ProtocolDropVSinItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VSIN_ITEM_TYPE);
};

class ProtocolDropISinItem : public ProtocolDropXSinItem {
    Q_OBJECT

public:
    ProtocolDropISinItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_ISIN_ITEM_TYPE);
};

class ProtocolDropLoopsItem : public ProtocolDropItem {
public:
    ProtocolDropLoopsItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type = PROT_DROP_LIST_LOOPS_ITEM_TYPE);

    QString getName() override;
};

class ProtocolDropXRepSeqScaledItem : public ProtocolDropLoopsItem {
    Q_OBJECT

public:
    ProtocolDropXRepSeqScaledItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type);

    void openPropertyDialog() override;
    void setStimulusRange(RangedMeasurement_t &range) override;

    double getHoldLeak();
    int getScaleFactor();
    virtual int getRepsNum();
    int getItemNum();
    double getRestStimulus();
    double getRestTime();
    bool getPreceding();
    bool getReversed();
    bool getAlternating();
    bool getApplySteps();
    ProtocolDropItemDoubleParam * getHoldLeakParam();
    ProtocolDropItemIntParam * getScaleFactorParam();
    ProtocolDropItemIntParam * getRepNumParam();
    ProtocolDropItemIntParam * getItemNumParam();
    ProtocolDropItemDoubleParam * getRestStimulusParam();
    ProtocolDropItemDoubleParam * getRestTimeParam();

    YAML::Phase_t getYamlRepSeqScaled();
    YAML::Phase_t getYamlRepSeq();
    YAML::Phase_t getYamlRepSeqWithSteps();
    YAML::Phase_t getYamlInfRepSeq();

    void setPhaseFromYaml(const YAML::RepSeqScaled_t &yamlPhase);
    void setPhaseFromYaml(const YAML::RepSeq_t &yamlPhase);
    void setPhaseFromYaml(const YAML::RepSeqWithSteps_t &yamlPhase);
    void setPhaseFromYaml(const YAML::InfRepSeq_t &yamlPhase);

protected:
    ProtocolDropItemDoubleParam * holdLeakParam = nullptr;
    ProtocolDropItemIntParam * scaleFactorParam = nullptr;
    ProtocolDropItemIntParam * repNumParam = nullptr;
    ProtocolDropItemIntParam * itemNumParam = nullptr;
    ProtocolDropItemDoubleParam * restStimulusParam = nullptr;
    ProtocolDropItemDoubleParam * restTimeParam = nullptr;
    QDoubleSpinBox * holdLeakEditWidget;
    QDoubleSpinBox * restStimulusEditWidget;

    bool preceding;
    bool reversed;
    bool alternating;
    bool applySteps;

    QCheckBox * precedingEdit;
    QCheckBox * reversedEdit;
    QCheckBox * alternatingEdit;

public slots:
    virtual void onSetString() override;
    virtual void onAcceptPropertyDialog() override;
    virtual void onRejectPropertyDialog() override;
    virtual void onUpdateHold(double value) override;
};

class ProtocolDropVRepSeqScaledItem : public ProtocolDropXRepSeqScaledItem {
    Q_OBJECT

public:
    ProtocolDropVRepSeqScaledItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VREP_SEQ_SCALED_ITEM_TYPE);
};

class ProtocolDropIRepSeqScaledItem : public ProtocolDropXRepSeqScaledItem {
    Q_OBJECT

public:
    ProtocolDropIRepSeqScaledItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_IREP_SEQ_SCALED_ITEM_TYPE);
};

class ProtocolDropXRepSeqItem : public ProtocolDropXRepSeqScaledItem {
    Q_OBJECT

public:
    ProtocolDropXRepSeqItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type);

    int getRepsNum() override;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVRepSeqItem : public ProtocolDropXRepSeqItem {
    Q_OBJECT

public:
    ProtocolDropVRepSeqItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VREP_SEQ_ITEM_TYPE);
};

class ProtocolDropIRepSeqItem : public ProtocolDropXRepSeqItem {
    Q_OBJECT

public:
    ProtocolDropIRepSeqItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_IREP_SEQ_ITEM_TYPE);
};

class ProtocolDropXRepSeqWithStepsItem : public ProtocolDropXRepSeqScaledItem {
    Q_OBJECT

public:
    ProtocolDropXRepSeqWithStepsItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type);

    int getRepsNum() override;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVRepSeqWithStepsItem : public ProtocolDropXRepSeqWithStepsItem {
    Q_OBJECT

public:
    ProtocolDropVRepSeqWithStepsItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VREP_SEQ_WITH_STEPS_ITEM_TYPE);
};

class ProtocolDropIRepSeqWithStepsItem : public ProtocolDropXRepSeqWithStepsItem {
    Q_OBJECT

public:
    ProtocolDropIRepSeqWithStepsItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_IREP_SEQ_WITH_STEPS_ITEM_TYPE);
};

class ProtocolDropXInfRepSeqItem : public ProtocolDropXRepSeqScaledItem {
    Q_OBJECT

public:
    ProtocolDropXInfRepSeqItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type);

    int getRepsNum() override;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVInfRepSeqItem : public ProtocolDropXInfRepSeqItem {
    Q_OBJECT

public:
    ProtocolDropVInfRepSeqItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VINF_REP_SEQ_ITEM_TYPE);
};

class ProtocolDropIInfRepSeqItem : public ProtocolDropXInfRepSeqItem {
    Q_OBJECT

public:
    ProtocolDropIInfRepSeqItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_IINF_REP_SEQ_ITEM_TYPE);
};

class ProtocolDropControlItem : public ProtocolDropItem {
    Q_OBJECT

public:
    ProtocolDropControlItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type = PROT_DROP_LIST_CONTROL_ITEM_TYPE);

    void openPropertyDialog() override;

    QString getName() override;
    int getIntValue();
    double getDoubleValue();
    QSpinBox * getIntEdit();
    QDoubleSpinBox * getDoubleEdit();
    QString getUnit();

    YAML::Control_t getYamlVoltageCtrl();
    YAML::Control_t getYamlCurrentCtrl();
    YAML::Control_t getYamlTimeCtrl();
    YAML::Control_t getYamlFrequencyCtrl();
    YAML::Control_t getYamlNaturalNumCtrl();

    void setCtrlFromYaml(const YAML::VoltageCtrl &yamlCtrl);
    void setCtrlFromYaml(const YAML::CurrentCtrl &yamlCtrl);
    void setCtrlFromYaml(const YAML::TimeCtrl &yamlCtrl);
    void setCtrlFromYaml(const YAML::FrequencyCtrl &yamlCtrl);
    void setCtrlFromYaml(const YAML::NaturalNumCtrl &yamlCtrl);

protected:
    QString name;
    int intValue;
    double doubleValue;

    QLineEdit * nameEdit;
    QSpinBox * intEdit;
    QDoubleSpinBox * doubleEdit;

    QLabel * valueUnit;

signals:
    void valueChanged(int);
    void valueChanged(double);

public slots:
    virtual void onSetString() override = 0;
    virtual void onAcceptPropertyDialog() override = 0;
    virtual void onRejectPropertyDialog() override = 0;
    virtual void onUpdateHold(double value) override = 0;
};

class ProtocolDropVoltageControlItem : public ProtocolDropControlItem {
    Q_OBJECT

public:
    ProtocolDropVoltageControlItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VOLTAGE_CONTROL_ITEM_TYPE);
    void setStimulusRange(RangedMeasurement_t &range) override;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropCurrentControlItem : public ProtocolDropControlItem {
    Q_OBJECT

public:
    ProtocolDropCurrentControlItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_CURRENT_CONTROL_ITEM_TYPE);
    void setStimulusRange(RangedMeasurement_t &range) override;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropTimeControlItem : public ProtocolDropControlItem {
    Q_OBJECT

public:
    ProtocolDropTimeControlItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_TIME_CONTROL_ITEM_TYPE);
    void setStimulusRange(RangedMeasurement_t &range) override;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropFrequencyControlItem : public ProtocolDropControlItem {
    Q_OBJECT

public:
    ProtocolDropFrequencyControlItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_FREQUENCY_CONTROL_ITEM_TYPE);
    void setStimulusRange(RangedMeasurement_t &range) override;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropNaturalNumControlItem : public ProtocolDropControlItem {
    Q_OBJECT

public:
    ProtocolDropNaturalNumControlItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_NATURAL_NUM_CONTROL_ITEM_TYPE);
    void setStimulusRange(RangedMeasurement_t &range) override;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropAnalysisItem : public ProtocolDropItem {
    Q_OBJECT

public:
    ProtocolDropAnalysisItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, ClampingModality_t clampingModality, int type = PROT_DROP_LIST_ANALYSIS_ITEM_TYPE);
    ~ProtocolDropAnalysisItem();

    void openPropertyDialog() override;
    QString getName() override;
    void setStimulusRange(RangedMeasurement_t &range) override;

    void addCursors(QVector <ProtocolCursor *> * cursors);
    void removeCursors(QVector <ProtocolCursor *> * cursors, QVector <int> cursorsMap);
    QVector <int> getCursorMapping();

    YAML::Analysis_t getYamlAnalysis();

    void setAnalysisFromYaml(const YAML::Analysis_t &yamlAnalysis);

public slots:
    void onAcceptPropertyDialog() override;
    void setEnabled(bool enabled);
    virtual QString onCheckCursorsValidity();

protected:
    virtual void initializeWidgets();
    virtual void updateTimePointsNum(int timePointsNum);
    bool allTimePointsSet();
    void setValidToolTip();
    void setInvalidProtocolToolTip();
    void setMissingCursorsToolTip();
    void setInvalidPrecedenceToolTip();
    void setInvalidLoopsToolTip();

    bool analysisEnabled = true;
    int timePointsRequired = 0;
    QVector <ProtocolDropAnalysisItemParam *> timePointParams;
    QVector <ProtocolCursor *> * protocolCursors = nullptr;
    QVector <ProtocolCursor *> * analysisCursors = nullptr;

    YAML::AnalysisType_t analysisType;
};

/*! \todo FCON All analyses so far defined as voltage clamp analysis */
class ProtocolDropNoiseReportItem : public ProtocolDropAnalysisItem {
    Q_OBJECT

public:
    ProtocolDropNoiseReportItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_NOISE_REPORT_ITEM_TYPE);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
    QString onCheckCursorsValidity() override;
};

class ProtocolDropHistogramItem : public ProtocolDropAnalysisItem {
    Q_OBJECT

public:
    ProtocolDropHistogramItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_HISTOGRAM_ITEM_TYPE);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
    QString onCheckCursorsValidity() override;
};

class ProtocolDropSpectrumItem : public ProtocolDropAnalysisItem {
    Q_OBJECT

public:
    ProtocolDropSpectrumItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_SPECTRUM_ITEM_TYPE);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
    QString onCheckCursorsValidity() override;
};

class ProtocolDropResistanceEstimationItem : public ProtocolDropAnalysisItem {
    Q_OBJECT

public:
    ProtocolDropResistanceEstimationItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_RESISTANCE_ESTIMATION_ITEM_TYPE);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
    QString onCheckCursorsValidity() override;
};

class ProtocolDropMembraneTestItem : public ProtocolDropAnalysisItem {
    Q_OBJECT

public:
    ProtocolDropMembraneTestItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_MEMBRANE_TEST_ITEM_TYPE);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
    QString onCheckCursorsValidity() override;
};

class ProtocolDropIvGraphItem : public ProtocolDropAnalysisItem {
    Q_OBJECT

public:
    ProtocolDropIvGraphItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_IV_GRAPH_ITEM_TYPE);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
    QString onCheckCursorsValidity() override;

protected:
    void initializeWidgets() override;
    void updateTimePointsNum(int timePointsNum) override;

private:
    void updateTimePointsNumHouseKeeping();

    QSpinBox * intervalsNumSb;
};

class ProtocolDropVoltageTrackingItem : public ProtocolDropAnalysisItem {
    Q_OBJECT

public:
    ProtocolDropVoltageTrackingItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VOLTAGE_TRACKING_ITEM_TYPE);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
    QString onCheckCursorsValidity() override;
};

class ProtocolDropApThresholdItem : public ProtocolDropAnalysisItem {
    Q_OBJECT

public:
    ProtocolDropApThresholdItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_AP_THRESHOLD_ITEM_TYPE);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
    QString onCheckCursorsValidity() override;
};

class ProtocolDropApStatisticsItem : public ProtocolDropAnalysisItem {
    Q_OBJECT

public:
    ProtocolDropApStatisticsItem(MessageDispatcher * msgDisp, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_AP_STATISTICS_ITEM_TYPE);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
    QString onCheckCursorsValidity() override;
};

#endif // PROTOCOLDROPITEM_H
