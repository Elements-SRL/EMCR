#ifndef STAMPPLOT_H
#define STAMPPLOT_H

#define STP_STYLE_PLOT_ACTIVE "StampPlot { border: 2px solid green; }"
#define STP_STYLE_PLOT_INACTIVE "StampPlot { border: 1px solid black; }"

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_picker.h"
#include "qwt_picker_machine.h"
#include "qwt_text_label.h"

#include "e384commlib_global.h"

using namespace e384CommLib;

class StampPlot : public QwtPlot {
    Q_OBJECT

public:
    typedef enum States {
        StateNoEffects = 0x0000,
        StateSwitchedOff = 0x0001,
        StateStimuliDisabled = 0x0002,
        StateOffsetCompensation = 0x0004
    } States_t;

    StampPlot(int channelIdx, int idealPlotWidth, int idealPlotHeight, QWidget * parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void setSelected(bool flag);
    void setState(States_t newState);
    void addState(States_t newState);
    void removeState(States_t newState);

public slots:
    void onRangeUpdated(RangedMeasurement_t newRange, Axis axisIdx = yLeft);
    void onDurationUpdated(Measurement_t duration);

    void onClicked();
    void onUnclicked();

protected:
    virtual void resizeEvent(QResizeEvent * e) override;

private:
    int channelIdx = 0;
    int idealPlotWidth;
    int idealPlotHeight;

    States_t state = StateNoEffects;

    QwtTextLabel * channelIdxLbl;
    QwtTextLabel * stateLbl;

    QwtPlotPicker * selectPicker;
    QwtPlotPicker * deselectPicker;

    QPointF pickerFirstCornerPos;

    RangedMeasurement_t currentRange[axisCnt];
    Measurement_t sweepDuration = {1.0, UnitPfxNone, "s"};
    UnitPfx_t xAxisPrefix = UnitPfxNone;

    QVector <bool> rangeInitialized;
    bool selected = false;

    int xAxisMaxMajor;
    int yAxisMaxMajor;

signals:
    void clicked(bool flag);
};

#endif // STAMPPLOT_H
