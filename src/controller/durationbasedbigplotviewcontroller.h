#ifndef DURATIONBASEDBIGPLOTVIEWCONTROLLER_H
#define DURATIONBASEDBIGPLOTVIEWCONTROLLER_H

#include "bigplotviewcontroller.h"

class DurationBasedBigPlotViewController : public BigPlotViewController
{
    Q_OBJECT

public:
    explicit DurationBasedBigPlotViewController(std::unique_ptr<BigPlotModel> model, std::unique_ptr<BigPlot> plot, QObject *parent = nullptr);
    void setup();

public slots:
    Rect4 handleZoomInRequest(Rect4 r);
    Rect4 handleZoomOutRequest();
    Rect4 handleZoomResetRequest();
    Rect4 handleSingleAxisZoomRequest(QwtPlot::Axis, int, QPointF);
    Rect4 handleSingleAxisZoomRequest(QwtPlot::Axis axis, QwtInterval i);

signals:
    void durationChanged(Measurement_t duration);
};

#endif // DURATIONBASEDBIGPLOTVIEWCONTROLLER_H
