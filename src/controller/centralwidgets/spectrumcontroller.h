#ifndef SPECTRUMCONTROLLER_H
#define SPECTRUMCONTROLLER_H

#include "bigplotwidget.h"
#include "mainwindow.h"
#include "messagedispatcher.h"
#include "bigplotmodel.h"
#include "spectrumconsumer.h"
#include "application_status.h"
#include "plotmessage.h"
#include "centralwidgetcontroller.h"
#include "bigplotcontroller.h"

class SpectrumController : public CentralWidgetController {
    Q_OBJECT

public:
    SpectrumController(ApplicationStatus * appStatus, DeviceDataProducer * producer, Measurement_t defaultPlotBandwidth, BigPlotWidget * bigPlotWidget, BigPlotController * bigPlotController, MainWindow * mainWindow);
    ~SpectrumController();

    void stop() override;
    void start() override;
    PlotConsumer * getConsumer() override;
    SpectrumWidget * getSpectrumWidget();

private:
    BigPlotModel * model = nullptr;
    SpectrumConsumer * consumer = nullptr;
    BigPlot * plot = nullptr;
    std::vector <Curve *> currentCurves;
    SpectrumWidget * spectrumWidget = nullptr;
    MainWindow * mainWindow = nullptr;
    void detachCurves(const std::vector <uint16_t> &channelIndexes) override;
    void attachCurves(const std::vector <uint16_t> &channelIndexes) override;

public slots:
    void onRangeUpdated(commlib::RangedMeasurement_t newRange) override;
    void onCurrentColorsChanged(QVector <QColor> colors) override;
    void onCurrentColorChanged(int channelIdx, QColor color) override;
    void onBackgroundColorChanged(QColor color) override;
    void onReplot() override;
    void onExpandTrace(bool flag) override;
    void onSetPlotData(PlotMessage plotMessage) override;

signals:
    void integrationWindowChanged(Measurement_t integrationWindow);
};

#endif // SPECTRUMCONTROLLER_H
