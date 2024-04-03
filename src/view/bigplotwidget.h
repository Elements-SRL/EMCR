#ifndef BIGPLOTWIDGET_H
#define BIGPLOTWIDGET_H

#include <QWidget>
#include <QBoxLayout>

#include "messagedispatcher.h"
#include "bigplot.h"
#include "curve.h"
#include <QTabWidget>

class BigPlotWidget : public QTabWidget {
    Q_OBJECT

public:
    BigPlotWidget(QWidget * parent = nullptr);
    void setGapFreePlot(BigPlot * );
    void setIvGraph(BigPlot * );

private:
    QVBoxLayout * mainVl;
    QWidget * gapFreeTab;
    QWidget * ivTab;

};

#endif // BIGPLOTWIDGET_H
