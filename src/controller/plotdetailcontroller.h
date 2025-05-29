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
public slots:
    void plotDetailAction(bool);
private slots:
    // void plotDetailAction(bool);
    void onSetPlotData(PlotMessage plotMessage);
    void onReplot();
    void handleEndOfPlot();

signals:
    void removeState(std::vector<uint16_t>);
    void addState(std::vector<uint16_t>);
};

#endif // PLOTDETAILCONTROLLER_H
