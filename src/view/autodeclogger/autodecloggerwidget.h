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
    QDoubleSpinBox* timeBelowThreshold;
    QLabel* freePore;
    QLabel* cloggedPore;
    //add a red led

public:
    AutoDecloggerWidget(RangedMeasurement cr, RangedMeasurement vr, QWidget* parent = nullptr);
    double getThreshold();
    double getVoltage();
    double getTime();
    double getTimeBelowThreshold();

signals:
    void sigActive(bool);
    void sigThFieldChanged(double);
    void sigVotageFieldChanged(double);
    void sigTimeFieldChanged(double);
    void sigTimeBelowThresholdChanged(double);

public slots:
    //intercept change of current and voltage range
    void onPoreClogged();
    void onPoreFree();
    void currentRangeChanged(RangedMeasurement cr);
    void voltageRangeChanged(RangedMeasurement vr);
};

#endif // AUTODECLOGGERWIDGET_H
