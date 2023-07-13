#ifndef STAMPPLOTCONTROLLER_H
#define STAMPPLOTCONTROLLER_H

#include <QObject>

#include "chessboarddockwidget.h"
#include "stampplot.h"
#include "messagedispatcher.h"

class StampPlotController : public QObject {
    Q_OBJECT

public:
    StampPlotController(MessageDispatcher * msgDisp, ChessboardDockWidget * chessboardDw);

    void clearCurves();
    void channelsTurnedOnOff(bool flag);
    void stimuliTurnedOnOff(bool flag);
    void docTurnedOnOff(bool flag);
    void tracesExpandedOnOff(bool flag);

public slots:
    void onRangeUpdated(RangedMeasurement_t newRange, QwtPlot::Axis axisIdx = QwtPlot::yLeft);
    void onDurationUpdated(Measurement_t duration);
    void onSetGapFreePlotData(double * timeValues, QVector <double *> * voltageValues, QVector <double *> * currentValues, int dataSize, int channelsToPlotNumber);
    void onReplot();
    void onSelectedPlotsUdpated();

private:
    MessageDispatcher * msgDisp = nullptr;
    MessageDispatcher * chessboardDw = nullptr;
    QVector <StampPlot *> plots;
    QVector <Curve *> currentCurves;

    std::vector <ChannelModel *> channels;

    int voltageChannelsNum;
    int currentChannelsNum;
};

#endif // STAMPPLOTCONTROLLER_H
