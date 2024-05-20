#ifndef EVENTDETECTIONCONTROLLER_H
#define EVENTDETECTIONCONTROLLER_H

#include "plotconsumer.h"
#include <QPointF>
#include "application_status.h"
#include "plotmessage.h"
#include "eventdetectionconsumer.h"
#include "centralwidgetcontroller.h"
#include "eventdetectionwidget.h"
#include "H5Cpp.h"

constexpr int RANK = 1;
constexpr int CHUNK_SIZE = 10000;

class EventDetectionController : public CentralWidgetController {
    Q_OBJECT

public:
    EventDetectionController(ApplicationStatus* appStatus, DeviceDataProducer* producer, BigPlotWidget* bpw);
    ~EventDetectionController();

    void stop() override;
    void start() override;
    PlotConsumer* getConsumer() override;

private:
    EventDetectionConsumer* consumer = nullptr;
    std::map<uint32_t, std::pair<std::vector<Event>, Baseline>> eventsInfo;
    std::map<uint32_t, std::vector<QwtPlotCurve*>> eventCurves;
    EventDetectionMessage message;
    EventDetectionWidget* widget = nullptr;
    void detachCurves() override;
    void attachCurves() override;
    H5::Group group;
    uint64_t eventCounter = 0;
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
