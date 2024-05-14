#ifndef EVENTDETECTIONCONTROLLER_H
#define EVENTDETECTIONCONTROLLER_H

#include "plotconsumer.h"
#include <QPointF>
#include "application_status.h"
#include "plotmessage.h"
#include "eventdetectionconsumer.h"
#include "centralwidgetcontroller.h"

class EventDetectionController : public CentralWidgetController {
    Q_OBJECT

public:
    EventDetectionController(ApplicationStatus* appStatus, DeviceDataProducer* producer);
    ~EventDetectionController();

    void stop() override;
    void start() override;
    PlotConsumer* getConsumer() override;

private:
    EventDetectionConsumer* consumer = nullptr;
    std::vector <Curve*> currentCurves;
    EventDetectionMessage message;

    void detachCurves() override;
    void attachCurves() override;

signals:
    void durationChanged(Measurement_t duration);

public slots:
    void onRangeUpdated(commlib::RangedMeasurement_t newRange) override;
    void onCurrentColorsChanged(QVector <QColor> colors) override;
    void onCurrentColorChanged(int channelIdx, QColor color) override;
    void onBackgroundColorChanged(QColor color) override;
    void onReplot() override;
    void onExpandTrace(bool flag);
    void onSetPlotData(PlotMessage plotMessage) override;
};

#endif // EVENTDETECTIONCONTROLLER_H
#pragma once
