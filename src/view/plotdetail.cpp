#include "plotdetail.h"
#include <QVBoxLayout>
#include <QLabel>

PlotDetail::PlotDetail(PlotDetailModel * pdm, QWidget * parent):
    QWidget(parent),
    pdm(pdm){
    auto mainVl = new QVBoxLayout(this);
    plot = new BasePlot(pdm->getLabel(), "s", pdm->getUom(), parent);
    mainVl->addWidget(plot);
    pdm->getCurve()->attach(plot);
    setWindowFlags(Qt::Window);
    setObjectName("PlotDetail");
}

uint16_t PlotDetail::getChannel(){
    return pdm->getChannel();
}

void PlotDetail::closeEvent(QCloseEvent *event)  {
    emit close();
}

void PlotDetail::replot() {
    plot->replot();
}

void PlotDetail::updateLabel() {
    plot->setLabel(this->pdm->getLabel(), QwtPlot::Axis::yLeft);
}

void PlotDetail::updatePlot() {
    const auto by = pdm->getBottomY();
    const auto ty = pdm->getTopY();
    plot->getPlot()->setAxisScale(QwtPlot::Axis::yLeft, by, ty);
}

void PlotDetail::setBackgroundColor(QColor c) {
    plot->setCanvasBackground(c);
}
