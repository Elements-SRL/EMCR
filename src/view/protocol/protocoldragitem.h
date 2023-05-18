#ifndef PROTOCOLDRAGITEM_H
#define PROTOCOLDRAGITEM_H

#include <QListWidgetItem>

#include "globaldefines.h"
#include "protocoldefs.h"

class ProtocolDragItem : public QListWidgetItem {
public:
    ProtocolDragItem(ClampingModality_t clampingModality, int type = PROT_DRAG_LIST_WIDGET_ITEM_TYPE);

protected:
    QString stimulusAbbrName;
};

class ProtocolDragStimulusItem : public ProtocolDragItem {
public:
    ProtocolDragStimulusItem(ClampingModality_t clampingModality, int type = PROT_DRAG_LIST_STIMULUS_ITEM_TYPE);
};

class ProtocolDragXStepTStepItem : public ProtocolDragStimulusItem {
public:
    ProtocolDragXStepTStepItem(ClampingModality_t clampingModality, int type);
};

class ProtocolDragVStepTStepItem : public ProtocolDragXStepTStepItem {
public:
    ProtocolDragVStepTStepItem(int type = PROT_DRAG_LIST_VSTEP_TSTEP_ITEM_TYPE);
};

class ProtocolDragIStepTStepItem : public ProtocolDragXStepTStepItem {
public:
    ProtocolDragIStepTStepItem(int type = PROT_DRAG_LIST_ISTEP_TSTEP_ITEM_TYPE);
};

class ProtocolDragXStepItem : public ProtocolDragXStepTStepItem {
public:
    ProtocolDragXStepItem(ClampingModality_t clampingModality, int type);
};

class ProtocolDragVStepItem : public ProtocolDragXStepItem {
public:
    ProtocolDragVStepItem(int type = PROT_DRAG_LIST_VSTEP_ITEM_TYPE);
};

class ProtocolDragIStepItem : public ProtocolDragXStepItem {
public:
    ProtocolDragIStepItem(int type = PROT_DRAG_LIST_ISTEP_ITEM_TYPE);
};

class ProtocolDragXTStepItem : public ProtocolDragXStepTStepItem {
public:
    ProtocolDragXTStepItem(ClampingModality_t clampingModality, int type);
};

class ProtocolDragVTStepItem : public ProtocolDragXTStepItem {
public:
    ProtocolDragVTStepItem(int type = PROT_DRAG_LIST_VTSTEP_ITEM_TYPE);
};

class ProtocolDragITStepItem : public ProtocolDragXTStepItem {
public:
    ProtocolDragITStepItem(int type = PROT_DRAG_LIST_ITSTEP_ITEM_TYPE);
};

class ProtocolDragXConstItem : public ProtocolDragXStepTStepItem {
public:
    ProtocolDragXConstItem(ClampingModality_t clampingModality, int type);
};

class ProtocolDragVConstItem : public ProtocolDragXConstItem {
public:
    ProtocolDragVConstItem(int type = PROT_DRAG_LIST_VCONST_ITEM_TYPE);
};

class ProtocolDragIConstItem : public ProtocolDragXConstItem {
public:
    ProtocolDragIConstItem(int type = PROT_DRAG_LIST_ICONST_ITEM_TYPE);
};

class ProtocolDragXHoldItem : public ProtocolDragXStepTStepItem {
public:
    ProtocolDragXHoldItem(ClampingModality_t clampingModality, int type);
};

class ProtocolDragVHoldItem : public ProtocolDragXHoldItem {
public:
    ProtocolDragVHoldItem(int type = PROT_DRAG_LIST_VHOLD_ITEM_TYPE);
};

class ProtocolDragIHoldItem : public ProtocolDragXHoldItem {
public:
    ProtocolDragIHoldItem(int type = PROT_DRAG_LIST_IHOLD_ITEM_TYPE);
};

class ProtocolDragXRestItem : public ProtocolDragXStepTStepItem {
public:
    ProtocolDragXRestItem(ClampingModality_t clampingModality, int type);
};

class ProtocolDragVRestItem : public ProtocolDragXRestItem {
public:
    ProtocolDragVRestItem(int type = PROT_DRAG_LIST_VREST_ITEM_TYPE);
};

