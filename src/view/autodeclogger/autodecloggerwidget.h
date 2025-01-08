#ifndef AUTODECLOGGERWIDGET_H
#define AUTODECLOGGERWIDGET_H

#include <QDockWidget>
#include <QBoxLayout>
#include <qcheckbox.h>
#include "QDoubleSpinBox"
#include "QLabel"
#include "messagedispatcher.h"

class AutoDecloggerWidget : public QDockWidget {
    Q_OBJECT

private:
    QCheckBox* active;
    QDoubleSpinBox* thField;
    QDoubleSpinBox* voltageField;
    QDoubleSpinBox* timeField;
    //add a red led

public:
    AutoDecloggerWidget(RangedMeasurement cr, RangedMeasurement vr, QWidget* parent = nullptr);
    double getThreshold();
    double getVoltage();
    double getTime();

public slots:
    //intercept change of current and voltage range
    void currentRangeChanged(RangedMeasurement cr);
    void voltageRangeChanged(RangedMeasurement vr);
};

#endif // AUTODECLOGGERWIDGET_H
