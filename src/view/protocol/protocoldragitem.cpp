#include "protocoldragitem.h"

ProtocolDragItem::ProtocolDragItem(int clampingModality, int type) :
    QListWidgetItem(QString("Drag"), nullptr, type) {

    if (clampingModality == E4GCL_VOLTAGE_CLAMP_MODE) {
        stimulusAbbrName = "V";

    } else {
        stimulusAbbrName = "I";
    }
}

ProtocolDragStimulusItem::ProtocolDragStimulusItem(int clampingModality, int type) :
    ProtocolDragItem(clampingModality, type) {
    this->setBackground(PROT_EDITOR_STIMULUS_ITEM_COLOR);
}

ProtocolDragXStepTStepItem::ProtocolDragXStepTStepItem(int clampingModality, int type) :
    ProtocolDragStimulusItem(clampingModality, type) {

    QString iconString = ":imgs/stimulus step time step.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " step t step");
}

ProtocolDragVStepTStepItem::ProtocolDragVStepTStepItem(int type) :
    ProtocolDragXStepTStepItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

}

ProtocolDragIStepTStepItem::ProtocolDragIStepTStepItem(int type) :
    ProtocolDragXStepTStepItem(E4GCL_CURRENT_CLAMP_MODE, type) {

}

ProtocolDragXStepItem::ProtocolDragXStepItem(int clampingModality, int type) :
    ProtocolDragXStepTStepItem(clampingModality, type) {

    QString iconString = ":imgs/stimulus step.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " step");
}

ProtocolDragVStepItem::ProtocolDragVStepItem(int type) :
    ProtocolDragXStepItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

}

ProtocolDragIStepItem::ProtocolDragIStepItem(int type) :
    ProtocolDragXStepItem(E4GCL_CURRENT_CLAMP_MODE, type) {

}

ProtocolDragXTStepItem::ProtocolDragXTStepItem(int clampingModality, int type) :
    ProtocolDragXStepTStepItem(clampingModality, type) {

    QString iconString = ":imgs/time step.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("t step");
}

ProtocolDragVTStepItem::ProtocolDragVTStepItem(int type) :
    ProtocolDragXTStepItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

}

ProtocolDragITStepItem::ProtocolDragITStepItem(int type) :
    ProtocolDragXTStepItem(E4GCL_CURRENT_CLAMP_MODE, type) {

}

ProtocolDragXConstItem::ProtocolDragXConstItem(int clampingModality, int type) :
    ProtocolDragXStepTStepItem(clampingModality, type) {

    QString iconString = ":imgs/constant stimulus.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " const");
}

ProtocolDragVConstItem::ProtocolDragVConstItem(int type) :
    ProtocolDragXConstItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

}

ProtocolDragIConstItem::ProtocolDragIConstItem(int type) :
    ProtocolDragXConstItem(E4GCL_CURRENT_CLAMP_MODE, type) {

}

ProtocolDragXHoldItem::ProtocolDragXHoldItem(int clampingModality, int type) :
    ProtocolDragXStepTStepItem(clampingModality, type) {

    QString iconString = ":imgs/holding stimulus.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " hold");
}

ProtocolDragVHoldItem::ProtocolDragVHoldItem(int type) :
    ProtocolDragXHoldItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

}

ProtocolDragIHoldItem::ProtocolDragIHoldItem(int type) :
    ProtocolDragXHoldItem(E4GCL_CURRENT_CLAMP_MODE, type) {

}

ProtocolDragXRestItem::ProtocolDragXRestItem(int clampingModality, int type) :
    ProtocolDragXStepTStepItem(clampingModality, type) {

    QString iconString = ":imgs/rest.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Rest");
}

ProtocolDragVRestItem::ProtocolDragVRestItem(int type) :
    ProtocolDragXRestItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

}

ProtocolDragIRestItem::ProtocolDragIRestItem(int type) :
    ProtocolDragXRestItem(E4GCL_CURRENT_CLAMP_MODE, type) {

}

ProtocolDragXRampItem::ProtocolDragXRampItem(int clampingModality, int type) :
    ProtocolDragStimulusItem(clampingModality, type) {

    QString iconString = ":imgs/stimulus ramp.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " ramp");
}

ProtocolDragVRampItem::ProtocolDragVRampItem(int type) :
    ProtocolDragXRampItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

}

ProtocolDragIRampItem::ProtocolDragIRampItem(int type) :
    ProtocolDragXRampItem(E4GCL_CURRENT_CLAMP_MODE, type) {

}

ProtocolDragXSinItem::ProtocolDragXSinItem(int clampingModality, int type) :
    ProtocolDragStimulusItem(clampingModality, type) {

    QString iconString = ":imgs/stimulus sin.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " sin");
}

ProtocolDragVSinItem::ProtocolDragVSinItem(int type) :
    ProtocolDragXSinItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

}

ProtocolDragISinItem::ProtocolDragISinItem(int type) :
    ProtocolDragXSinItem(E4GCL_CURRENT_CLAMP_MODE, type) {

}

ProtocolDragLoopsItem::ProtocolDragLoopsItem(int clampingModality, int type) :
    ProtocolDragItem(clampingModality, type) {
    this->setBackground(PROT_EDITOR_LOOPS_ITEM_COLOR);
}

ProtocolDragXRepSeqScaledItem::ProtocolDragXRepSeqScaledItem(int clampingModality, int type) :
    ProtocolDragLoopsItem(clampingModality, type) {

    QString iconString = ":imgs/P over N.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("P/N");
}

ProtocolDragVRepSeqScaledItem::ProtocolDragVRepSeqScaledItem(int type) :
    ProtocolDragXRepSeqScaledItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

}

