#ifndef PLOTDETAILCONTROLLER_H
#define PLOTDETAILCONTROLLER_H

#include <QObject>
#include "plotdetail.h"
#include "multiplechannelcontroller.h"
#include "chessboardcontroller.h"

class PlotDetailController: public ControllerWithConsumer {
    Q_OBJECT

private:
    std::vector<PlotDetailModel *> pdms;
    std::map<uint16_t, PlotDetail *> pds;
    MainWindow* mainWindow;
    void manageCreation();
    void manageDeletion();
    void manageComsuner();
    GapFreePlotConsumer * consumer = nullptr;

protected:
    std::vector <DeviceDataConsumer*> getConsumers() override;
    void onCurrentRangeChanged() override;

public:
    PlotDetailController(ApplicationStatus * appStatus, Measurement_t defaultPlotDuration, MainWindow* mainWindow, MultipleChannelController * mcc, ChessboardController * cc, DeviceDataProducer* p);
    ~PlotDetailController();

public slots:
    void onPlotDetailAction(bool);
    void onCurrentColorsChanged(QVector <QColor> colors);
    void onCurrentColorChanged(int channelIdx, QColor color);
    void onBackgroundColorChanged(QColor color);

private slots:
    // void plotDetailAction(bool);
    void onSetPlotData(PlotMessage plotMessage);
    void onReplot();
    void onHandleEndOfPlot();

signals:
    void sigRemoveState(std::vector<uint16_t>);
    void sigAddState(std::vector<uint16_t>);
};

#endif // PLOTDETAILCONTROLLER_H
