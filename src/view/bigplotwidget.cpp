#include "bigplotwidget.h"

BigPlotWidget::BigPlotWidget(QWidget * parent) :
    QWidget(parent) {

    mainVl = new QVBoxLayout();
    mainVl->setContentsMargins(6, 0, 0, 6);
    mainVl->setSpacing(1);
    this->setLayout(mainVl);
}

void BigPlotWidget::setPlot(BigPlot * plot) {
    mainVl->addWidget(plot);
}

