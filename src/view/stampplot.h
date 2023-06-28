#ifndef STAMPPLOT_H
#define STAMPPLOT_H

#define STP_STYLE_PLOT_ACTIVE "StampPlot { border: 3px solid green; }"
#define STP_STYLE_PLOT_INACTIVE "StampPlot { border: 2px solid black; }"

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
    StampPlot(int idealPlotWidth, int idealPlotHeight, QWidget * parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void setSelected(bool flag);

public slots:
    void onRangeUpdated(RangedMeasurement_t newRange, Axis axisIdx = yLeft);
    void onDurationUpdated(Measurement_t duration);

private:
    int idealPlotWidth;
    int idealPlotHeight;

    QwtPlotPicker * selectPicker;
    QwtPlotPicker * deselectPicker;

    QPointF pickerFirstCornerPos;

    RangedMeasurement_t currentRange[axisCnt];
    Measurement_t sweepDuration = {1.0, UnitPfxNone, "s"};
    UnitPfx_t xAxisPrefix = UnitPfxNone;

    QVector <bool> rangeInitialized;
    bool selected = false;

public slots:
    void onClicked();
    void onUnclicked();

signals:
    void clicked(bool flag);
};

#endif // STAMPPLOT_H
