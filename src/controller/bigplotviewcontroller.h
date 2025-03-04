#ifndef BIGPLOTVIEWCONTROLLER_H
#define BIGPLOTVIEWCONTROLLER_H

#include <QObject>
#include "e384commlib_global_addendum.h"
#include "protocol/protocolwidget.h"
#include "bigplotmodel.h"
#include <memory>

class BigPlotViewController : public QObject
{
    Q_OBJECT

protected:
    std::unique_ptr<BigPlotModel> model;
    BigPlot* plot;

public:
    explicit BigPlotViewController(std::unique_ptr<BigPlotModel> model, BigPlot* plot, QObject *parent = nullptr);
    void setup();
    BigPlot* getPlot();
    BigPlotModel* getModel();

public slots:
    Rect4 handleZoomInRequest(Rect4 r);
    Rect4 handleZoomOutRequest();
    Rect4 handleZoomResetRequest();
    Rect4 handleSingleAxisZoomRequest(QwtPlot::Axis, int, QPointF);
    Rect4 handleSingleAxisZoomRequest(QwtPlot::Axis axis, QwtInterval i);
    Rect4 handleSingleAxisShiftRequest(QwtPlot::Axis, int);

signals:
};

#endif // BIGPLOTVIEWCONTROLLER_H
