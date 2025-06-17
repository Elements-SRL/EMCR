#ifndef PLOTCONTROLLER_H
#define PLOTCONTROLLER_H

#include <QObject>
#include "baseplot2.h"
#include <memory>

class PlotController: public QObject {
    Q_OBJECT

private:
    BasePlot2* bp;
    std::shared_ptr<PlotModel> pm;

public:
    PlotController(std::map<QwtPlot::Axis, AxisInfo>, QWidget *parent = nullptr);
    QwtPlot * getPlot();
    std::shared_ptr<PlotModel> getModel();
    void setRangedMeasurement(QwtPlot::Axis, e384CommLib::RangedMeasurement_t rm);

public slots:
    void onAutoZoom();

signals:
    void sigAutoZoom(std::vector <QwtPlot::Axis>);
    void sigPlotUpdated();
};

#endif // PLOTCONTROLLER_H
