#ifndef CENTRALWIDGETCONTROLLER_H
#define CENTRALWIDGETCONTROLLER_H

#include "controllerwithconsumer.h"
#include "bigplotwidget.h"
#include "plotconsumer.h"
#include "application_status.h"
#include "plotmessage.h"
#include "plotconsumer.h"
#include "application_status.h"

class CentralWidgetController : public ControllerWithConsumer {
    Q_OBJECT

public:
    CentralWidgetController(ApplicationStatus* appStatus, DeviceDataProducer* producer, BigPlotWidget* bigPlotWidget);

    //void clean();
    virtual void stop() = 0;
    virtual void start() = 0;
    virtual PlotConsumer* getConsumer() = 0;

private:
    virtual void detachCurves(const std::vector <uint16_t>& channelIndexes) = 0;
    virtual void attachCurves(const std::vector <uint16_t>& channelIndexes) = 0;

public slots:
    virtual void onRangeUpdated(RangedMeasurement_t newRange) = 0;
    virtual void onCurrentColorsChanged(QVector <QColor> colors) = 0;
    virtual void onCurrentColorChanged(int channelIdx, QColor color) = 0;
    virtual void onBackgroundColorChanged(QColor color) = 0;
    virtual void onReplot() = 0;
    virtual void onExpandTrace(bool flag) = 0;
    virtual void onSetPlotData(PlotMessage plotMessage) = 0;

protected:
    BigPlotWidget* bigPlotWidget;
    ApplicationStatus* appStatus;
    DeviceDataProducer* producer;

    int currentChannelsNum;
    int voltageChannelsNum;
    bool isAtLeastOneChannelExpanded();
};

#endif // CENTRALWIDGETCONTROLLER_H
