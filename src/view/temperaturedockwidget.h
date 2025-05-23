#ifndef TEMPERATUREDOCKWIDGET_H
#define TEMPERATUREDOCKWIDGET_H

#include <QDockWidget>
#include <QLabel>
#include <QLayout>
#include <QDoubleSpinBox>

#include "e384commlib_global.h"

namespace e384cl = e384CommLib;

class TemperatureDockWidget : public QDockWidget {
    Q_OBJECT

public:
    TemperatureDockWidget(QWidget * parent = nullptr);

    void setChannels(int channelsNum);
    void enableFansControls(e384cl::RangedMeasurement_t range);
    e384cl::Measurement_t getTSet();

public slots:
    void onTemperatureRead(std::vector <e384cl::Measurement_t> values);

private:
    QVBoxLayout * mainVl = nullptr;
    QDoubleSpinBox * setTSbx = nullptr;
    QDoubleSpinBox * setT1Sbx = nullptr;

    QVector <QLabel *> temperatureLbls;

signals:
    void sigSetFanSpeed(e384cl::Measurement_t speed);
    void sigEnableKTControl(bool enable);
    void sigEnableTControl(e384cl::Measurement_t temperature, bool enable);
    void sigPidParams(double pg, double ig, double ieMax);
};

#endif // TEMPERATUREDOCKWIDGET_H
