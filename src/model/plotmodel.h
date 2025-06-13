#ifndef PLOTMODEL_H
#define PLOTMODEL_H

#include <QObject>
#include <QwtPlot>
#include "e384commlib_global_addendum.h"
#include <map>
#include "zoom.h"
#include "qwt_plot_picker.h"
#include <QWheelEvent>

struct AxisInfo {
    // if log is false then scale is linear
    e384CommLib::RangedMeasurement_t range;
    std::optional<double> fixedMinimum = std::nullopt;
    bool log = false;
    bool inverted = false;
};

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
    // used for gapfree plot which has fixed minimum at 0;
    // std::optional<bool> getMinimumFixed();
    // virtual void setRangedMeasurement(QwtPlot::Axis, e384CommLib::RangedMeasurement_t) = 0;
    // virtual std::optional<std::string> getUnitLabel(QwtPlot::Axis) = 0;
    // virtual std::map<QwtPlot::Axis, std::string> getUnitLabes() = 0;
    void setRangedMeasurement(QwtPlot::Axis, e384CommLib::RangedMeasurement_t);
    std::optional<std::string> getUnitLabel(QwtPlot::Axis);
    std::map<QwtPlot::Axis, std::string> getUnitLabes();
    Rect4 getZoom();
    std::optional<QPointF> getPickerFirstCornerPos();
    std::optional<QwtPlotPicker *> getPicker();
    std::optional<QwtPlotPicker::RubberBand> getRubberBand();
    bool isAxisEnabled(QwtPlot::Axis);

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
