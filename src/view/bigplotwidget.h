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
    void setEventDetectionTab(QWidget* eventDetectionWidget);

private:
    QVBoxLayout * mainVl;
    QWidget * gapFreeTab;
    QWidget * ivTab;
    QWidget* eventDetectionTab;

};

#endif // BIGPLOTWIDGET_H
