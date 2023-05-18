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
#include "steppedspinbox.h"
#include "epmlmanager.h"
#include "globaldefines.h"
#include "e4gcommlib.h"

namespace e4gcl = e4gCommLib;

#define PDI_MAX_PARAMS 16
#define PDI_DESCRIPTION_ROW 0
#define PDI_ANALYSIS_OPTION_ROW (PDI_DESCRIPTION_ROW+1)
#define PDI_FIRST_PARAM_ROW (PDI_ANALYSIS_OPTION_ROW+1)
#define PDI_VISIBLE_CHECK_ROW (PDI_ANALYSIS_OPTION_ROW+PDI_MAX_PARAMS)
#define PDI_OK_CANC_BTN_ROW (PDI_VISIBLE_CHECK_ROW+1)

class ProtocolDropItem : public QObject, public QListWidgetItem {
    Q_OBJECT

public:
    ProtocolDropItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type = PROT_DROP_LIST_WIDGET_ITEM_TYPE);
    virtual ~ProtocolDropItem();

    virtual bool importEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus) = 0;
    virtual bool exportEpml(EpmlManager * epmlManager, EpmlStatus_t &epmlStatus) = 0;
    virtual void openPropertyDialog() = 0;
    bool hasVisibleData();
    ProtocolItemCtrlTypes_t getItemCtrlType();
    ProtocolConsumerType_t getConsumerType();
    virtual QString getName() = 0;
    virtual void setStimulusRange(e4gcl::RangedMeasurement_t &range) = 0;

protected:
    e4gcl::CommLib * commLib;
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

    e4gcl::RangedMeasurement_t stimulusRange;
    e4gcl::RangedMeasurement_t timeRange;
    int timeDecimals;
    e4gcl::RangedMeasurement_t frequencyRange;
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
    ProtocolDropStimulusItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type = PROT_DROP_LIST_STIMULUS_ITEM_TYPE);

    QString getName() override;
};

class ProtocolDropXStepTStepItem : public ProtocolDropStimulusItem {
    Q_OBJECT

public:
    ProtocolDropXStepTStepItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type);

    bool importEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus) override;
    bool exportEpml(EpmlManager * epmlManager, EpmlStatus_t &epmlStatus) override;
    void openPropertyDialog() override;
    void setStimulusRange(e4gcl::RangedMeasurement_t &range) override;

    double getX0();
    double getXStep();
    double getT0();
    double getTStep();
    ProtocolDropItemDoubleParam * getX0Param();
    ProtocolDropItemDoubleParam * getXStepParam();
    ProtocolDropItemDoubleParam * getT0Param();
    ProtocolDropItemDoubleParam * getTStepParam();

protected:
    ProtocolDropItemDoubleParam * x0Param = nullptr;
    ProtocolDropItemDoubleParam * xStepParam = nullptr;
    ProtocolDropItemDoubleParam * t0Param = nullptr;
    ProtocolDropItemDoubleParam * tStepParam = nullptr;
    SteppedSpinBox * x0EditWidget;
    SteppedSpinBox * xStepEditWidget;

public slots:
    virtual void onSetString() override;
    virtual void onAcceptPropertyDialog() override;
    virtual void onRejectPropertyDialog() override;
    virtual void onUpdateHold(double value) override;
};

class ProtocolDropVStepTStepItem : public ProtocolDropXStepTStepItem {
    Q_OBJECT

public:
    ProtocolDropVStepTStepItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VSTEP_TSTEP_ITEM_TYPE);
};

class ProtocolDropIStepTStepItem : public ProtocolDropXStepTStepItem {
    Q_OBJECT

public:
    ProtocolDropIStepTStepItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_ISTEP_TSTEP_ITEM_TYPE);
};

class ProtocolDropXStepItem : public ProtocolDropXStepTStepItem {
    Q_OBJECT

public:
    ProtocolDropXStepItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVStepItem : public ProtocolDropXStepItem {
    Q_OBJECT

public:
    ProtocolDropVStepItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VSTEP_ITEM_TYPE);
};

class ProtocolDropIStepItem : public ProtocolDropXStepItem {
    Q_OBJECT

public:
    ProtocolDropIStepItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_ISTEP_ITEM_TYPE);
};

