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
    BigPlotWidget(uint16_t channelsNumber, QWidget * parent = nullptr);
    void setGapFreePlot(QWidget* wid);
    void setIvGraph(QWidget* wid);
    void setEventDetectionTab(QWidget * wid);
    void setSpectrumPlot(QWidget * wid);

private:
    QVBoxLayout * mainVl;
    QWidget * gapFreeTab;
    QWidget * ivTab;
    QWidget * eventDetectionTab;
    QWidget * spectrumTab;

};

#endif // BIGPLOTWIDGET_H
