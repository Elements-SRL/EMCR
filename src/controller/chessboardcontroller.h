#ifndef CHESSBOARDCONTROLLER_H
#define CHESSBOARDCONTROLLER_H

#include <QObject>
#include "chessboarddockwidget.h"
#include "mainwindow.h"
#include "messagedispatcher.h"
#include <QMouseEvent>
#include "plotconsumer.h"
#include "application_status.h"
#include "plotmessage.h"

class ChessboardController : public QObject {
    Q_OBJECT

public:
    ChessboardController(ApplicationStatus * appStatus, PlotConsumer * plotConsumer, Measurement_t defaultDuration, MainWindow * mainWindow);
    ~ChessboardController();

    void clearCurves();
    void clearPlots();
    void channelsTurnedOnOff(bool flag);
    void calibrationResistorsTurnedOnOff(bool flag);
    void stimuliTurnedOnOff(bool flag);
    void offsetRecalibrationTurnedOnOff(bool flag);
    void ljcTurnedOnOff(bool flag);
    void tracesExpandedOnOff(bool flag);
    PlotConsumer * getPlotConsumer();
    void onBoardMappingLoaded();

public slots:
    void onChannelsTurnedOnOff(bool flag);
    void onCalibrationResistorsTurnedOnOff(bool flag);
    void onStimuliTurnedOnOff(bool flag);
    void onOffsetRecalibrationTurnedOnOff(bool flag);
    void onLjcTurnedOnOff(bool flag);
    void onTracesExpandedOnOff(bool flag);
    void onRangeUpdated(RangedMeasurement_t newRange);
    void onDurationUpdated(Measurement_t duration);
    void onSetPlotData(PlotMessage plotMessage);
    void onReplot();
    void onSelectedPlotsUpdated();

    void onCurrentColorsChanged(QVector <QColor> colors);
    void onCurrentColorChanged(int channelIdx, QColor color);
//    void onBackgroundColorChanged(QColor color);

private:
    ApplicationStatus * appStatus;
    MainWindow * mainWindow = nullptr;
    ChessboardDockWidget * chessboard = nullptr;
    PlotConsumer * stampPlotConsumer = nullptr;

    QVector <StampPlot *> plots;
    QVector <Curve *> currentCurves;

    std::vector <ChannelModel *> channels;

    int voltageChannelsNum;
    int currentChannelsNum;
    void onSetConsumerStatus(bool status);
    void updateChessboard();

signals:
    void sigAllChannelsClicked(bool newChannelState);
    void sigOneBoardClicked(uint16_t changedBoardIndex, bool newChannelState);
    void sigOneRowClicked(uint16_t changedRowIndex, bool newChannelState);
    void sigSingleChannelClicked(uint16_t changedChannelIndex, QMouseEvent * event);
};

#endif // CHESSBOARDCONTROLLER_H
