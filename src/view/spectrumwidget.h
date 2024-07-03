#ifndef SPECTRUMWIDGET_H
#define SPECTRUMWIDGET_H

#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QPushButton>

class SpectrumWidget : public QDockWidget {
    Q_OBJECT

public:
    SpectrumWidget(uint32_t channelsNum, QWidget * parent = nullptr);

private:
    QPushButton * startButton;
    QPushButton * stopButton;
    QDoubleSpinBox * integrationWindowS;

signals:
    void startPressed();
    void stopPressed();
    void integrationWindowChanged(double windowS);
};

#endif // SPECTRUMWIDGET_H
