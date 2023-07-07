#ifndef CHESSBOARDDOCKWIDGET_H
#define CHESSBOARDDOCKWIDGET_H

#include <QDockWidget>
#include <QPushButton>

#include "stampplot.h"
#include "curve.h"
#include "myleftrightmousepushbutton.h"
#include "channeloverviewwidget.h"
#include "livenoiseconsumer.h"
#include "messagedispatcher.h"

class ChessboardDockWidget : public QDockWidget {
    Q_OBJECT

public:
    ChessboardDockWidget(MessageDispatcher * msgDisp, QWidget * parent = nullptr);

    void clearCurves();

public slots:
    void onRangeUpdated(RangedMeasurement_t newRange, QwtPlot::Axis axisIdx = QwtPlot::yLeft);
    void onDurationUpdated(Measurement_t duration);
    void onSetGapFreePlotData(double * timeValues, QVector <double *> * voltageValues, QVector <double *> * currentValues, int dataSize, int channelsToPlotNumber);
    void onReplot();
    void onSelectedPlotsUdpated();
    void onNoiseValueUpdated(LiveNoiseConsumer::Result_t result);

private:
    MessageDispatcher * msgDisp = nullptr;

    MyLeftRightMousePushButton * allChannelsSelector = nullptr;
    QVector <MyLeftRightMousePushButton *> boardSelectors;
    QVector <MyLeftRightMousePushButton *> rowSelectors;
    QVector <StampPlot *> plots;
    QVector <Curve *> currentCurves;
    QVector <ChannelOverviewWidget *> overviewWidgets;

    int voltageChannelsNum;
    int currentChannelsNum;

signals:
    void sigAllChannelsClicked(bool newChannelState);
    void sigOneBoardClicked(uint16_t changedBoardIndex, bool newChannelState);
    void sigOneRowClicked(uint16_t changedRowIndex, bool newChannelState);
    void sigSingleChannelClicked(uint16_t changedChannelIndex, bool newChannelState);
    void sigExportLiveNoiseEstimates();
};

#endif // CHESSBOARDDOCKWIDGET_H
