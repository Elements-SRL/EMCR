#include "conversionscaledraw.h"

ConversionScaleDraw::ConversionScaleDraw(double conversionFactor) :
    conversionFactor(conversionFactor) {
}

void ConversionScaleDraw::setConversionFactor(double value) {
    conversionFactor = value;
    this->invalidateCache();
}

QwtText ConversionScaleDraw::label(double value) const {
    return QwtScaleDraw::label(value*conversionFactor);
}
