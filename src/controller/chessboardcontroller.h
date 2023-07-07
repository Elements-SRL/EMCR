#ifndef CHESSBOARDCONTROLLER_H
#define CHESSBOARDCONTROLLER_H

#include <QObject>

#include "chessboarddockwidget.h"
#include "mainwindow.h"
#include "messagedispatcher.h"

class ChessboardController : public QObject {
    Q_OBJECT

public:
    ChessboardController(MessageDispatcher * msgDisp, MainWindow * mainWindow);

private:
    MessageDispatcher * msgDisp = nullptr;
    MainWindow * mainWindow = nullptr;
    ChessboardDockWidget * chessboard = nullptr;

signals:
    void sigAllChannelsClicked(bool newChannelState);
    void sigOneBoardClicked(uint16_t changedBoardIndex, bool newChannelState);
    void sigOneRowClicked(uint16_t changedRowIndex, bool newChannelState);
    void sigSingleChannelClicked(uint16_t changedChannelIndex, bool newChannelState);
};

#endif // CHESSBOARDCONTROLLER_H
