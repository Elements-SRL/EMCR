#include "protocoldragitem.h"

ProtocolDragItem::ProtocolDragItem(ClampingModality_t clampingModality, int type) :
    QListWidgetItem(QString("Drag"), nullptr, type) {

    if (clampingModality == ClampingModality_t::VOLTAGE_CLAMP) {
        stimulusAbbrName = "V";

    } else {
        stimulusAbbrName = "I";
    }
}

ProtocolDragStimulusItem::ProtocolDragStimulusItem(ClampingModality_t clampingModality, int type) :
    ProtocolDragItem(clampingModality, type) {
    this->setBackground(PROT_EDITOR_STIMULUS_ITEM_COLOR);
}

ProtocolDragXStepTStepItem::ProtocolDragXStepTStepItem(ClampingModality_t clampingModality, int type) :
    ProtocolDragStimulusItem(clampingModality, type) {

    QString iconString = ":imgs/stimulus step time step.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " step t step");
}

ProtocolDragVStepTStepItem::ProtocolDragVStepTStepItem(int type) :
    ProtocolDragXStepTStepItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIStepTStepItem::ProtocolDragIStepTStepItem(int type) :
    ProtocolDragXStepTStepItem(ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXStepItem::ProtocolDragXStepItem(ClampingModality_t clampingModality, int type) :
    ProtocolDragXStepTStepItem(clampingModality, type) {

    QString iconString = ":imgs/stimulus step.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " step");
}

ProtocolDragVStepItem::ProtocolDragVStepItem(int type) :
    ProtocolDragXStepItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIStepItem::ProtocolDragIStepItem(int type) :
    ProtocolDragXStepItem(ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXTStepItem::ProtocolDragXTStepItem(ClampingModality_t clampingModality, int type) :
    ProtocolDragXStepTStepItem(clampingModality, type) {

    QString iconString = ":imgs/time step.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("t step");
}

ProtocolDragVTStepItem::ProtocolDragVTStepItem(int type) :
    ProtocolDragXTStepItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragITStepItem::ProtocolDragITStepItem(int type) :
    ProtocolDragXTStepItem(ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXConstItem::ProtocolDragXConstItem(ClampingModality_t clampingModality, int type) :
    ProtocolDragXStepTStepItem(clampingModality, type) {

    QString iconString = ":imgs/constant stimulus.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " const");
}

ProtocolDragVConstItem::ProtocolDragVConstItem(int type) :
    ProtocolDragXConstItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIConstItem::ProtocolDragIConstItem(int type) :
    ProtocolDragXConstItem(ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXHoldItem::ProtocolDragXHoldItem(ClampingModality_t clampingModality, int type) :
    ProtocolDragXStepTStepItem(clampingModality, type) {

    QString iconString = ":imgs/holding stimulus.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " hold");
}

ProtocolDragVHoldItem::ProtocolDragVHoldItem(int type) :
    ProtocolDragXHoldItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIHoldItem::ProtocolDragIHoldItem(int type) :
    ProtocolDragXHoldItem(ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXRestItem::ProtocolDragXRestItem(ClampingModality_t clampingModality, int type) :
    ProtocolDragXStepTStepItem(clampingModality, type) {

    QString iconString = ":imgs/rest.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Rest");
}

ProtocolDragVRestItem::ProtocolDragVRestItem(int type) :
    ProtocolDragXRestItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIRestItem::ProtocolDragIRestItem(int type) :
    ProtocolDragXRestItem(ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXRampItem::ProtocolDragXRampItem(ClampingModality_t clampingModality, int type) :
    ProtocolDragStimulusItem(clampingModality, type) {

    QString iconString = ":imgs/stimulus ramp.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " ramp");
}

ProtocolDragVRampItem::ProtocolDragVRampItem(int type) :
    ProtocolDragXRampItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIRampItem::ProtocolDragIRampItem(int type) :
    ProtocolDragXRampItem(ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXSinItem::ProtocolDragXSinItem(ClampingModality_t clampingModality, int type) :
    ProtocolDragStimulusItem(clampingModality, type) {

    QString iconString = ":imgs/stimulus sin.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " sin");
}

ProtocolDragVSinItem::ProtocolDragVSinItem(int type) :
    ProtocolDragXSinItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragISinItem::ProtocolDragISinItem(int type) :
    ProtocolDragXSinItem(ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragLoopsItem::ProtocolDragLoopsItem(ClampingModality_t clampingModality, int type) :
    ProtocolDragItem(clampingModality, type) {
    this->setBackground(PROT_EDITOR_LOOPS_ITEM_COLOR);
}

ProtocolDragXRepSeqScaledItem::ProtocolDragXRepSeqScaledItem(ClampingModality_t clampingModality, int type) :
    ProtocolDragLoopsItem(clampingModality, type) {

    QString iconString = ":imgs/P over N.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("P/N");
}

ProtocolDragVRepSeqScaledItem::ProtocolDragVRepSeqScaledItem(int type) :
    ProtocolDragXRepSeqScaledItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIRepSeqScaledItem::ProtocolDragIRepSeqScaledItem(int type) :
    ProtocolDragXRepSeqScaledItem(ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXRepSeqItem::ProtocolDragXRepSeqItem(ClampingModality_t clampingModality, int type) :
    ProtocolDragXRepSeqScaledItem(clampingModality, type) {

    QString iconString = ":imgs/repeat sequence.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Repeat");
}

ProtocolDragVRepSeqItem::ProtocolDragVRepSeqItem(int type) :
    ProtocolDragXRepSeqItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIRepSeqItem::ProtocolDragIRepSeqItem(int type) :
    ProtocolDragXRepSeqItem(ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXRepSeqWithStepsItem::ProtocolDragXRepSeqWithStepsItem(ClampingModality_t clampingModality, int type) :
    ProtocolDragXRepSeqScaledItem(clampingModality, type) {

    QString iconString = ":imgs/repeat with steps.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Repeat with steps");
}

ProtocolDragVRepSeqWithStepsItem::ProtocolDragVRepSeqWithStepsItem(int type) :
    ProtocolDragXRepSeqWithStepsItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIRepSeqWithStepsItem::ProtocolDragIRepSeqWithStepsItem(int type) :
    ProtocolDragXRepSeqWithStepsItem(ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXInfRepSeqItem::ProtocolDragXInfRepSeqItem(ClampingModality_t clampingModality, int type) :
    ProtocolDragXRepSeqScaledItem(clampingModality, type) {

    QString iconString = ":imgs/infinite repeat sequence.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Repeat (Inf)");
}

ProtocolDragVInfRepSeqItem::ProtocolDragVInfRepSeqItem(int type) :
    ProtocolDragXInfRepSeqItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIInfRepSeqItem::ProtocolDragIInfRepSeqItem(int type) :
    ProtocolDragXInfRepSeqItem(ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragControlItem::ProtocolDragControlItem(ClampingModality_t clampingModality, int type) :
    ProtocolDragItem(clampingModality, type) {
    this->setBackground(PROT_EDITOR_CONTROLS_ITEM_COLOR);
}

ProtocolDragVoltageControlItem::ProtocolDragVoltageControlItem(int type) :
    ProtocolDragControlItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/voltage control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("V control");
}

ProtocolDragCurrentControlItem::ProtocolDragCurrentControlItem(int type) :
    ProtocolDragControlItem(ClampingModality_t::CURRENT_CLAMP, type) {

    QString iconString = ":imgs/current control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("I control");
}

ProtocolDragTimeControlItem::ProtocolDragTimeControlItem(int type) :
    ProtocolDragControlItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/time control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("t control");
}

ProtocolDragFrequencyControlItem::ProtocolDragFrequencyControlItem(int type) :
    ProtocolDragControlItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/frequency control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("freq control");
}

ProtocolDragNaturalNumControlItem::ProtocolDragNaturalNumControlItem(int type) :
    ProtocolDragControlItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/number control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("N control");
}

ProtocolDragAnalysisItem::ProtocolDragAnalysisItem(ClampingModality_t clampingModality, int type) :
    ProtocolDragItem(clampingModality, type) {
    this->setBackground(PROT_EDITOR_ANALYSIS_ITEM_COLOR);
}

ProtocolDragNoiseReportItem::ProtocolDragNoiseReportItem(int type) :
    ProtocolDragAnalysisItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/analysis noise report.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Noise report");
}

ProtocolDragHistogramItem::ProtocolDragHistogramItem(int type) :
    ProtocolDragAnalysisItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/analysis histogram.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Histogram");
}

ProtocolDragSpectrumItem::ProtocolDragSpectrumItem(int type) :
    ProtocolDragAnalysisItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/analysis spectrum.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Spectrum");
}

ProtocolDragResistanceEstimationItem::ProtocolDragResistanceEstimationItem(int type) :
    ProtocolDragAnalysisItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/analysis resistance estimation.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Resistance estimation");
}

ProtocolDragMembraneTestItem::ProtocolDragMembraneTestItem(int type) :
    ProtocolDragAnalysisItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/analysis membrane test.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Membrane test");
}

ProtocolDragIvGraphItem::ProtocolDragIvGraphItem(int type) :
    ProtocolDragAnalysisItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/analysis iv graph.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("I/V graph");
}

ProtocolDragVoltageTrackingItem::ProtocolDragVoltageTrackingItem(int type) :
    ProtocolDragAnalysisItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/analysis voltage tracking.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Voltage tracking");
}

ProtocolDragApThresholdItem::ProtocolDragApThresholdItem(int type) :
    ProtocolDragAnalysisItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/analysis ap threshold.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("AP threshold");
}

ProtocolDragApStatisticsItem::ProtocolDragApStatisticsItem(int type) :
    ProtocolDragAnalysisItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/analysis ap statistics.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("AP statistics");
}

ProtocolDragSeparator::ProtocolDragSeparator(int type) :
    ProtocolDragItem(ClampingModality_t::VOLTAGE_CLAMP, type) {

    this->setText("");
}
