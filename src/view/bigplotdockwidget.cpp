#include "bigplotdockwidget.h"

#include <QBoxLayout>

BigPlotDockWidget::BigPlotDockWidget(QWidget * parent) :
    QDockWidget(parent) {

    QWidget * mainWg = new QWidget();
    mainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setWindowTitle("Kawaii");

    this->setWidget(mainWg);

    QVBoxLayout * mainVl = new QVBoxLayout();
    mainVl->setContentsMargins(0, 0, 0, 0);
    mainVl->setSpacing(1);
    mainWg->setLayout(mainVl);

    plot = new BigPlot("", "s", "", this);
    mainVl->addWidget(plot);
}

void BigPlotDockWidget::onRangeUpdated(RangedMeasurement_t newRange) {
//    for (auto plot : plots) {
//        plot->onRangeUpdated(newRange);
//    }
}

void BigPlotDockWidget::onDurationUpdated(Measurement_t duration) {
//    for (auto plot : plots) {
//        plot->onDurationUpdated(duration);
//    }
}

void BigPlotDockWidget::onSetGapFreePlotData(double * timeValues, QVector <double *> * voltageValues, QVector <double *> * currentValues, int dataSize) {
//    for (int idx = 0; idx < currentChannelsNum; idx++) {
//        currentCurves.at(idx)->setRawSamples(timeValues, currentValues->at(idx), dataSize);
//    }
}

void BigPlotDockWidget::onReplot() {
//    for (auto plot : plots) {
//        plot->replot();
//    }
}
