#ifndef GAPFREECONTROLLER_H
#define GAPFREECONTROLLER_H

#include "plotmessage.h"
#include "centralwidgetcontroller.h"
#include "gapfreewidget.h"
#include "abfdatawriterconsumer.h"
#include "devicecontroller.h"
#include <memory>
#include "plotcontroller.h"

class GapFreeController : public CentralWidgetController {
    Q_OBJECT

public:
    GapFreeController(ApplicationStatus*, DeviceDataProducer*, RangedMeasurement_t, BigPlotWidget*, MainWindow*, DeviceController*);
    ~GapFreeController();
    void stop() override;
    void start() override;
    PlotConsumer* getConsumer() override;
    std::vector <DeviceDataConsumer*> getConsumers() override;

private:
    std::unique_ptr<PlotController> pc;
    GapFreePlotConsumer * consumer = nullptr;
    GapFreeWidget* gapFreeWidget = nullptr;
    AbfDataWriterConsumer* abfDataWriterConsumer = nullptr;
    std::vector <Curve*> currentCurves;
    std::vector <Curve*> voltageCurves;
    void detachCurves(const std::vector <uint16_t> &channelIndexes) override;
    void attachCurves(const std::vector <uint16_t>& channelIndexes) override;

public slots:
    void onRangeUpdated(commlib::RangedMeasurement_t newRange) override;
    void onCurrentColorsChanged(QVector <QColor> colors) override;
    void onCurrentColorChanged(int channelIdx, QColor color) override;
    void onBackgroundColorChanged(QColor color) override;
    void onReplot() override;
    void onExpandTrace(bool flag) override;
    void onSetPlotData(PlotMessage plotMessage) override;
    void onRecordingRequest(bool flag);
    void onRecordingExecution(bool flag);
    void onProtocolStarted(unsigned int protocolId, ProtocolWidget * protocol) override;

private slots:
    void onStartRecording();
    void onStopRecording();
    void onAxesChanged();

signals:
    void sigDurationChanged(Measurement_t duration);
    void sigStartRecording();
    void sigStopRecording();
};

#endif // GAPFREECONTROLLER_H
