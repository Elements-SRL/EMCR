#ifndef IVGRAPHCONTROLLER_H
#define IVGRAPHCONTROLLER_H

#include "bigplotwidget.h"
#include "mainwindow.h"
#include "plotconsumer.h"
#include <QPointF>
#include "application_status.h"
#include "plotmessage.h"
#include "ivgraphconsumer.h"
#include "ivgraphwidget.h"
#include "centralwidgetcontroller.h"
#include "bigplotviewcontroller.h"
#include <memory>

namespace e384cl = e384CommLib;

class IvGraphController : public CentralWidgetController {
    Q_OBJECT

public:
    IvGraphController(ApplicationStatus* appStatus, DeviceDataProducer* producer, BigPlotWidget* bigPlotWidget, MainWindow* mainWindow);
    ~IvGraphController();

    void stop() override;
    void start() override;
    PlotConsumer * getConsumer() override;
    std::vector <DeviceDataConsumer*> getConsumers() override;
    IvGraphWidget * getIvGraphWidget();

private:
    std::unique_ptr<BigPlotViewController> bpvc;
    // BigPlotModel * model = nullptr;
    IvGraphConsumer * consumer = nullptr;
    // BigPlot * plot = nullptr;
    std::vector <Curve *> currentCurves;
    std::vector <Curve *> fitCurves;
    IvGraphWidget * ivGraphWidget = nullptr;
    MainWindow * mainWindow = nullptr;
    IvMessage message;
    std::vector <QVector <double>> fitVoltageValues;
    std::vector <QVector <double>> fitCurrentValues;
    void saveToCSV(const QString& filePath, const IvMessage& data);

    void detachCurves(const std::vector <uint16_t>& channelIndexes) override;
    void attachCurves(const std::vector <uint16_t>& channelIndexes) override;

signals:
    void durationChanged(e384cl::Measurement_t duration);

public slots:
    void onRangeUpdated(e384cl::RangedMeasurement_t newRange) override;
    void onCurrentColorsChanged(QVector <QColor> colors) override;
    void onCurrentColorChanged(int channelIdx, QColor color) override;
    void onBackgroundColorChanged(QColor color) override;
    void onReplot() override;
    void onExpandTrace(bool flag) override;
    void onSetPlotData(PlotMessage plotMessage) override;

private slots:
    void onExportIvGraph();
    void onCalcMeanSquared();
    void onStartIvGraph();
    void onStopIvGraph();
};

#endif // IVGRAPHCONTROLLER_H
