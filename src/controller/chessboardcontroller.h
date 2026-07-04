#ifndef CHESSBOARDCONTROLLER_H
#define CHESSBOARDCONTROLLER_H

#include "controllerwithconsumer.h"
#include "chessboarddockwidget.h"
#include "mainwindow.h"
#include <QMouseEvent>
#include "plotconsumer.h"
#include "application_status.h"
#include "plotmessage.h"
#include "singlechannelcontroller.h"
#include "multiplechannelcontroller.h"
#include "measurementoverviewcontroller.h"

class ChessboardController : public ControllerWithConsumer {
    Q_OBJECT

public:
    ChessboardController(ApplicationStatus * appStatus, DeviceDataProducer * dataProducer, Measurement_t defaultDuration, MainWindow * mainWindow);
    ~ChessboardController();

    void clearCurves();
    void clearPlots();
    void calibrationResistorsTurnedOnOff(bool flag);
    void offsetRecalibrationTurnedOnOff(bool flag);
    void ljcTurnedOnOff(bool flag);
    PlotConsumer * getPlotConsumer();
    virtual std::vector <DeviceDataConsumer *> getConsumers() override;
    void onBoardMappingLoaded();
    void clickBehaviour(bool newState);
    void connectSingleChannelController(SingleChannelController*);
    void connectMultipleChannelController(MultipleChannelController*);
    void connectMeasurementOverviewController(MeasurementOverviewController*);

public slots:
    void onSingleChannelClicked(uint16_t chIdx, QMouseEvent *event);
    void onOneBoardClicked(uint16_t brdIdx, bool newState);
    void onOneRowClicked(uint16_t rowIdx, bool newState);
    void onAllChannelsClicked(bool newState);

    void onChannelsTurnedOnOff(bool flag);
    void onChannelsTurnedOnOffEx(bool flag);
    void onCalibrationResistorsTurnedOnOff(bool flag);
    void onStimuliTurnedOnOff(bool flag);
    void onStimuliTurnedOnOffEx(bool flag);
    void onOffsetRecalibrationTurnedOnOff(bool flag);
    void onLjcTurnedOnOff(bool flag);
    void onTracesExpandedOnOff(bool flag);
    void onTracesExpandedOnOffEx(bool flag);
    // Plot detail
    void onPlotDetailOnOff(bool flag);
    void onPlotDetailOnOffEx(bool flag);
    void onPlotDetailCreation(std::vector<uint16_t>);
    void onPlotDetailDeletion(std::vector<uint16_t>);

    void onRangeUpdated(std::vector <RangedMeasurement_t> newRange);
    void onDurationUpdated(Measurement_t duration);
    void onSetPlotData(PlotMessage plotMessage);
    void onReplot();
    void onSelectedPlotsUpdated();

    void onCurrentColorsChanged(QVector <QColor> colors);
    void onCurrentColorChanged(int channelIdx, QColor color);
//    void onBackgroundColorChanged(QColor color);

    void onSyncFaults(std::vector <bool> syncFaultFlags);

private:
    MainWindow * mainWindow = nullptr;
    ChessboardDockWidget * chessboard = nullptr;
    GapFreePlotConsumer * stampPlotConsumer = nullptr;

    QVector <StampPlot *> plots;
    QVector <Curve *> currentCurves;

    int voltageChannelsNum;
    int currentChannelsNum;
    void onSetConsumerStatus(bool status);
    void updateChessboard();
    void setSelectedStatus(std::vector<int>, bool);

signals:
    void sigAllChannelsClicked(bool newChannelState);
    void sigOneBoardClicked(uint16_t changedBoardIndex, bool newChannelState);
    void sigOneRowClicked(uint16_t changedRowIndex, bool newChannelState);
    void sigSingleChannelClicked(uint16_t changedChannelIndex, QMouseEvent * event);
    void sigInvertSelectionClicked();
};

#endif // CHESSBOARDCONTROLLER_H
