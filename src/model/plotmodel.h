#ifndef PLOTMODEL_H
#define PLOTMODEL_H

#include <QObject>
#include <QwtPlot>
#include "e384commlib_global_addendum.h"
#include <map>
#include "zoom.h"
#include "qwt_plot_picker.h"
#include <QWheelEvent>
#include "axisinfo.h"

class PlotModel: public QObject {
    Q_OBJECT

private:
    std::map<QwtPlot::Axis, AxisInfo> axisInfo;
    std::unique_ptr<Zoom> zoom;
    QPointF processPoint (const QPointF &p);
    QwtPlotPicker * picker;
    double pickerZoomDiscriminantNorm = 1.0;
    std::optional<QPointF> pickerFirstCornerPos;
    std::optional<QwtPlotPicker::RubberBand> rubberBand;

public:
    PlotModel(std::map<QwtPlot::Axis, AxisInfo>);
    void setRangedMeasurement(QwtPlot::Axis, e384CommLib::RangedMeasurement_t);
    std::optional<std::string> getUnitLabel(QwtPlot::Axis);
    std::map<QwtPlot::Axis, std::string> getUnitLabes();
    Rect4 getZoom();
    std::optional<QPointF> getPickerFirstCornerPos();
    std::optional<QwtPlotPicker *> getPicker();
    std::optional<QwtPlotPicker::RubberBand> getRubberBand();
    bool isAxisEnabled(QwtPlot::Axis);
    std::vector <QwtPlot::Axis> getActiveAxes();
    e384CommLib::RangedMeasurement_t getAxisRangedMeasurement(QwtPlot::Axis);

public slots:
    // virtual void onAxisChanged(QwtPlot::Axis, e384CommLib::RangedMeasurement_t) = 0;
    // virtual void onCurveColorChanged(std::map<uint16_t, QColor>) = 0;
    // virtual void onBackgroundColorChanged(QColor) = 0;
    void onZoomInPickerAppended(const QPointF &p, QWidget* canvas);
    void onZoomInPickerMoved(const QPointF &p);
    void onZoomInPickerSelected(const QRectF &r);
    void onSingleAxisZoom(QwtPlot::Axis ax, int zoomInFactor, QPointF mousePosition);
    void onSingleAxisShift(QwtPlot::Axis ax, int shiftFactor);
    void onZoomOut();
    void onZoomReset();

signals:
    void sigReplot();
    void sigRubberBandUpdated();
};

#endif // PLOTMODEL_H