class ProtocolDragIRestItem : public ProtocolDragXRestItem {
public:
    ProtocolDragIRestItem(int type = PROT_DRAG_LIST_IREST_ITEM_TYPE);
};

class ProtocolDragXRampItem : public ProtocolDragStimulusItem {
public:
    ProtocolDragXRampItem(ClampingModality_t clampingModality, int type);
};

class ProtocolDragVRampItem : public ProtocolDragXRampItem {
public:
    ProtocolDragVRampItem(int type = PROT_DRAG_LIST_VRAMP_ITEM_TYPE);
};

class ProtocolDragIRampItem : public ProtocolDragXRampItem {
public:
    ProtocolDragIRampItem(int type = PROT_DRAG_LIST_IRAMP_ITEM_TYPE);
};

class ProtocolDragXSinItem : public ProtocolDragStimulusItem {
public:
    ProtocolDragXSinItem(ClampingModality_t clampingModality, int type = PROT_DRAG_LIST_VSIN_ITEM_TYPE);
};

class ProtocolDragVSinItem : public ProtocolDragXSinItem {
public:
    ProtocolDragVSinItem(int type = PROT_DRAG_LIST_VSIN_ITEM_TYPE);
};

class ProtocolDragISinItem : public ProtocolDragXSinItem {
public:
    ProtocolDragISinItem(int type = PROT_DRAG_LIST_ISIN_ITEM_TYPE);
};

class ProtocolDragLoopsItem : public ProtocolDragItem {
public:
    ProtocolDragLoopsItem(ClampingModality_t clampingModality, int type = PROT_DRAG_LIST_LOOPS_ITEM_TYPE);
};

class ProtocolDragXRepSeqScaledItem : public ProtocolDragLoopsItem {
public:
    ProtocolDragXRepSeqScaledItem(ClampingModality_t clampingModality, int type);
};

class ProtocolDragVRepSeqScaledItem : public ProtocolDragXRepSeqScaledItem {
public:
    ProtocolDragVRepSeqScaledItem(int type = PROT_DRAG_LIST_VREP_SEQ_SCALED_ITEM_TYPE);
};

class ProtocolDragIRepSeqScaledItem : public ProtocolDragXRepSeqScaledItem {
public:
    ProtocolDragIRepSeqScaledItem(int type = PROT_DRAG_LIST_IREP_SEQ_SCALED_ITEM_TYPE);
};

class ProtocolDragXRepSeqItem : public ProtocolDragXRepSeqScaledItem {
public:
    ProtocolDragXRepSeqItem(ClampingModality_t clampingModality, int type);
};

class ProtocolDragVRepSeqItem : public ProtocolDragXRepSeqItem {
public:
    ProtocolDragVRepSeqItem(int type = PROT_DRAG_LIST_VREP_SEQ_ITEM_TYPE);
};

class ProtocolDragIRepSeqItem : public ProtocolDragXRepSeqItem {
public:
    ProtocolDragIRepSeqItem(int type = PROT_DRAG_LIST_IREP_SEQ_ITEM_TYPE);
};

class ProtocolDragXRepSeqWithStepsItem : public ProtocolDragXRepSeqScaledItem {
public:
    ProtocolDragXRepSeqWithStepsItem(ClampingModality_t clampingModality, int type);
};

class ProtocolDragVRepSeqWithStepsItem : public ProtocolDragXRepSeqWithStepsItem {
public:
    ProtocolDragVRepSeqWithStepsItem(int type = PROT_DRAG_LIST_VREP_SEQ_WITH_STEPS_ITEM_TYPE);
};

class ProtocolDragIRepSeqWithStepsItem : public ProtocolDragXRepSeqWithStepsItem {
public:
    ProtocolDragIRepSeqWithStepsItem(int type = PROT_DRAG_LIST_IREP_SEQ_WITH_STEPS_ITEM_TYPE);
};

class ProtocolDragXInfRepSeqItem : public ProtocolDragXRepSeqScaledItem {
public:
    ProtocolDragXInfRepSeqItem(ClampingModality_t clampingModality, int type);
};

class ProtocolDragVInfRepSeqItem : public ProtocolDragXInfRepSeqItem {
public:
    ProtocolDragVInfRepSeqItem(int type = PROT_DRAG_LIST_VINF_REP_SEQ_ITEM_TYPE);
};

