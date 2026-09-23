#ifndef BIGPLOTWIDGET_H
#define BIGPLOTWIDGET_H

#include <QWidget>
#include <QBoxLayout>

#include <QTabWidget>
#include <qlineedit.h>

class BigPlotWidget : public QTabWidget {
    Q_OBJECT

public:
    BigPlotWidget(uint16_t channelsNumber, QWidget * parent = nullptr);
    void setGapFreePlot(QWidget* wid);
    void setEpisodicPlot(QWidget* wid);
    void setIvGraph(QWidget* wid);
    void setEventDetectionTab(QWidget * wid);
    void setSpectrumPlot(QWidget * wid);
    void setTabsStatus(bool status, int idxOfDefaultEnabledTab);
    void addEpisodicTab();
    void addIvTab();
    void addSpectrumTab();
    void addGapFreeTab();
    void addEventDetectionTab();
    void emitProjectName();

private:
    QVBoxLayout * mainVl;
    QWidget * gapFreeTab;
    QWidget * episodicTab;
    QWidget * ivTab;
    QWidget * spectrumTab;
    QWidget * eventDetectionTab;
    QLineEdit * projectNameLbl;
};

#endif // BIGPLOTWIDGET_H
