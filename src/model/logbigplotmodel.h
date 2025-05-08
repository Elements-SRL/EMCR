#ifndef LOGBIGPLOTMODEL_H
#define LOGBIGPLOTMODEL_H

#include <QObject>
#include "bigplotmodel.h"
#include <QRectF>

class LogBigPlotModel : public BigPlotModel
{

public:
    LogBigPlotModel();

    Rect4 zoomOnSingleAxis(QwtPlot::Axis, int, QPointF);
    Rect4 shiftOnSingleAxis(QwtPlot::Axis, int);

signals:
};

#endif // LOGBIGPLOTMODEL_H
