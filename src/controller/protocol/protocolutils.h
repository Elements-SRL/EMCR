#ifndef PROTOCOLUTILS_H
#define PROTOCOLUTILS_H

#include "QDoubleSpinBox"
#include "e384commlib_global_addendum.h"

typedef enum {
    MIN_MAX,
    ZERO_MAX,
    DELTA
}RangedQDoubleSpinBox_t;

QDoubleSpinBox * initQdoubleSpinBox(QDoubleSpinBox * dsb, e384CommLib::RangedMeasurement_t rm, RangedQDoubleSpinBox_t rqdsb);

void copy(QDoubleSpinBox * from, QDoubleSpinBox * to);
#endif // PROTOCOLUTILS_H
