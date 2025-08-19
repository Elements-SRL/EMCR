#ifndef SPECTRUMCONTROLLER_H
#define SPECTRUMCONTROLLER_H

#include "bigplotwidget.h"
#include "mainwindow.h"
#include "spectrumconsumer.h"
#include "application_status.h"
#include "plotmessage.h"
#include "centralwidgetcontroller.h"
#include "spectrumwidget.h"
#include <memory>
#include "plotcontroller.h"

class SpectrumController : public CentralWidgetController {
    Q_OBJECT

public:
    SpectrumController(ApplicationStatus * appStatus, DeviceDataProducer * producer, RangedMeasurement_t defaultPlotBandwidth, BigPlotWidget * bigPlotWidget, MainWindow * mainWindow);
    ~SpectrumController();

    void stop() override;
    void start() override;
    PlotConsumer * getConsumer() override;
    std::vector <DeviceDataConsumer*> getConsumers() override;
    SpectrumWidget * getSpectrumWidget();

private:
    std::unique_ptr<PlotController> pc;
    //std::unique_ptr<BigPlotViewController> bpvc;
    SpectrumConsumer * consumer = nullptr;
    std::vector <Curve *> psdCurves;
    std::vector <Curve *> irmsCurves;
    SpectrumWidget * spectrumWidget = nullptr;
    MainWindow * mainWindow = nullptr;
    SpectrumMessage message;
    bool plotInitializedFlag = false;

    void detachCurves(const std::vector <uint16_t> &channelIndexes) override;
    void attachCurves(const std::vector <uint16_t> &channelIndexes) override;
    void saveToCSV(const QString& filePath, const SpectrumMessage& data);

public slots:
    void onRangeUpdated(commlib::RangedMeasurement_t newRange) override;
    void onCurrentColorsChanged(QVector <QColor> colors) override;
    void onCurrentColorChanged(int channelIdx, QColor color) override;
    void onBackgroundColorChanged(QColor color) override;
    void onReplot() override;
    void onExpandTrace(bool flag) override;
    void onSetPlotData(PlotMessage plotMessage) override;
    void onExportSpectrum();

signals:
    void sigLabelsOverride(std::map<QwtPlot::Axis, std::string> newLabels);
};

#endif // SPECTRUMCONTROLLER_H
