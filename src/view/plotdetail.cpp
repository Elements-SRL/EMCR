#include "plotdetail.h"
#include <QVBoxLayout>
#include <QLabel>

PlotDetail::PlotDetail(PlotDetailModel * pdm, QWidget * parent):
    QWidget(parent),
    pdm(pdm){
    // this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    auto mainVl = new QVBoxLayout(this);
    mainVl->addWidget(new QLabel(QString::fromStdString(pdm->getLabel())));
    plot = new BasePlot("Detailed", "s", "A", parent);
    mainVl->addWidget(plot);
    pdm->getCurve()->attach(plot);
    // plot->setAxisAutoScale(QwtPlot::yLeft);
    plot->setAxisScale(QwtPlot::yLeft, -200, 200);
    // plot->setAxisScale(QwtPlot::xBottom, 0, 2);
}

uint16_t PlotDetail::getChannel(){
    return pdm->getChannel();
}

void PlotDetail::closeEvent(QCloseEvent *event)  {
    emit close();
}

void PlotDetail::replot() {
    plot->update();
    plot->replot();
}
