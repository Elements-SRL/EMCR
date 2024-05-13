#ifndef CENTRALWIDGETCONTROLLER_H
#define CENTRALWIDGETCONTROLLER_H

#include "bigplotwidget.h"
#include "mainwindow.h"
#include "messagedispatcher.h"
#include "bigplotmodel.h"
#include "plotconsumer.h"
#include <QPointF>
#include "application_status.h"
#include "plotmessage.h"
#include "ivgraphconsumer.h"
#include "eventdetectionconsumer.h"
#include "plotconsumer.h"
#include "ivgraphwidget.h"
#include "application_status.h"

class CentralWidgetController : public QObject {
    Q_OBJECT

public:
    CentralWidgetController(ApplicationStatus* appStatus, DeviceDataProducer* producer, BigPlotWidget* bigPlotWidget);
    ~CentralWidgetController();

    //void clean();

protected:
    BigPlotWidget* bigPlotWidget;
    ApplicationStatus* appStatus;
    DeviceDataProducer* producer;

    int currentChannelsNum;
    int voltageChannelsNum;
};

#endif // CENTRALWIDGETCONTROLLER_H