ProtocolDragIRepSeqScaledItem::ProtocolDragIRepSeqScaledItem(int type) :
    ProtocolDragXRepSeqScaledItem(E4GCL_CURRENT_CLAMP_MODE, type) {

}

ProtocolDragXRepSeqItem::ProtocolDragXRepSeqItem(int clampingModality, int type) :
    ProtocolDragXRepSeqScaledItem(clampingModality, type) {

    QString iconString = ":imgs/repeat sequence.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Repeat");
}

ProtocolDragVRepSeqItem::ProtocolDragVRepSeqItem(int type) :
    ProtocolDragXRepSeqItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

}

ProtocolDragIRepSeqItem::ProtocolDragIRepSeqItem(int type) :
    ProtocolDragXRepSeqItem(E4GCL_CURRENT_CLAMP_MODE, type) {

}

ProtocolDragXRepSeqWithStepsItem::ProtocolDragXRepSeqWithStepsItem(int clampingModality, int type) :
    ProtocolDragXRepSeqScaledItem(clampingModality, type) {

    QString iconString = ":imgs/repeat with steps.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Repeat with steps");
}

ProtocolDragVRepSeqWithStepsItem::ProtocolDragVRepSeqWithStepsItem(int type) :
    ProtocolDragXRepSeqWithStepsItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

}

ProtocolDragIRepSeqWithStepsItem::ProtocolDragIRepSeqWithStepsItem(int type) :
    ProtocolDragXRepSeqWithStepsItem(E4GCL_CURRENT_CLAMP_MODE, type) {

}

ProtocolDragXInfRepSeqItem::ProtocolDragXInfRepSeqItem(int clampingModality, int type) :
    ProtocolDragXRepSeqScaledItem(clampingModality, type) {

    QString iconString = ":imgs/infinite repeat sequence.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Repeat (Inf)");
}

ProtocolDragVInfRepSeqItem::ProtocolDragVInfRepSeqItem(int type) :
    ProtocolDragXInfRepSeqItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

}

ProtocolDragIInfRepSeqItem::ProtocolDragIInfRepSeqItem(int type) :
    ProtocolDragXInfRepSeqItem(E4GCL_CURRENT_CLAMP_MODE, type) {

}

ProtocolDragControlItem::ProtocolDragControlItem(int clampingModality, int type) :
    ProtocolDragItem(clampingModality, type) {
    this->setBackground(PROT_EDITOR_CONTROLS_ITEM_COLOR);
}

ProtocolDragVoltageControlItem::ProtocolDragVoltageControlItem(int type) :
    ProtocolDragControlItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

    QString iconString = ":imgs/voltage control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("V control");
}

ProtocolDragCurrentControlItem::ProtocolDragCurrentControlItem(int type) :
    ProtocolDragControlItem(E4GCL_CURRENT_CLAMP_MODE, type) {

    QString iconString = ":imgs/current control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("I control");
}

ProtocolDragTimeControlItem::ProtocolDragTimeControlItem(int type) :
    ProtocolDragControlItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

    QString iconString = ":imgs/time control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("t control");
}

ProtocolDragFrequencyControlItem::ProtocolDragFrequencyControlItem(int type) :
    ProtocolDragControlItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

    QString iconString = ":imgs/frequency control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("freq control");
}

ProtocolDragNaturalNumControlItem::ProtocolDragNaturalNumControlItem(int type) :
    ProtocolDragControlItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

    QString iconString = ":imgs/number control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("N control");
}

ProtocolDragAnalysisItem::ProtocolDragAnalysisItem(int clampingModality, int type) :
    ProtocolDragItem(clampingModality, type) {
    this->setBackground(PROT_EDITOR_ANALYSIS_ITEM_COLOR);
}

ProtocolDragNoiseReportItem::ProtocolDragNoiseReportItem(int type) :
    ProtocolDragAnalysisItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

    QString iconString = ":imgs/analysis noise report.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Noise report");
}

ProtocolDragHistogramItem::ProtocolDragHistogramItem(int type) :
    ProtocolDragAnalysisItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

    QString iconString = ":imgs/analysis histogram.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Histogram");
}

ProtocolDragSpectrumItem::ProtocolDragSpectrumItem(int type) :
    ProtocolDragAnalysisItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

    QString iconString = ":imgs/analysis spectrum.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Spectrum");
}

ProtocolDragResistanceEstimationItem::ProtocolDragResistanceEstimationItem(int type) :
    ProtocolDragAnalysisItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

    QString iconString = ":imgs/analysis resistance estimation.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Resistance estimation");
}

ProtocolDragMembraneTestItem::ProtocolDragMembraneTestItem(int type) :
    ProtocolDragAnalysisItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

    QString iconString = ":imgs/analysis membrane test.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Membrane test");
}

ProtocolDragIvGraphItem::ProtocolDragIvGraphItem(int type) :
    ProtocolDragAnalysisItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

    QString iconString = ":imgs/analysis iv graph.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("I/V graph");
}

ProtocolDragVoltageTrackingItem::ProtocolDragVoltageTrackingItem(int type) :
    ProtocolDragAnalysisItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

    QString iconString = ":imgs/analysis voltage tracking.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Voltage tracking");
}

ProtocolDragApThresholdItem::ProtocolDragApThresholdItem(int type) :
    ProtocolDragAnalysisItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

    QString iconString = ":imgs/analysis ap threshold.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("AP threshold");
}

ProtocolDragApStatisticsItem::ProtocolDragApStatisticsItem(int type) :
    ProtocolDragAnalysisItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

    QString iconString = ":imgs/analysis ap statistics.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("AP statistics");
}

ProtocolDragSeparator::ProtocolDragSeparator(int type) :
    ProtocolDragItem(E4GCL_VOLTAGE_CLAMP_MODE, type) {

    this->setText("");
}
