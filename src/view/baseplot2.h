#ifndef BASEPLOT2_H
#define BASEPLOT2_H

#include <QObject>
#include "qwt_plot.h"
#include "qwt_text_label.h"
#include <memory>
#include "plotmodel.h"
#include <map>

class BasePlot2 : public QwtPlot
{
    Q_OBJECT
private:
    QwtPlotPicker * zoomInPicker;
    void handleLabelsPosition();
    std::shared_ptr<PlotModel> pm;
    std::map<QwtPlot::Axis, QwtTextLabel*> labels;
    QwtTextLabel* createTextLabel(std::string, QwtPlot::Axis);

protected:
    virtual void resizeEvent(QResizeEvent * e) override;
    virtual void wheelEvent(QWheelEvent * e) override;

public slots:
    void onReplot();
    void onRubberBandUpdated();
    void onAutoZoom(std::vector <QwtPlot::Axis>);

public:
    explicit BasePlot2(std::shared_ptr<PlotModel> pm, QWidget *parent = nullptr);

    void updateRect();
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void drawCanvas(QPainter * p) override;

signals:
    void sigZoomOut();
    void sigZoomReset();
    void singleAxisZoomRequest(QwtPlot::Axis, int, QPointF);
    void singleAxisShiftRequest(QwtPlot::Axis, int);
    void sigZoomInPickerAppended(const QPointF &p, QWidget* canvas);
    void sigZoomInPickerMoved(const QPointF &p);
    void sigZoomInPickerSelected(const QRectF &r);
};

#endif // BASEPLOT2_H
