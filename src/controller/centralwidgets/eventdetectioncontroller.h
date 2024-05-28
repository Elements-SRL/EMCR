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
#include "eventpacket.h"
#include "binner.h"

constexpr int RANK = 1;
constexpr int CHUNK_SIZE = 10000;
constexpr double STD_MULTIPLIER = 3.0;

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
    std::map<uint32_t, EventPacket> eventPackets;
    std::map<uint32_t, QwtPlotCurve*> eventCurves;
    EventDetectionMessage message;
    EventDetectionWidget* widget = nullptr;
    void detachCurves(const std::vector <uint16_t>& channelIndexes) override;
    void attachCurves(const std::vector <uint16_t>& channelIndexes) override;
    uint64_t eventCounter = 0;
    H5::Group parentGroup;

    uint32_t totalEvents = 0;
    uint32_t eventsPerSec = 0;
    
    double durationAccumulator = 0;
    double amplitudeAccumulator = 0;

    Binner * durationBinner;
    Binner* amplitudeBinner;

    double minDurationInSeconds;
    double maxDurationInSeconds;
    uint32_t durationBins = 200;

    double minAmplitude = 0.0;
    double maxAmplitude;
    uint32_t amplitudeBins = 200;
    /// <summary>
    /// HDF5 staff to manage baseline dataset
    /// </summary>
    H5::DataSet baselineDataset;

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
