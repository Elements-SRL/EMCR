#ifndef STAMPPLOT_H
#define STAMPPLOT_H

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_picker.h"
#include "qwt_picker_machine.h"

#include "e384commlib_global.h"

using namespace e384CommLib;

class StampPlot : public QwtPlot {
    Q_OBJECT

public:
    StampPlot(QWidget * parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

public slots:
    void onRangeUpdated(RangedMeasurement_t newRange, Axis axisIdx = yLeft);
    void onDurationUpdated(Measurement_t duration);

private:
    QwtPlotPicker * selectPicker;
    QwtPlotPicker * deselectPicker;

    QPointF pickerFirstCornerPos;

    RangedMeasurement_t currentRange[axisCnt];
    Measurement_t sweepDuration = {1.0, UnitPfxNone, "s"};
    UnitPfx_t xAxisPrefix = UnitPfxNone;

    QVector <bool> rangeInitialized;

public slots:
    void onSelected();
    void onDeselected();

signals:
    void selected(bool flag);
};

#endif // STAMPPLOT_H
