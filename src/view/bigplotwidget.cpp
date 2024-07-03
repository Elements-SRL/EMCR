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
#ifdef DEBUG
    addTab(eventDetectionTab, "Event Detection");
#else
    eventDetectionTab->setVisible(false);
#endif

    // Create the fourth tab
    auto spectrumLayout = new QVBoxLayout();
    spectrumTab = new QWidget();
    spectrumTab->setLayout(spectrumLayout);
#ifdef DEBUG
    addTab(spectrumTab, "Spectrum");
#else
    spectrumTab->setVisible(false);
#endif

    mainVl->setContentsMargins(6, 0, 0, 6);
    mainVl->setSpacing(1);
    this->setLayout(mainVl);
}

void BigPlotWidget::setGapFreePlot(BigPlot * wid) {
    gapFreeTab->layout()->addWidget(wid);
}

void BigPlotWidget::setIvGraph(BigPlot * wid){
    ivTab->layout()->addWidget(wid);
}

void BigPlotWidget::setEventDetectionTab(QWidget * wid) {
    eventDetectionTab->layout()->addWidget(wid);
}

void BigPlotWidget::setSpectrumPlot(BigPlot * wid) {
    spectrumTab->layout()->addWidget(wid);
}
