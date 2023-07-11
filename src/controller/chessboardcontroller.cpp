#include "chessboardcontroller.h"

ChessboardController::ChessboardController(MessageDispatcher * msgDisp, MainWindow * mainWindow) :
    msgDisp(msgDisp),
    mainWindow(mainWindow) {

    chessboard = new ChessboardDockWidget(msgDisp);
    stamplPlotController = new StampPlotController(msgDisp, chessboard);

    connect(chessboard, &ChessboardDockWidget::sigAllChannelsClicked,   this,       &ChessboardController::sigAllChannelsClicked);
    connect(chessboard, &ChessboardDockWidget::sigOneBoardClicked,      this,       &ChessboardController::sigOneBoardClicked);
    connect(chessboard, &ChessboardDockWidget::sigOneRowClicked,        this,       &ChessboardController::sigOneRowClicked);
    connect(chessboard, &ChessboardDockWidget::sigSingleChannelClicked, this,       &ChessboardController::sigSingleChannelClicked);

    connect(chessboard, &ChessboardDockWidget::sigAllChannelsClicked,   stamplPlotController, &StampPlotController::onSelectedPlotsUdpated);
    connect(chessboard, &ChessboardDockWidget::sigOneBoardClicked,      stamplPlotController, &StampPlotController::onSelectedPlotsUdpated);
    connect(chessboard, &ChessboardDockWidget::sigOneRowClicked,        stamplPlotController, &StampPlotController::onSelectedPlotsUdpated);
    connect(chessboard, &ChessboardDockWidget::sigSingleChannelClicked, stamplPlotController, &StampPlotController::onSelectedPlotsUdpated);

    mainWindow->setChessboardDw(chessboard);
}

StampPlotController * ChessboardController::getStampPlotController() {
    return stamplPlotController;
}

void ChessboardController::onChannelsTurnedOnOff(bool flag) {
    stamplPlotController->channelsTurnedOnOff(flag);
}

void ChessboardController::onStimuliTurnedOnOff(bool flag) {
    stamplPlotController->stimuliTurnedOnOff(flag);
}

void ChessboardController::onDocTurnedOnOff(bool flag) {
    stamplPlotController->docTurnedOnOff(flag);
}