class ProtocolDropXTStepItem : public ProtocolDropXStepTStepItem {
    Q_OBJECT

public:
    ProtocolDropXTStepItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVTStepItem : public ProtocolDropXTStepItem {
    Q_OBJECT

public:
    ProtocolDropVTStepItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VTSTEP_ITEM_TYPE);
};

class ProtocolDropITStepItem : public ProtocolDropXTStepItem {
    Q_OBJECT

public:
    ProtocolDropITStepItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_ITSTEP_ITEM_TYPE);
};

class ProtocolDropXConstItem : public ProtocolDropXStepTStepItem {
    Q_OBJECT

public:
    ProtocolDropXConstItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVConstItem : public ProtocolDropXConstItem {
    Q_OBJECT

public:
    ProtocolDropVConstItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VCONST_ITEM_TYPE);
};

class ProtocolDropIConstItem : public ProtocolDropXConstItem {
    Q_OBJECT

public:
    ProtocolDropIConstItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_ICONST_ITEM_TYPE);
};

class ProtocolDropXHoldItem : public ProtocolDropXStepTStepItem {
    Q_OBJECT

public:
    ProtocolDropXHoldItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVHoldItem : public ProtocolDropXHoldItem {
    Q_OBJECT

public:
    ProtocolDropVHoldItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VHOLD_ITEM_TYPE);
};

class ProtocolDropIHoldItem : public ProtocolDropXHoldItem {
    Q_OBJECT

public:
    ProtocolDropIHoldItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_IHOLD_ITEM_TYPE);
};

class ProtocolDropXRestItem : public ProtocolDropXStepTStepItem {
    Q_OBJECT

public:
    ProtocolDropXRestItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVRestItem : public ProtocolDropXRestItem {
    Q_OBJECT

public:
    ProtocolDropVRestItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VREST_ITEM_TYPE);
};

class ProtocolDropIRestItem : public ProtocolDropXRestItem {
    Q_OBJECT

public:
    ProtocolDropIRestItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_IREST_ITEM_TYPE);
};

class ProtocolDropXRampItem : public ProtocolDropStimulusItem {
    Q_OBJECT

public:
    ProtocolDropXRampItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type);

    bool importEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus) override;
    bool exportEpml(EpmlManager * epmlManager, EpmlStatus_t &epmlStatus) override;
    void openPropertyDialog() override;
    void setStimulusRange(e4gcl::RangedMeasurement_t &range) override;

    double getX0();
    double getXFinal();
    double getT0();
    ProtocolDropItemDoubleParam * getX0Param();
    ProtocolDropItemDoubleParam * getXFinalParam();
    ProtocolDropItemDoubleParam * getT0Param();

protected:
    ProtocolDropItemDoubleParam * x0Param = nullptr;
    ProtocolDropItemDoubleParam * xFinalParam = nullptr;
    ProtocolDropItemDoubleParam * t0Param = nullptr;
    SteppedSpinBox * x0EditWidget;
    SteppedSpinBox * xFinalEditWidget;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVRampItem : public ProtocolDropXRampItem {
    Q_OBJECT

public:
    ProtocolDropVRampItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VRAMP_ITEM_TYPE);
};

class ProtocolDropIRampItem : public ProtocolDropXRampItem {
    Q_OBJECT

public:
    ProtocolDropIRampItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_IRAMP_ITEM_TYPE);
};

class ProtocolDropXSinItem : public ProtocolDropStimulusItem {
    Q_OBJECT

public:
    ProtocolDropXSinItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type = PROT_DROP_LIST_VSIN_ITEM_TYPE);

    bool importEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus) override;
    bool exportEpml(EpmlManager * epmlManager, EpmlStatus_t &epmlStatus) override;
    void openPropertyDialog() override;
    void setStimulusRange(e4gcl::RangedMeasurement_t &range) override;

    double getX0();
    double getXAmp();
    double getFreq();
    ProtocolDropItemDoubleParam * getX0Param();
    ProtocolDropItemDoubleParam * getXAmpParam();
    ProtocolDropItemDoubleParam * getFreqParam();

