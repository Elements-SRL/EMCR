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

enum BigPlotStatus {
    GapFree = 0,
    Iv = 1
};

class BigPlot : public QwtPlot {
    Q_OBJECT

private:
    void setAndFormatText(QString, QwtTextLabel *, Qt::AlignmentFlag = Qt::AlignLeft);
    BigPlotStatus status = BigPlotStatus::GapFree;
public:
    BigPlot(QString titleString, QString xUnitString, QString yUnitString, QWidget * parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void setLabel(QString, QwtPlot::Axis);
    void shiftVertAxis(Axis axis, double shiftValue);
    void setRect(Rect4 r);
    Rect4 getRect();
    void drawCanvas(QPainter * p) override;
    void setStatus(BigPlotStatus status);
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
    commlib::UnitPfx_t xAxisPrefix = commlib::UnitPfxNone;

protected slots:
    virtual void onZoomInPickerAppended(const QPointF &p);
    virtual void onZoomInPickerMoved(const QPointF &p);
    virtual void onZoomInPickerSelected(const QRectF &r);
    virtual void onZoomOutPickerSelected(const QPointF &p);
    virtual void onZoomResetPickerSelected(const QPointF &p);

private:
    void handleLabelsPosition();

    QVector <bool> rangeInitialized;

signals:
    void zoomInRequest(Rect4 rect);
    void zoomOutRequest();
    void zoomResetRequest();
    void singleAxisZoomRequest(QwtPlot::Axis, int, QPointF);
    void singleAxisShiftRequest(QwtPlot::Axis, int);
};

#endif // BIGPLOT_H
