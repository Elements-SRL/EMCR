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
    auto spectrumLayout = new QVBoxLayout();
    spectrumTab = new QWidget();
    spectrumTab->setLayout(spectrumLayout);
    addTab(spectrumTab, "Spectrum");

    // Create the fourth tab
    auto eventDetectionLayout = new QVBoxLayout();
    eventDetectionTab = new QWidget();
    eventDetectionTab->setLayout(eventDetectionLayout);
    // todo this will change in the future to support multichannel devices
    addTab(eventDetectionTab, "Event Detection");
    if (channelsNumber > 1) {
        this->setTabEnabled(3, false);
    }
    mainVl->setContentsMargins(6, 0, 0, 6);
    mainVl->setSpacing(1);
    this->setLayout(mainVl);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void BigPlotWidget::setGapFreePlot(QWidget* wid) {
    gapFreeTab->layout()->addWidget(wid);
}

void BigPlotWidget::setIvGraph(QWidget* wid){
    ivTab->layout()->addWidget(wid);
}

void BigPlotWidget::setEventDetectionTab(QWidget * wid) {
    eventDetectionTab->layout()->addWidget(wid);
}

void BigPlotWidget::setSpectrumPlot(QWidget* wid) {
    spectrumTab->layout()->addWidget(wid);
}

void BigPlotWidget::setTabsStatus(bool status, int idxOfDefaultEnabledTab) {
    for (int i = 0; i < this->count(); i++) {
        if (i != idxOfDefaultEnabledTab) {
            setTabEnabled(i, status);
        }
    }
}