protected:
    ProtocolDropItemDoubleParam * x0Param = nullptr;
    ProtocolDropItemDoubleParam * xAmpParam = nullptr;
    ProtocolDropItemDoubleParam * freqParam = nullptr;
    SteppedSpinBox * x0EditWidget;
    SteppedSpinBox * xAmpEditWidget;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropVSinItem : public ProtocolDropXSinItem {
    Q_OBJECT

public:
    ProtocolDropVSinItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VSIN_ITEM_TYPE);
};

class ProtocolDropISinItem : public ProtocolDropXSinItem {
    Q_OBJECT

public:
    ProtocolDropISinItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_ISIN_ITEM_TYPE);
};

class ProtocolDropLoopsItem : public ProtocolDropItem {
public:
    ProtocolDropLoopsItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type = PROT_DROP_LIST_LOOPS_ITEM_TYPE);

    QString getName() override;
};

class ProtocolDropXRepSeqScaledItem : public ProtocolDropLoopsItem {
    Q_OBJECT

public:
    ProtocolDropXRepSeqScaledItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type);

    bool importEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus) override;
    bool exportEpml(EpmlManager * epmlManager, EpmlStatus_t &epmlStatus) override;
    void openPropertyDialog() override;
    void setStimulusRange(e4gcl::RangedMeasurement_t &range) override;

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

protected:
    ProtocolDropItemDoubleParam * holdLeakParam = nullptr;
    ProtocolDropItemIntParam * scaleFactorParam = nullptr;
    ProtocolDropItemIntParam * repNumParam = nullptr;
    ProtocolDropItemIntParam * itemNumParam = nullptr;
    ProtocolDropItemDoubleParam * restStimulusParam = nullptr;
    ProtocolDropItemDoubleParam * restTimeParam = nullptr;
    SteppedSpinBox * holdLeakEditWidget;
    SteppedSpinBox * restStimulusEditWidget;

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
    ProtocolDropVRepSeqScaledItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VREP_SEQ_SCALED_ITEM_TYPE);
};

class ProtocolDropIRepSeqScaledItem : public ProtocolDropXRepSeqScaledItem {
    Q_OBJECT

public:
    ProtocolDropIRepSeqScaledItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_IREP_SEQ_SCALED_ITEM_TYPE);
};

class ProtocolDropXRepSeqItem : public ProtocolDropXRepSeqScaledItem {
    Q_OBJECT

public:
    ProtocolDropXRepSeqItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type);

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
    ProtocolDropVRepSeqItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VREP_SEQ_ITEM_TYPE);
};

class ProtocolDropIRepSeqItem : public ProtocolDropXRepSeqItem {
    Q_OBJECT

public:
    ProtocolDropIRepSeqItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_IREP_SEQ_ITEM_TYPE);
};

class ProtocolDropXRepSeqWithStepsItem : public ProtocolDropXRepSeqScaledItem {
    Q_OBJECT

public:
    ProtocolDropXRepSeqWithStepsItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type);

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
    ProtocolDropVRepSeqWithStepsItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VREP_SEQ_WITH_STEPS_ITEM_TYPE);
};

class ProtocolDropIRepSeqWithStepsItem : public ProtocolDropXRepSeqWithStepsItem {
    Q_OBJECT

public:
    ProtocolDropIRepSeqWithStepsItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_IREP_SEQ_WITH_STEPS_ITEM_TYPE);
};

class ProtocolDropXInfRepSeqItem : public ProtocolDropXRepSeqScaledItem {
    Q_OBJECT

public:
    ProtocolDropXInfRepSeqItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type);

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
    ProtocolDropVInfRepSeqItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VINF_REP_SEQ_ITEM_TYPE);
};

class ProtocolDropIInfRepSeqItem : public ProtocolDropXInfRepSeqItem {
    Q_OBJECT

public:
    ProtocolDropIInfRepSeqItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_IINF_REP_SEQ_ITEM_TYPE);
};

class ProtocolDropControlItem : public ProtocolDropItem {
    Q_OBJECT

public:
    ProtocolDropControlItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type = PROT_DROP_LIST_CONTROL_ITEM_TYPE);

    bool importEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus) override;
    bool exportEpml(EpmlManager * epmlManager, EpmlStatus_t &epmlStatus) override;
    void openPropertyDialog() override;

    QString getName() override;
    int getIntValue();
    double getDoubleValue();
    QSpinBox * getIntEdit();
    SteppedSpinBox * getDoubleEdit();
    QString getUnit();

