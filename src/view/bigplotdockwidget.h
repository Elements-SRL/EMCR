#ifndef BIGPLOTDOCKWIDGET_H
#define BIGPLOTDOCKWIDGET_H

#include <QDockWidget>

#include "bigplot.h"

class BigPlotDockWidget : public QDockWidget {
    Q_OBJECT

public:
    BigPlotDockWidget();

private:
    BigPlot * plot = nullptr;
};

#endif // BIGPLOTDOCKWIDGET_H
