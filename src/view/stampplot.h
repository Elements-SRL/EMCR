#ifndef STAMPPLOT_H
#define STAMPPLOT_H

#include <QMouseEvent>

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_picker.h"
#include "qwt_picker_machine.h"
#include "qwt_text_label.h"

#include "e384commlib_global.h"

#define SMP_LEGEND_SIZE 10

using namespace e384CommLib;

class StampPlot : public QwtPlot {
    Q_OBJECT

public:
    typedef enum States {
        StateNoEffects = 0x0000,
        StateSwitchedOff = 0x0001,
        StateStimuliDisabled = 0x0002,
        StateLiquidJunctionCompensation = 0x0004,
        StateTraceExpanded = 0x0008,
        StateCalibrationResistorsOn = 0x0010,
        StateOffsetRecalibrationOn = 0x0020
    } States_t;

    StampPlot(int channelIdx, std::string channelname, int idealPlotWidth, int idealPlotHeight, QWidget * parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void setSelected(bool flag);
    void setState(States_t newState);
    void addState(States_t newState);
    void removeState(States_t newState);
    void setLegendColor(QColor colot);
    void setName(std::string name);
    void drawCanvas(QPainter * p) override;

public slots:
    void onRangeUpdated(RangedMeasurement_t newRange);
    void onDurationUpdated(Measurement_t duration);

protected:
    virtual void resizeEvent(QResizeEvent * e) override;
    void mousePressEvent(QMouseEvent *event) override;

private:
    void handleLabelsPosition();

    int channelIdx = 0;
    int idealPlotWidth;
    int idealPlotHeight;

    States_t state = StateNoEffects;

    QwtTextLabel * channelIdxLbl;
    QwtTextLabel * stateLbl;

    QPointF pickerFirstCornerPos;

    RangedMeasurement_t currentRange[axisCnt];
    Measurement_t sweepDuration = {1.0, UnitPfxNone, "s"};
    UnitPfx_t xAxisPrefix = UnitPfxNone;

    bool selected = false;

    int xAxisMaxMajor;
    int yAxisMaxMajor;
    QFrame * colorLabel = nullptr;

signals:
    void clicked(QMouseEvent *event);
};

#endif // STAMPPLOT_H