protected:
    QString name;
    int intValue;
    double doubleValue;

    QLineEdit * nameEdit;
    QSpinBox * intEdit;
    SteppedSpinBox * doubleEdit;

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
    ProtocolDropVoltageControlItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VOLTAGE_CONTROL_ITEM_TYPE);
    void setStimulusRange(e4gcl::RangedMeasurement_t &range) override;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropCurrentControlItem : public ProtocolDropControlItem {
    Q_OBJECT

public:
    ProtocolDropCurrentControlItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_CURRENT_CONTROL_ITEM_TYPE);
    void setStimulusRange(e4gcl::RangedMeasurement_t &range) override;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropTimeControlItem : public ProtocolDropControlItem {
    Q_OBJECT

public:
    ProtocolDropTimeControlItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_TIME_CONTROL_ITEM_TYPE);
    void setStimulusRange(e4gcl::RangedMeasurement_t &range) override;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropFrequencyControlItem : public ProtocolDropControlItem {
    Q_OBJECT

public:
    ProtocolDropFrequencyControlItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_FREQUENCY_CONTROL_ITEM_TYPE);
    void setStimulusRange(e4gcl::RangedMeasurement_t &range) override;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropNaturalNumControlItem : public ProtocolDropControlItem {
    Q_OBJECT

public:
    ProtocolDropNaturalNumControlItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_NATURAL_NUM_CONTROL_ITEM_TYPE);
    void setStimulusRange(e4gcl::RangedMeasurement_t &range) override;

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
};

class ProtocolDropAnalysisItem : public ProtocolDropItem {
    Q_OBJECT

public:
    ProtocolDropAnalysisItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int clampingModality, int type = PROT_DROP_LIST_ANALYSIS_ITEM_TYPE);
    ~ProtocolDropAnalysisItem();

    bool importEpml(EpmlManager * epmlManager, QString parentTag, EpmlStatus_t &epmlStatus) override;
    bool exportEpml(EpmlManager * epmlManager, EpmlStatus_t &epmlStatus) override;
    void openPropertyDialog() override;
    QString getName() override;
    void setStimulusRange(e4gcl::RangedMeasurement_t &range) override;

    void addCursors(QVector <ProtocolCursor *> * cursors);
    void removeCursors(QVector <ProtocolCursor *> * cursors, QVector <int> cursorsMap);
    QVector <int> getCursorMapping();

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
};

/*! \todo FCON All analyses so far defined as voltage clamp analysis */
class ProtocolDropNoiseReportItem : public ProtocolDropAnalysisItem {
    Q_OBJECT

public:
    ProtocolDropNoiseReportItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_NOISE_REPORT_ITEM_TYPE);

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
    ProtocolDropHistogramItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_HISTOGRAM_ITEM_TYPE);

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
    ProtocolDropSpectrumItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_SPECTRUM_ITEM_TYPE);

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
    ProtocolDropResistanceEstimationItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_RESISTANCE_ESTIMATION_ITEM_TYPE);

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
    ProtocolDropMembraneTestItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_MEMBRANE_TEST_ITEM_TYPE);

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
    ProtocolDropIvGraphItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_IV_GRAPH_ITEM_TYPE);

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
    ProtocolDropVoltageTrackingItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_VOLTAGE_TRACKING_ITEM_TYPE);

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
    ProtocolDropApThresholdItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_AP_THRESHOLD_ITEM_TYPE);

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
    ProtocolDropApStatisticsItem(e4gcl::CommLib * commLib, ProtocolItemCtrlManager * ctrlManager, double hold0, int type = PROT_DROP_LIST_AP_STATISTICS_ITEM_TYPE);

public slots:
    void onSetString() override;
    void onAcceptPropertyDialog() override;
    void onRejectPropertyDialog() override;
    void onUpdateHold(double value) override;
    QString onCheckCursorsValidity() override;
};

#endif // PROTOCOLDROPITEM_H
