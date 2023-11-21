#ifndef BIGPLOTWIDGET_H
#define BIGPLOTWIDGET_H

#include <QWidget>
#include <QBoxLayout>

#include "messagedispatcher.h"
#include "bigplot.h"
#include "curve.h"

class BigPlotWidget : public QWidget {
    Q_OBJECT

public:
    BigPlotWidget(QWidget * parent = nullptr);

    void setPlot(BigPlot * plot);

private:
    QVBoxLayout * mainVl;
};

#endif // BIGPLOTWIDGET_H
