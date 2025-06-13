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

};

#endif // PLOTCONTROLLER_H
