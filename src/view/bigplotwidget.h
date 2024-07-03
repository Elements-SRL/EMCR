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
    void setGapFreePlot(BigPlot * wid);
    void setIvGraph(BigPlot * wid);
    void setEventDetectionTab(QWidget * wid);
    void setSpectrumPlot(BigPlot * wid);

private:
    QVBoxLayout * mainVl;
    QWidget * gapFreeTab;
    QWidget * ivTab;
    QWidget * eventDetectionTab;
    QWidget * spectrumTab;

};

#endif // BIGPLOTWIDGET_H
