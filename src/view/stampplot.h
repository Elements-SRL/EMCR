#ifndef STAMPPLOT_H
#define STAMPPLOT_H

#include <QMouseEvent>
#include <QColor>
#include <QPixmap>
#include <QString>

#include "qlabel.h"
#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_picker.h"
#include "qwt_picker_machine.h"
#include "qwt_text_label.h"

#include "e384commlib_global.h"

#define SMP_LEGEND_SIZE 10

namespace e384cl = e384CommLib;

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
        StateOffsetRecalibrationOn = 0x0020,
        StatePlotDetailOn = 0x0040,
    } States_t;

    StampPlot(int channelIdx, std::string channelname, int idealPlotWidth, int idealPlotHeight, QWidget * parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    void setSelected(bool flag);
    bool isSelected();
    void setState(States_t newState);
    void addState(States_t newState);
    void removeState(States_t newState);
    void setLegendColor(QColor colot);
    void setName(std::string name);
    void drawCanvas(QPainter * p) override;

    /*! Replot for new data. Plots that are not on screen (hidden, scrolled out,
     *  minimized window) are only marked dirty and redrawn when they are painted. */
    void replotData();
    bool takeDataDirty();

public slots:
    void onRangeUpdated(e384cl::RangedMeasurement_t newRange);
    void onDurationUpdated(e384cl::Measurement_t duration);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;

private:
    
    /*! Custom overlay drawing - status badges, channel name etc.
     *  Draw is made by custom QPainter actions rather than stylized
     *  child widgets. */
    void drawOverlays(QPainter *p);
    const QPixmap &namePixmap(qreal dpr);
    void invalidateOverlayCache();
    void requestReplot();

    int channelIdx = 0;
    int idealPlotWidth;
    int idealPlotHeight;

    States_t state = StateNoEffects;
    QString channelName;
    QColor legendColor = Qt::white;

    QPixmap nameCache;          // name box already rendered (for the current DPR)
    bool replotPending = false;
    bool dataDirty = false;

    QPointF pickerFirstCornerPos;

    e384cl::RangedMeasurement_t currentRange[axisCnt];
    e384cl::Measurement_t sweepDuration = {1.0, e384cl::UnitPfxNone, "s"};
    e384cl::UnitPfx_t xAxisPrefix = e384cl::UnitPfxNone;

    bool selected = false;

    int xAxisMaxMajor;
    int yAxisMaxMajor;

signals:
    void clicked(QMouseEvent *event);
};

#endif // STAMPPLOT_H
