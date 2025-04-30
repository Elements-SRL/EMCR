#ifndef TEMPERATUREDOCKWIDGET_H
#define TEMPERATUREDOCKWIDGET_H

#include <QDockWidget>
#include <QLabel>
#include <QLayout>

#include "e384commlib_global.h"

namespace e384cl = e384CommLib;

class TemperatureDockWidget : public QDockWidget {
    Q_OBJECT

public:
    TemperatureDockWidget(QWidget * parent = nullptr);

    void setChannels(int channelsNum);

public slots:
    void onTemperatureRead(std::vector <e384cl::Measurement_t> values);

private:
    QVBoxLayout * mainVl = nullptr;

    QVector <QLabel *> temperatureLbls;
};

#endif // TEMPERATUREDOCKWIDGET_H
