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
