#ifndef BIGPLOT_H
#define BIGPLOT_H

#include <QWheelEvent>

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_picker.h"
#include "qwt_picker_machine.h"
#include "qwt_plot_grid.h"
#include "qwt_text_label.h"

#include "conversionscaledraw.h"
#include "doubleclickmachine.h"
#include "rect4.h"

#include "e384commlib_global.h"

namespace commlib = e384CommLib;

class BigPlot : public QwtPlot {
    Q_OBJECT

public:
    BigPlot(QString titleString, QString xUnitString, QString yUnitString, QWidget * parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void setTitle(QString text);
    void setXUnit(QString text);
    void setYUnit(QString text);
    void shiftVertAxis(Axis axis, double shiftValue);
    void setRect(Rect4 r);
    Rect4 getRect();

public slots:
    void onHorzZoomInRequest(Rect4 * rect);
    void onVertZoomInRequest(Rect4 * rect);
    void onVertZoomFullRequest();
    void onUpdateBaseline(Axis axisIdx, double baseline);
    void onDurationUpdated(commlib::Measurement_t duration);

protected:
    typedef enum {
        PickerZoomRect,
        PickerZoomHorz,
        PickerZoomVert
    } PickerZoomType_t;

    virtual void resizeEvent(QResizeEvent * e) override;
    virtual void wheelEvent(QWheelEvent * e) override;
    virtual void recomputeXAxisFactor(double duration);

    QwtPlotPicker * zoomInPicker;
    QwtPlotPicker * zoomOutPicker;
    QwtPlotPicker * zoomResetPicker;
    PickerZoomType_t pickerZoomType = PickerZoomRect;
    double pickerZoomDiscriminantNorm = 1.0;

    QwtTextLabel * plotTitle;
    QwtTextLabel * xUnit;
    QwtTextLabel * yUnit;

    int xAxisMaxMajor;
    int yAxisMaxMajor;

    double yScale;

    QPointF pickerFirstCornerPos;

    ConversionScaleDraw * xBottomScaleDraw;

    commlib::RangedMeasurement_t currentRange[axisCnt];
    commlib::Measurement_t sweepDuration = {1.0, commlib::UnitPfxNone, "s"};
    commlib::UnitPfx_t xAxisPrefix = commlib::UnitPfxNone;

protected slots:
    virtual void onZoomInPickerAppended(const QPointF &p);
    virtual void onZoomInPickerMoved(const QPointF &p);
    virtual void onZoomInPickerSelected(const QRectF &r);
    virtual void onZoomOutPickerSelected(const QPointF &p);
    virtual void onZoomResetPickerSelected(const QPointF &p);

private:
    QVector <bool> rangeInitialized;

signals:
    void zoomInRequest(Rect4 rect);
    void zoomOutRequest();
    void zoomResetRequest();
};

#endif // BIGPLOT_H