class ProtocolDragIInfRepSeqItem : public ProtocolDragXInfRepSeqItem {
public:
    ProtocolDragIInfRepSeqItem(int type = PROT_DRAG_LIST_IINF_REP_SEQ_ITEM_TYPE);
};

class ProtocolDragControlItem : public ProtocolDragItem {
public:
    ProtocolDragControlItem(ClampingModality_t clampingModality, int type = PROT_DRAG_LIST_CONTROL_ITEM_TYPE);
};

class ProtocolDragVoltageControlItem : public ProtocolDragControlItem {
public:
    ProtocolDragVoltageControlItem(int type = PROT_DRAG_LIST_VOLTAGE_CONTROL_ITEM_TYPE);
};

class ProtocolDragCurrentControlItem : public ProtocolDragControlItem {
public:
    ProtocolDragCurrentControlItem(int type = PROT_DRAG_LIST_CURRENT_CONTROL_ITEM_TYPE);
};

class ProtocolDragTimeControlItem : public ProtocolDragControlItem {
public:
    ProtocolDragTimeControlItem(int type = PROT_DRAG_LIST_TIME_CONTROL_ITEM_TYPE);
};

class ProtocolDragFrequencyControlItem : public ProtocolDragControlItem {
public:
    ProtocolDragFrequencyControlItem(int type = PROT_DRAG_LIST_FREQUENCY_CONTROL_ITEM_TYPE);
};

class ProtocolDragNaturalNumControlItem : public ProtocolDragControlItem {
public:
    ProtocolDragNaturalNumControlItem(int type = PROT_DRAG_LIST_NATURAL_NUM_CONTROL_ITEM_TYPE);
};

class ProtocolDragAnalysisItem : public ProtocolDragItem {
public:
    ProtocolDragAnalysisItem(ClampingModality_t clampingModality, int type = PROT_DRAG_LIST_ANALYSIS_ITEM_TYPE);
};

/*! \todo FCON All analyses so far defined as voltage clamp analysis */
class ProtocolDragNoiseReportItem : public ProtocolDragAnalysisItem {
public:
    ProtocolDragNoiseReportItem(int type = PROT_DRAG_LIST_NOISE_REPORT_ITEM_TYPE);
};

class ProtocolDragHistogramItem : public ProtocolDragAnalysisItem {
public:
    ProtocolDragHistogramItem(int type = PROT_DRAG_LIST_HISTOGRAM_ITEM_TYPE);
};

class ProtocolDragSpectrumItem : public ProtocolDragAnalysisItem {
public:
    ProtocolDragSpectrumItem(int type = PROT_DRAG_LIST_SPECTRUM_ITEM_TYPE);
};

class ProtocolDragResistanceEstimationItem : public ProtocolDragAnalysisItem {
public:
    ProtocolDragResistanceEstimationItem(int type = PROT_DRAG_LIST_RESISTANCE_ESTIMATION_ITEM_TYPE);
};

class ProtocolDragMembraneTestItem : public ProtocolDragAnalysisItem {
public:
    ProtocolDragMembraneTestItem(int type = PROT_DRAG_LIST_MEMBRANE_TEST_ITEM_TYPE);
};

class ProtocolDragIvGraphItem : public ProtocolDragAnalysisItem {
public:
    ProtocolDragIvGraphItem(int type = PROT_DRAG_LIST_IV_GRAPH_ITEM_TYPE);
};

class ProtocolDragVoltageTrackingItem : public ProtocolDragAnalysisItem {
public:
    ProtocolDragVoltageTrackingItem(int type = PROT_DRAG_LIST_VOLTAGE_TRACKING_ITEM_TYPE);
};

class ProtocolDragApThresholdItem : public ProtocolDragAnalysisItem {
public:
    ProtocolDragApThresholdItem(int type = PROT_DRAG_LIST_AP_THRESHOLD_ITEM_TYPE);
};

class ProtocolDragApStatisticsItem : public ProtocolDragAnalysisItem {
public:
    ProtocolDragApStatisticsItem(int type = PROT_DRAG_LIST_AP_STATISTICS_ITEM_TYPE);
};

class ProtocolDragSeparator : public ProtocolDragItem {
public:
    ProtocolDragSeparator(int type = PROT_DRAG_LIST_SEPARATOR_TYPE);
};

#endif // PROTOCOLDRAGITEM_H
