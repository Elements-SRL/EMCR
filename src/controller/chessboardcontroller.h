#ifndef CHESSBOARDCONTROLLER_H
#define CHESSBOARDCONTROLLER_H

#include <QObject>

#include "chessboarddockwidget.h"
#include "mainwindow.h"
#include "messagedispatcher.h"
#include <QMouseEvent>

class ChessboardController : public QObject {
    Q_OBJECT

public:
    ChessboardController(MessageDispatcher * msgDisp, MainWindow * mainWindow);
    ~ChessboardController();

    void clearCurves();
    void clearPlots();
    void channelsTurnedOnOff(bool flag);
    void stimuliTurnedOnOff(bool flag);
    void docTurnedOnOff(bool flag);
    void tracesExpandedOnOff(bool flag);

public slots:
    void onChannelsTurnedOnOff(bool flag);
    void onStimuliTurnedOnOff(bool flag);
    void onDocTurnedOnOff(bool flag);
    void onTracesExpandedOnOff(bool flag);
    void onRangeUpdated(RangedMeasurement_t newRange, QwtPlot::Axis axisIdx = QwtPlot::yLeft);
    void onDurationUpdated(Measurement_t duration);
    void onSetGapFreePlotData(double * timeValues, QVector <double *> * voltageValues, QVector <double *> * currentValues, int dataSize);
    void onReplot();
    void onSelectedPlotsUpdated();
    void onConsumerUpdated(bool flag);

private:
    MessageDispatcher * msgDisp = nullptr;
    MainWindow * mainWindow = nullptr;
    ChessboardDockWidget * chessboard = nullptr;

    QVector <StampPlot *> plots;
    QVector <Curve *> currentCurves;

    std::vector <ChannelModel *> channels;

    int voltageChannelsNum;
    int currentChannelsNum;

signals:
    void sigAllChannelsClicked(bool newChannelState);
    void sigOneBoardClicked(uint16_t changedBoardIndex, bool newChannelState);
    void sigOneRowClicked(uint16_t changedRowIndex, bool newChannelState);
    void sigSingleChannelClicked(uint16_t changedChannelIndex, QMouseEvent * event);
};

#endif // CHESSBOARDCONTROLLER_H
