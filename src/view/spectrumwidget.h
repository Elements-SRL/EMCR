#ifndef SPECTRUMWIDGET_H
#define SPECTRUMWIDGET_H

#include <QDockWidget>

class SpectrumWidget : public QDockWidget {
    Q_OBJECT

public:
    SpectrumWidget(uint32_t channelsNum, QWidget * parent = nullptr);
};

#endif // SPECTRUMWIDGET_H
