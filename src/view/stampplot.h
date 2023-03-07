#ifndef STAMPPLOT_H
#define STAMPPLOT_H

#include "qwt_plot.h"
#include "qwt_plot_curve.h"
#include "qwt_scale_widget.h"
#include "qwt_plot_picker.h"
#include "qwt_picker_machine.h"

class StampPlot : public QwtPlot {
    Q_OBJECT

public:
    StampPlot(QWidget * parent = nullptr);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    QwtPlotPicker * selectPicker;
    QwtPlotPicker * deselectPicker;

public slots:
    void onSelected();
    void onDeselected();

signals:
    void selected(bool flag);
};

#endif // STAMPPLOT_H
