#include "bigplotdockwidget.h"

#include <QBoxLayout>


BigPlotDockWidget::BigPlotDockWidget(MessageDispatcher * msgDisp, QWidget * parent) :
    QDockWidget(parent) {

    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);

    QWidget * mainWg = new QWidget();
    mainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setWindowTitle("Enlarged traces");

    this->setWidget(mainWg);

    QVBoxLayout * mainVl = new QVBoxLayout();
    mainVl->setContentsMargins(6, 0, 0, 6);
    mainVl->setSpacing(1);
    mainWg->setLayout(mainVl);

    plot = new BigPlot("", "s", "", this);
    plot->enableAxis(QwtPlot::yRight);
    mainVl->addWidget(plot);

    for(int i = 0; i < currentChannelsNum; i++){
        currentCurves.append(new Curve(CurveType_t::CurveTypePlotSolid));
    }

    for(int i = 0; i < voltageChannelsNum; i++){
        voltageCurves.append(new Curve(CurveType_t::CurveTypePlotSolid));
        voltageCurves[i]->setColor(QColor(Qt::red));
        voltageCurves[i]->setYAxis(QwtPlot::yRight);
    }
}

void BigPlotDockWidget::clearCurves() {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[idx]->detach();
        delete currentCurves[idx];
        delete [] currentCurves[idx];
    }

    for (int idx = 0; idx < voltageChannelsNum; idx++) {
        voltageCurves[idx]->detach();
        delete voltageCurves[idx];
        delete [] voltageCurves[idx];
    }

    currentCurves.clear();
    voltageCurves.clear();
}

void BigPlotDockWidget::onRangeUpdated(RangedMeasurement_t newRange, QwtPlot::Axis axisIdx) {
    plot->onRangeUpdated(newRange, axisIdx);
}

void BigPlotDockWidget::onDurationUpdated(Measurement_t duration) {
    plot->onDurationUpdated(duration);
}

void BigPlotDockWidget::onSetGapFreePlotData(double * timeValues, QVector <double *> * voltageValues, QVector <double *> * currentValues, int dataSize, int channelsToPlotNumber) {
    for (int idx = 0; idx < channelsToPlotNumber; idx++) {
        currentCurves.at(idx)->attach(plot);
        currentCurves.at(idx)->setRawSamples(timeValues, currentValues->at(idx), dataSize);

        voltageCurves.at(idx)->attach(plot);
        voltageCurves.at(idx)->setRawSamples(timeValues, voltageValues->at(idx), dataSize);
    }

    for (int idx = channelsToPlotNumber; idx < currentChannelsNum; idx++) {
        currentCurves.at(idx)->detach();
        voltageCurves.at(idx)->detach();
    }
}

void BigPlotDockWidget::onReplot() {
    plot->replot();
}
