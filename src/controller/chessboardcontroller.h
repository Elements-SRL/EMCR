#ifndef CHESSBOARDCONTROLLER_H
#define CHESSBOARDCONTROLLER_H

#include <QObject>
#include "chessboarddockwidget.h"
#include "mainwindow.h"
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
    void calibrationResistorsTurnedOnOff(bool flag);
    void offsetRecalibrationTurnedOnOff(bool flag);
    void ljcTurnedOnOff(bool flag);
    PlotConsumer * getPlotConsumer();
    void onBoardMappingLoaded();
    void clickBehaviour(bool newState);

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
    void setSelectedStatus(std::vector<int>, bool);

signals:
    void sigAllChannelsClicked(bool newChannelState);
    void sigOneBoardClicked(uint16_t changedBoardIndex, bool newChannelState);
    void sigOneRowClicked(uint16_t changedRowIndex, bool newChannelState);
    void sigSingleChannelClicked(uint16_t changedChannelIndex, QMouseEvent * event);
};

#endif // CHESSBOARDCONTROLLER_H
