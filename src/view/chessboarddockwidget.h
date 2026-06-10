#ifndef CHESSBOARDDOCKWIDGET_H
#define CHESSBOARDDOCKWIDGET_H

#define CHB_SCREEN_PERCENTAGE_HEIGHT 0.75
#define CHB_SCREEN_PERCENTAGE_WIDTH 0.6
#define CHB_PLOT_ASPECT_RATIO 0.75

#include <QDockWidget>
#include <QPushButton>

#include "stampplot.h"
#include "curve.h"
#include "leftrightmousepushbutton.h"
#include "channeloverviewwidget.h"
#include "messagedispatcher.h"
#include <QMouseEvent>
#include "application_status.h"

class ChessboardDockWidget : public QDockWidget {
    Q_OBJECT

public:
    ChessboardDockWidget(ApplicationStatus * appStatus, QWidget * parent = nullptr);

    void addPlot(StampPlot * plot, int channelIdx);
    int getIdealPlotWidth();
    int getIdealPlotHeight();
    void updateBoardMappings(std::set <int> visibleBoards);
    void updateSelectedCounter(int selected, int total);

private:
    QGridLayout * mainGl = nullptr;

    LeftRightMousePushButton * allChannelsSelector = nullptr;
    QVector <LeftRightMousePushButton *> boardSelectors;
    QVector <LeftRightMousePushButton *> rowSelectors;
    QLabel *selectedChannelsTxt = nullptr;
    QLabel *totalChannelsTxt = nullptr;

    int voltageChannelsNum;
    int currentChannelsNum;
    int boardsNum;
    int channelsPerBoard;

signals:
    void sigAllChannelsClicked(bool newChannelState);
    void sigOneBoardClicked(uint16_t changedBoardIndex, bool newChannelState);
    void sigOneRowClicked(uint16_t changedRowIndex, bool newChannelState);
    void sigSingleChannelClicked(uint16_t changedChannelIndex, QMouseEvent * event);
    void sigUpdateConsumer(bool consume);
    void sigInvertSelectionClicked();
};

#endif // CHESSBOARDDOCKWIDGET_H
