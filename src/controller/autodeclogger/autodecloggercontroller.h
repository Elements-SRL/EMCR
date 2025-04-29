#ifndef AUTODECLOGGERCONTROLLER_H
#define AUTODECLOGGERCONTROLLER_H

#include <vector>
#include "e384commlib_global_addendum.h"

#include <QObject>
#include <QVector>

#include "mainwindow.h"
#include "application_status.h"
#include "messagedispatcher.h"
#include "autodeclogger/autodecloggerwidget.h"
#include "autodeclogger/autodecloggermodel.h"
#include "autodecloggerconsumer.h"
#include "controllerwithconsumer.h"

class AutoDecloggerController : public ControllerWithConsumer {
    Q_OBJECT

public:
    AutoDecloggerController(ApplicationStatus* appStatus, MainWindow* mainWindow, DeviceDataProducer *ddt);
    ~AutoDecloggerController();

    std::vector <DeviceDataConsumer *> getConsumers() override;

private:
    AutoDecloggerModel * model = nullptr;
    AutoDecloggerWidget* widget = nullptr;
    AutodecloggerConsumer* consumer = nullptr;
    MainWindow* mainWindow = nullptr;

public slots:
    void onActive(bool);
    void onThFieldChanged(double);
    void onVoltageFieldChanged(double);
    void onTimeFieldChanged(double);
    void onTimeBelowThresholdChanged(double);
    void onDecloggingStarted(std::vector<unsigned short>);
    void onDecloggingCompleted(std::vector<unsigned short>);

signals:
    // signals sent to Compensation Control Widget
    void sigDecloggingStarted();
    void sigDecloggingCompleted();
};

#endif // AUTODECLOGGERCONTROLLER_H
