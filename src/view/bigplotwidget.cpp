#include "bigplotwidget.h"

BigPlotWidget::BigPlotWidget(QWidget * parent) :
    QTabWidget(parent) {

    mainVl = new QVBoxLayout();

    // Create the first tab
    auto gapFreeLayout = new QVBoxLayout();
    gapFreeTab = new QWidget();
    gapFreeTab->setLayout(gapFreeLayout);
    addTab(gapFreeTab, "GapFree");

    // Create the second tab
    auto ivLayout = new QVBoxLayout();
    ivTab = new QWidget();
    ivTab->setLayout(ivLayout);
    addTab(ivTab, "IV Graph");

    // Create the third tab
    auto eventDetectionLayout = new QVBoxLayout();
    eventDetectionTab = new QWidget();
    eventDetectionTab->setLayout(eventDetectionLayout);
    addTab(eventDetectionTab, "Event Detection");

    mainVl->setContentsMargins(6, 0, 0, 6);
    mainVl->setSpacing(1);
    this->setLayout(mainVl);
}

void BigPlotWidget::setGapFreePlot(BigPlot * gapFreePlot) {
    gapFreeTab->layout()->addWidget(gapFreePlot);
}

void BigPlotWidget::setIvGraph(BigPlot * ivGraph){
    ivTab->layout()->addWidget(ivGraph);
}

void BigPlotWidget::setEventDetectionTab(QWidget* eventDetectionWidget) {
    eventDetectionTab->layout()->addWidget(eventDetectionWidget);
}
