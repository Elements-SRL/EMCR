#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QWidget>
#include <QPushButton>

#include "modeldevice.h"
#include "stampplot.h"
#include "curve.h"
#include "myleftrightmousepushbutton.h"

class Chessboard : public QWidget {
    Q_OBJECT

public:
    Chessboard(ModelDevice * mDev, QWidget * parent = nullptr);

    void clearCurves();

public slots:
    void onRangeUpdated(RangedMeasurement_t newRange, QwtPlot::Axis axisIdx = QwtPlot::yLeft);
    void onDurationUpdated(Measurement_t duration);
    void onSetGapFreePlotData(double * timeValues, QVector <double *> * voltageValues, QVector <double *> * currentValues, int dataSize, int channelsToPlotNumber);
    void onReplot();

private:
    MyLeftRightMousePushButton * allChannelsSelector = nullptr;
    QVector <MyLeftRightMousePushButton *> boardSelectors;
    QVector <MyLeftRightMousePushButton *> rowSelectors;
    QVector <StampPlot *> plots;
    QVector <Curve *> currentCurves;

    int voltageChannelsNum;
    int currentChannelsNum;

signals:
    void allChannelsClicked(bool newChannelState);
    void oneBoardClicked(uint16_t changedBoardIndex, bool newChannelState);
    void oneRowClicked(uint16_t changedRowIndex, bool newChannelState);
    void singleChannelClicked(uint16_t changedChannelIndex, bool newChannelState);
};

#endif // CHESSBOARD_H
