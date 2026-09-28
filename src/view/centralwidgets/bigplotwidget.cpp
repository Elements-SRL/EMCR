#include "bigplotwidget.h"
#include "bigplot.h"
#include "qlineedit.h"
#include "globaldefines.h"
#include <qlabel.h>
#include <qregularexpression.h>
#include <qsettings.h>
#include <qvalidator.h>

BigPlotWidget::BigPlotWidget(uint16_t channelsNumber, QWidget * parent) :
    QTabWidget(parent) {
    mainVl = new QVBoxLayout();
    mainVl->setContentsMargins(6, 0, 0, 6);
    this->setLayout(mainVl);
    this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->setObjectName("bigPlotTabWidget");

    // Corner widget - extends the tab menu
    QWidget* tabCornerWidget = new QWidget(this);

    projectNameLbl = new QLineEdit(tabCornerWidget);
    projectNameLbl->setObjectName("projectNameLbl");
    projectNameLbl->setText(PSD_DEFAULT_PROJECT_NAME);

    QLabel* prefixLabel = new QLabel("PROJECT:", projectNameLbl);
    prefixLabel->setObjectName("projectNamePrefixLbl");

    // Sanitizer - Prevents special chars  \ / : * ? " < > |
    QRegularExpression rx("^[^\\\\/:*?\"<>|]{1,100}$");
    QRegularExpressionValidator *validator = new QRegularExpressionValidator(rx, projectNameLbl);
    projectNameLbl->setValidator(validator);

    // Perfix inside prjLabel
    int prefixWidth = prefixLabel->fontMetrics().horizontalAdvance("PROJECT:") + 6;
    projectNameLbl->setTextMargins(prefixWidth, 0, 0, 0);

    QHBoxLayout* prefixLayout = new QHBoxLayout(projectNameLbl);
    prefixLayout->setContentsMargins(6, 0, 0, 0);
    prefixLayout->addWidget(prefixLabel);
    prefixLayout->addStretch();
    projectNameLbl->setLayout(prefixLayout);

    connect(projectNameLbl, &QLineEdit::editingFinished, [=]() {
        emitProjectName();
        projectNameLbl->clearFocus();
    });

    QHBoxLayout* pHLayout = new QHBoxLayout(tabCornerWidget);
    tabCornerWidget->setContentsMargins(0, 2, 0, 0);
    pHLayout->addWidget(projectNameLbl);
    this->setCornerWidget(tabCornerWidget, Qt::TopRightCorner);

}

void BigPlotWidget::emitProjectName(){
    QSettings settings;
    QString filename = projectNameLbl->text().trimmed();

    while (filename.endsWith('.')) {
        filename.chop(1);
    }

    if (filename.isEmpty()) {
        filename = PSD_DEFAULT_PROJECT_NAME;
    }

    projectNameLbl->setText(filename);
    settings.setValue(PSD_DEFAULT_PROJECT_NAME, filename);
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
    gapFreeLayout->setContentsMargins(10,10,10,10);
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
