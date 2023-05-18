#include "protocol/protocolutils.h"

QDoubleSpinBox * initQdoubleSpinBox(QDoubleSpinBox * dsb, e384CommLib::RangedMeasurement_t rm, RangedQDoubleSpinBox_t rqdsb){
    switch (rqdsb) {
    case RangedQDoubleSpinBox_t::MIN_MAX:{
        dsb->setRange(rm.min, rm.max);
        break;
    }
    case RangedQDoubleSpinBox_t::DELTA:{
        dsb->setRange(-rm.delta(), rm.delta());
        break;
    }
    case RangedQDoubleSpinBox_t::ZERO_MAX:{
        dsb->setRange(0, rm.max);
        break;
    }
    }
    dsb->setDecimals(rm.decimals());
    dsb->setSingleStep(rm.step);
    return dsb;
}

void copy(QDoubleSpinBox * from, QDoubleSpinBox * to){
    double minimum = from->minimum();
    double maximum = from->maximum();
    int decimals = from->decimals();
    double singleStep = from->singleStep();
    double value = from->value();

    to->setRange(minimum, maximum);
    to->setDecimals(decimals);
    to->setSingleStep(singleStep);
    to->setValue(value);
}

