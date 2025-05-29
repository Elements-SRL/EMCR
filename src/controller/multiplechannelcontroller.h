#ifndef MULTIPLECHANNELCONTROLLER_H
#define MULTIPLECHANNELCONTROLLER_H

#include <QObject>

#include "multiplechannelcontroldockwidget.h"
#include "multiplechannelmodel.h"
#include "mainwindow.h"
#include "messagedispatcher.h"
#include "offsetcorrectioncontroller.h"

class MultipleChannelController : public QObject {
    Q_OBJECT

public:
    MultipleChannelController(ApplicationStatus * appStatus, MainWindow * mainWindow);
    ~MultipleChannelController();
    void addRemoveFromBigPlot(bool flag);
    void addRemoveFromBigPlotEx(bool flag);

public slots:
    void onChannelsSelected();

private:
    void turnSelectedChannelsOnOff(bool flag);
    void turnSelectedChannelsOnOffEx(bool flag);
    void turnSelectedCalibrationResistorsOnOff(bool flag);
    void turnSelectedStimuliOnOff(bool flag);
    void turnSelectedStimuliOnOffEx(bool flag);
    void zap(Measurement_t duration);
    void offsetCorrection(OffsetCorrectionController::OffsetCorrectionCheck_t step);
    void turnSelectedOffsetRecalibrationOnOff(bool flag);
    void resetOffsetRecalibration();
    void turnSelectedLjcOnOff(bool flag);
    void resetLj();

    ApplicationStatus * appStatus = nullptr;
    MessageDispatcher * msgDisp = nullptr;
    MainWindow * mainWindow = nullptr;
    MultipleChannelControlDockWidget * multipleChannelControlsDw = nullptr;
    MultipleChannelModel * model = nullptr;
    OffsetCorrectionController * offsetCorrectionController = nullptr;
    std::vector <uint16_t> allChannels;
private slots:
    void addRemovePlotDetail(bool flag);

signals:
    void sigChannelsTurnedOnOff(bool flag);
    void sigChannelsTurnedOnOffEx(bool flag);
    void sigCalibrationResistorsTurnedOnOff(bool flag);
    void sigStimuliTurnedOnOff(bool flag);
    void sigStimuliTurnedOnOffEx(bool flag);
    void sigOffsetRecalibrationTurnedOnOff(bool flag);
    void sigOffsetRecalibrationResetted();
    void sigLjcTurnedOnOff(bool flag);
    void sigLjResetted();
    void sigAddRemoveFromBigPlot(bool flag);
    void sigAddRemoveFromBigPlotEx(bool flag);
    // Plot detail
    void sigAddRemovePlotDetail(bool flag);
};

#endif // MULTIPLECHANNELCONTROLLER_H
