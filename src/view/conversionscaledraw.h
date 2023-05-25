#ifndef CONVERSIONSCALEDRAW_H
#define CONVERSIONSCALEDRAW_H

#include "qwt_text.h"
#include "qwt_scale_draw.h"

class ConversionScaleDraw : public QwtScaleDraw{
public:
    ConversionScaleDraw(double conversionFactor = 1.0);

    void setConversionFactor(double value);
    QwtText label(double value) const override;

private:
    double conversionFactor;
};

#endif // CONVERSIONSCALEDRAW_H
