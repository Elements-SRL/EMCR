#include "bigplotwidget.h"
#include "plotmessage.h"
#include "bigplot.h"

BigPlotWidget::BigPlotWidget(uint16_t channelsNumber, QWidget * parent) :
    QTabWidget(parent) {
    mainVl = new QVBoxLayout();
    mainVl->setContentsMargins(6, 0, 0, 6);
    mainVl->setSpacing(1);
    this->setLayout(mainVl);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setObjectName("BigPlotCanvas");
}

void BigPlotWidget::setGapFreePlot(QWidget* wid) {
    gapFreeTab->layout()->addWidget(wid);
}

void BigPlotWidget::setEpisodicPlot(QWidget* wid) {
    episodicTab->layout()->addWidget(wid);
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
        bool ok;
        auto tabID = this->widget(i)->property("TabID").toInt(&ok);
        if (ok && (tabID != idxOfDefaultEnabledTab)) {
            setTabEnabled(i, status);
        }
    }
}

void BigPlotWidget::addEpisodicTab() {
    auto episodicLayout = new QVBoxLayout();
    episodicTab = new QWidget();
    episodicTab->setLayout(episodicLayout);
    episodicTab->setProperty("TabID", BigPlot::BigPlotStatus::Episodic);
    addTab(episodicTab, "Episodic");
}

void BigPlotWidget::addIvTab() {
    auto ivLayout = new QVBoxLayout();
    ivTab = new QWidget();
    ivTab->setLayout(ivLayout);
    ivTab->setProperty("TabID", BigPlot::BigPlotStatus::Iv);
    addTab(ivTab, "IV Graph");
}

void BigPlotWidget::addSpectrumTab() {
    auto spectrumLayout = new QVBoxLayout();
    spectrumTab = new QWidget();
    spectrumTab->setLayout(spectrumLayout);
    spectrumTab->setProperty("TabID", BigPlot::BigPlotStatus::Spectrum);
    addTab(spectrumTab, "Spectrum");
}

void BigPlotWidget::addGapFreeTab() {
    auto gapFreeLayout = new QVBoxLayout();
    gapFreeTab = new QWidget();
    gapFreeTab->setLayout(gapFreeLayout);
    gapFreeTab->setProperty("TabID", BigPlot::BigPlotStatus::GapFree);
    addTab(gapFreeTab, "GapFree");
}

void BigPlotWidget::addEventDetectionTab() {
    auto eventDetectionLayout = new QVBoxLayout();
    eventDetectionTab = new QWidget();
    eventDetectionTab->setLayout(eventDetectionLayout);
    eventDetectionTab->setProperty("TabID", BigPlot::BigPlotStatus::Event);
    addTab(eventDetectionTab, "Event Detection");
}
