#ifndef CHESSBOARDDOCKWIDGET_H
#define CHESSBOARDDOCKWIDGET_H

#include <QDockWidget>
#include <QPushButton>

#include "stampplot.h"
#include "curve.h"
#include "myleftrightmousepushbutton.h"
#include "channeloverviewwidget.h"
#include "messagedispatcher.h"

class ChessboardDockWidget : public QDockWidget {
    Q_OBJECT

public:
    ChessboardDockWidget(MessageDispatcher * msgDisp, QWidget * parent = nullptr);

    void addPlot(StampPlot * plot, int channelIdx);
    int getIdealPlotWidth();
    int getIdealPlotHeight();

private:
    MessageDispatcher * msgDisp = nullptr;

    QGridLayout * mainGl = nullptr;

    MyLeftRightMousePushButton * allChannelsSelector = nullptr;
    QVector <MyLeftRightMousePushButton *> boardSelectors;
    QVector <MyLeftRightMousePushButton *> rowSelectors;

    int voltageChannelsNum;
    int currentChannelsNum;
    int boardsNum;
    int channelsPerBoard;
    int idealPlotHeight;
    int idealPlotWidth;

signals:
    void sigAllChannelsClicked(bool newChannelState);
    void sigOneBoardClicked(uint16_t changedBoardIndex, bool newChannelState);
    void sigOneRowClicked(uint16_t changedRowIndex, bool newChannelState);
    void sigSingleChannelClicked(uint16_t changedChannelIndex, bool newChannelState);
};

#endif // CHESSBOARDDOCKWIDGET_H
