#ifndef RECT4_H
#define RECT4_H

#include "qwt_plot.h"

class Rect4 : public QVector <QwtInterval> {
public:
    Rect4(QwtPlot * plot);
    Rect4(QRectF rect);
    Rect4();

    Rect4 & operator = (const Rect4 &other);
};

#endif // RECT4_H
