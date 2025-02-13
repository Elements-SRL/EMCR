#include "rect4.h"

#include "qwt_interval.h"

Rect4::Rect4(QwtPlot * plot) {
    for (int axisIdx = 0; axisIdx < QwtPlot::axisCnt; axisIdx++) {
        this->push_back(plot->axisInterval(axisIdx));
    }
}

Rect4::Rect4(QRectF rect) {
    for (int axisIdx = 0; axisIdx < QwtPlot::axisCnt; axisIdx++) {
        if (axisIdx == QwtPlot::yLeft) {
            this->push_back(QwtInterval(rect.y(), rect.y()+rect.height()));

        } else if (axisIdx == QwtPlot::xBottom) {
            this->push_back(QwtInterval(rect.x(), rect.x()+rect.width()));

        } else {
            this->push_back(QwtInterval(0.0, 1000.0));
        }
    }
}

Rect4::Rect4() {
    for (int axisIdx = 0; axisIdx < QwtPlot::axisCnt; axisIdx++) {
        this->push_back(QwtInterval(0.0, 1000.0));
    }
}

Rect4 & Rect4::operator = (const Rect4 &other) {
    if (&other == this) {
        return * this;
    }

    this->clear();
    for (int axisIdx = 0; axisIdx < QwtPlot::axisCnt; axisIdx++) {
        this->push_back(other.at(axisIdx));
    }

    return * this;
}
