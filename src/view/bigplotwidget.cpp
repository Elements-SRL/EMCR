#include "bigplotwidget.h"

BigPlotWidget::BigPlotWidget(uint16_t channelsNumber, QWidget * parent) :
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
    // todo this will change in the future to support multichannel devices
    if (channelsNumber == 1) {
        addTab(eventDetectionTab, "Event Detection");
    } else {
        eventDetectionTab->setVisible(false);
    }
    
    // Create the fourth tab
    auto spectrumLayout = new QVBoxLayout();
    spectrumTab = new QWidget();
    spectrumTab->setLayout(spectrumLayout);
    addTab(spectrumTab, "Spectrum");

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
