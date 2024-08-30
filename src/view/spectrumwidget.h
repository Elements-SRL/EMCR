#ifndef SPECTRUMWIDGET_H
#define SPECTRUMWIDGET_H

#include <QWidget>
#include <QDoubleSpinBox>
#include <QPushButton>
#include "bigplot.h"

class SpectrumWidget : public QWidget {
    Q_OBJECT

public:
    SpectrumWidget(uint32_t channelsNum, BigPlot* plot, QWidget * parent = nullptr);

private:
    QPushButton * startButton;
    QPushButton * stopButton;
    QDoubleSpinBox * integrationWindowS;

signals:
    void startPressed();
    void stopPressed();
    void exportSpectrum();
    void integrationWindowChanged(double windowS);
};

#endif // SPECTRUMWIDGET_H
