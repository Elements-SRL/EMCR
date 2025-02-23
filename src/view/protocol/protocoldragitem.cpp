#include "protocoldragitem.h"

ProtocolDragItem::ProtocolDragItem(e384CommLib::ClampingModality_t clampingModality, int type) :
    QListWidgetItem(QString("Drag"), nullptr, type) {

    if (clampingModality == e384CommLib::ClampingModality_t::VOLTAGE_CLAMP) {
        stimulusAbbrName = "V";

    } else {
        stimulusAbbrName = "I";
    }
}

ProtocolDragStimulusItem::ProtocolDragStimulusItem(e384CommLib::ClampingModality_t clampingModality, int type) :
    ProtocolDragItem(clampingModality, type) {
    this->setBackground(PROT_EDITOR_STIMULUS_ITEM_COLOR);
}

ProtocolDragXStepTStepItem::ProtocolDragXStepTStepItem(e384CommLib::ClampingModality_t clampingModality, int type) :
    ProtocolDragStimulusItem(clampingModality, type) {

    QString iconString = ":imgs/stimulus step time step.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " step t step");
}

ProtocolDragVStepTStepItem::ProtocolDragVStepTStepItem(int type) :
    ProtocolDragXStepTStepItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIStepTStepItem::ProtocolDragIStepTStepItem(int type) :
    ProtocolDragXStepTStepItem(e384CommLib::ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXStepItem::ProtocolDragXStepItem(e384CommLib::ClampingModality_t clampingModality, int type) :
    ProtocolDragXStepTStepItem(clampingModality, type) {

    QString iconString = ":imgs/stimulus step.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " step");
}

ProtocolDragVStepItem::ProtocolDragVStepItem(int type) :
    ProtocolDragXStepItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIStepItem::ProtocolDragIStepItem(int type) :
    ProtocolDragXStepItem(e384CommLib::ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXTStepItem::ProtocolDragXTStepItem(e384CommLib::ClampingModality_t clampingModality, int type) :
    ProtocolDragXStepTStepItem(clampingModality, type) {

    QString iconString = ":imgs/time step.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("t step");
}

ProtocolDragVTStepItem::ProtocolDragVTStepItem(int type) :
    ProtocolDragXTStepItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragITStepItem::ProtocolDragITStepItem(int type) :
    ProtocolDragXTStepItem(e384CommLib::ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXConstItem::ProtocolDragXConstItem(e384CommLib::ClampingModality_t clampingModality, int type) :
    ProtocolDragXStepTStepItem(clampingModality, type) {

    QString iconString = ":imgs/constant stimulus.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " const");
}

ProtocolDragVConstItem::ProtocolDragVConstItem(int type) :
    ProtocolDragXConstItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIConstItem::ProtocolDragIConstItem(int type) :
    ProtocolDragXConstItem(e384CommLib::ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXHoldItem::ProtocolDragXHoldItem(e384CommLib::ClampingModality_t clampingModality, int type) :
    ProtocolDragXStepTStepItem(clampingModality, type) {

    QString iconString = ":imgs/holding stimulus.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " hold");
}

ProtocolDragVHoldItem::ProtocolDragVHoldItem(int type) :
    ProtocolDragXHoldItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIHoldItem::ProtocolDragIHoldItem(int type) :
    ProtocolDragXHoldItem(e384CommLib::ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXRestItem::ProtocolDragXRestItem(e384CommLib::ClampingModality_t clampingModality, int type) :
    ProtocolDragXStepTStepItem(clampingModality, type) {

    QString iconString = ":imgs/rest.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Rest");
}

ProtocolDragVRestItem::ProtocolDragVRestItem(int type) :
    ProtocolDragXRestItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIRestItem::ProtocolDragIRestItem(int type) :
    ProtocolDragXRestItem(e384CommLib::ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXRampItem::ProtocolDragXRampItem(e384CommLib::ClampingModality_t clampingModality, int type) :
    ProtocolDragStimulusItem(clampingModality, type) {

    QString iconString = ":imgs/stimulus ramp.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " ramp");
}

ProtocolDragVRampItem::ProtocolDragVRampItem(int type) :
    ProtocolDragXRampItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIRampItem::ProtocolDragIRampItem(int type) :
    ProtocolDragXRampItem(e384CommLib::ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXSinItem::ProtocolDragXSinItem(e384CommLib::ClampingModality_t clampingModality, int type) :
    ProtocolDragStimulusItem(clampingModality, type) {

    QString iconString = ":imgs/stimulus sin.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText(stimulusAbbrName + " sin");
}

ProtocolDragVSinItem::ProtocolDragVSinItem(int type) :
    ProtocolDragXSinItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragISinItem::ProtocolDragISinItem(int type) :
    ProtocolDragXSinItem(e384CommLib::ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragLoopsItem::ProtocolDragLoopsItem(e384CommLib::ClampingModality_t clampingModality, int type) :
    ProtocolDragItem(clampingModality, type) {
    this->setBackground(PROT_EDITOR_LOOPS_ITEM_COLOR);
}

ProtocolDragXRepSeqScaledItem::ProtocolDragXRepSeqScaledItem(e384CommLib::ClampingModality_t clampingModality, int type) :
    ProtocolDragLoopsItem(clampingModality, type) {

    QString iconString = ":imgs/P over N.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("P/N");
}

ProtocolDragVRepSeqScaledItem::ProtocolDragVRepSeqScaledItem(int type) :
    ProtocolDragXRepSeqScaledItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIRepSeqScaledItem::ProtocolDragIRepSeqScaledItem(int type) :
    ProtocolDragXRepSeqScaledItem(e384CommLib::ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXRepSeqItem::ProtocolDragXRepSeqItem(e384CommLib::ClampingModality_t clampingModality, int type) :
    ProtocolDragXRepSeqScaledItem(clampingModality, type) {

    QString iconString = ":imgs/repeat sequence.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Repeat");
}

ProtocolDragVRepSeqItem::ProtocolDragVRepSeqItem(int type) :
    ProtocolDragXRepSeqItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIRepSeqItem::ProtocolDragIRepSeqItem(int type) :
    ProtocolDragXRepSeqItem(e384CommLib::ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXRepSeqWithStepsItem::ProtocolDragXRepSeqWithStepsItem(e384CommLib::ClampingModality_t clampingModality, int type) :
    ProtocolDragXRepSeqScaledItem(clampingModality, type) {

    QString iconString = ":imgs/repeat with steps.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Repeat with steps");
}

ProtocolDragVRepSeqWithStepsItem::ProtocolDragVRepSeqWithStepsItem(int type) :
    ProtocolDragXRepSeqWithStepsItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIRepSeqWithStepsItem::ProtocolDragIRepSeqWithStepsItem(int type) :
    ProtocolDragXRepSeqWithStepsItem(e384CommLib::ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragXInfRepSeqItem::ProtocolDragXInfRepSeqItem(e384CommLib::ClampingModality_t clampingModality, int type) :
    ProtocolDragXRepSeqScaledItem(clampingModality, type) {

    QString iconString = ":imgs/infinite repeat sequence.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("Repeat (Inf)");
}

ProtocolDragVInfRepSeqItem::ProtocolDragVInfRepSeqItem(int type) :
    ProtocolDragXInfRepSeqItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

}

ProtocolDragIInfRepSeqItem::ProtocolDragIInfRepSeqItem(int type) :
    ProtocolDragXInfRepSeqItem(e384CommLib::ClampingModality_t::CURRENT_CLAMP, type) {

}

ProtocolDragControlItem::ProtocolDragControlItem(e384CommLib::ClampingModality_t clampingModality, int type) :
    ProtocolDragItem(clampingModality, type) {
    this->setBackground(PROT_EDITOR_CONTROLS_ITEM_COLOR);
}

ProtocolDragVoltageControlItem::ProtocolDragVoltageControlItem(int type) :
    ProtocolDragControlItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/voltage control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("V control");
}

ProtocolDragCurrentControlItem::ProtocolDragCurrentControlItem(int type) :
    ProtocolDragControlItem(e384CommLib::ClampingModality_t::CURRENT_CLAMP, type) {

    QString iconString = ":imgs/current control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("I control");
}

ProtocolDragTimeControlItem::ProtocolDragTimeControlItem(int type) :
    ProtocolDragControlItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/time control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("t control");
}

ProtocolDragFrequencyControlItem::ProtocolDragFrequencyControlItem(int type) :
    ProtocolDragControlItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/frequency control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("freq control");
}

ProtocolDragNaturalNumControlItem::ProtocolDragNaturalNumControlItem(int type) :
    ProtocolDragControlItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

    QString iconString = ":imgs/number control.png";
    QIcon icon;
    icon.addPixmap(iconString);
    this->setIcon(icon);

    this->setText("N control");
}

ProtocolDragSeparator::ProtocolDragSeparator(int type) :
    ProtocolDragItem(e384CommLib::ClampingModality_t::VOLTAGE_CLAMP, type) {

    this->setText("");
}
