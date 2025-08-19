#ifndef SPECTRUMWIDGET_H
#define SPECTRUMWIDGET_H

#include <QWidget>
#include <QDoubleSpinBox>
#include <QPushButton>

class SpectrumWidget : public QWidget {
    Q_OBJECT

public:
    SpectrumWidget(uint32_t channelsNum, QWidget* plot, QWidget * parent = nullptr);

private:
    QPushButton * startButton;
    QPushButton * stopButton;
    QDoubleSpinBox * integrationWindowS;

signals:
    void sigStartPressed();
    void sigStopPressed();
    void sigExportSpectrum();
    void sigIntegrationWindowChanged(double windowS);
    void sigAutoZoom();
};

#endif // SPECTRUMWIDGET_H
