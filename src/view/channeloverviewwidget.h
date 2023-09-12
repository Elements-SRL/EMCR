#ifndef CHANNELOVERVIEWWIDGET_H
#define CHANNELOVERVIEWWIDGET_H

#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include "stampplot.h"

class ChannelOverviewWidget : public QFrame {
    Q_OBJECT

public:
    typedef enum {
        Plot,
        Noise
    } VisualizationOption_t;

    ChannelOverviewWidget(QWidget * parent = nullptr);

    void setVisualizationOption(VisualizationOption_t type);
    void setChannelIndex(int index);
    void setStampPlot(StampPlot * plot);
    void setNoiseValue(Measurement_t value);

private:
    QLabel * channelLbl;
    QLabel * rmsLbl;
    QVBoxLayout * mainVl;
    StampPlot * plot;

signals:
    void clicked(QMouseEvent *event);
};

#endif // CHANNELOVERVIEWWIDGET_H
