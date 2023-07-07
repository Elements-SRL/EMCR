#include "chessboardcontroller.h"

ChessboardController::ChessboardController(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp),
    mainWindow(mainWindow) {

    chessboard = new ChessboardDockWidget(msgDisp);

    connect(chessboard, &ChessboardDockWidget::sigAllChannelsClicked,   this,       &ChessboardController::sigAllChannelsClicked);
    connect(chessboard, &ChessboardDockWidget::sigOneBoardClicked,      this,       &ChessboardController::sigOneBoardClicked);
    connect(chessboard, &ChessboardDockWidget::sigOneRowClicked,        this,       &ChessboardController::sigOneRowClicked);
    connect(chessboard, &ChessboardDockWidget::sigSingleChannelClicked, this,       &ChessboardController::sigSingleChannelClicked);

    connect(chessboard, &ChessboardDockWidget::sigAllChannelsClicked,   chessboard, &ChessboardDockWidget::onSelectedPlotsUdpated);
    connect(chessboard, &ChessboardDockWidget::sigOneBoardClicked,      chessboard, &ChessboardDockWidget::onSelectedPlotsUdpated);
    connect(chessboard, &ChessboardDockWidget::sigOneRowClicked,        chessboard, &ChessboardDockWidget::onSelectedPlotsUdpated);
    connect(chessboard, &ChessboardDockWidget::sigSingleChannelClicked, chessboard, &ChessboardDockWidget::onSelectedPlotsUdpated);

    mainWindow->setChessboardDw(chessboard);
}
