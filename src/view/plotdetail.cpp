#include "plotdetail.h"
#include <QVBoxLayout>
#include <QLabel>

PlotDetail::PlotDetail(PlotDetailModel * pdm, QWidget * parent):
    QWidget(parent),
    pdm(pdm){
    this->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    auto mainVl = new QVBoxLayout(this);
    mainVl->addWidget(new QLabel(QString::fromStdString(pdm->getLabel())));
    mainVl->addWidget(new QLabel("This is a very very long string, hopefully enough to move around widgets"));
}

uint16_t PlotDetail::getChannel(){
    return pdm->getChannel();
}

void PlotDetail::closeEvent(QCloseEvent *event)  {
    emit close();
}
