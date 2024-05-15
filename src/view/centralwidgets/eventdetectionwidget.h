#ifndef EVENTDETECTIONWIDGET_H
#define EVENTDETECTIONWIDGET_H

#include <QWidget>
#include <qwt_plot.h>
#include <qwt_plot_histogram.h>
#include <qwt_legend.h>
#include <qwt_legend_label.h>
#include <QLineEdit>

class EventDetectionWidget : public QWidget
{
    Q_OBJECT

public:
    EventDetectionWidget(QWidget* parent = nullptr);
    ~EventDetectionWidget();

    QwtPlot* getPlot();

private:
    // Widgets for different parts of the layout
    QwtPlotHistogram* upperLeftHistogram;
    QwtPlotHistogram* bottomRightHistogram;
    QLineEdit* inputField1;
    QLineEdit* inputField2;
    QwtPlot* bottomLeftPlot;
};

#endif // EVENTDETECTIONWIDGET_H