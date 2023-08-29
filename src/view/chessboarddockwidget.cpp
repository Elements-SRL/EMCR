#include "chessboarddockwidget.h"

#include <QBoxLayout>
#include <QComboBox>
#include <QApplication>
#include "globaldefines.h"

ChessboardDockWidget::ChessboardDockWidget(MessageDispatcher * msgDisp, QWidget * parent) :
    QDockWidget(parent),
    msgDisp(msgDisp) {

    this->setObjectName("chessboard");

    msgDisp->getChannelNumberFeatures(voltageChannelsNum, currentChannelsNum);
    msgDisp->getBoardsNumberFeatures(boardsNum);
    channelsPerBoard = currentChannelsNum/boardsNum;

    QWidget * mainWg = new QWidget();
    mainWg->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->setWindowTitle("Channels overview");

    this->setWidget(mainWg);

    mainGl = new QGridLayout;
    mainGl->setMargin(0);
    mainGl->setSpacing(1);
    mainWg->setLayout(mainGl);

    idealPlotHeight = qMax(300/channelsPerBoard, STAMP_PLOT_SIZE);
    idealPlotWidth = qMax(450/boardsNum, STAMP_PLOT_SIZE);

    if (currentChannelsNum > 1) {
        allChannelsSelector = new MyLeftRightMousePushButton();
        allChannelsSelector->setText("ALL");
        if (boardsNum > 1) {
            allChannelsSelector->setFixedSize(STAMP_PLOT_SIZE, idealPlotHeight);

        } else {
            allChannelsSelector->setFixedSize(idealPlotWidth, STAMP_PLOT_SIZE);
        }
        connect(allChannelsSelector, &MyLeftRightMousePushButton::clicked, this, &ChessboardDockWidget::sigAllChannelsClicked);

        mainGl->addWidget(allChannelsSelector, 1, 0);
    }

    if (boardsNum > 1 && channelsPerBoard > 1) {
        boardSelectors.resize(boardsNum);
        for (int boardIdx = 0; boardIdx < boardsNum; boardIdx++) {
            MyLeftRightMousePushButton * btn = new MyLeftRightMousePushButton();
            btn->setText(QString("%1").arg(boardIdx+1));
            btn->setFixedSize(idealPlotWidth, STAMP_PLOT_SIZE);
            connect(btn, &MyLeftRightMousePushButton::clicked, this, [=] (bool selected) {
                emit sigOneBoardClicked(boardIdx, selected);
            });

            mainGl->addWidget(btn, 1, boardIdx+1);
            boardSelectors[boardIdx] = btn;
        }

        rowSelectors.resize(channelsPerBoard);
        for (int rowIdx = 0; rowIdx < channelsPerBoard; rowIdx++) {
            MyLeftRightMousePushButton * btn = new MyLeftRightMousePushButton();
            btn->setText(QString("%1").arg(rowIdx+1));
            btn->setFixedSize(STAMP_PLOT_SIZE, idealPlotHeight);
            connect(btn, &MyLeftRightMousePushButton::clicked, this, [=] (bool selected) {
                emit sigOneRowClicked(rowIdx, selected);
            });

            mainGl->addWidget(btn, rowIdx+2, 0);
            rowSelectors[rowIdx] = btn;
        }
    }
}

void ChessboardDockWidget::addPlot(StampPlot * plot, int channelIdx) {
    int rowIdx = channelIdx % channelsPerBoard;
    int boardIdx = channelIdx / channelsPerBoard;
    if (boardsNum > 1 && channelsPerBoard > 1) {
        mainGl->addWidget(plot, rowIdx+2, boardIdx+1);

    } else if (boardsNum > 1) {
        mainGl->addWidget(plot, rowIdx+1, boardIdx+1);

    } else if (channelsPerBoard > 1) {
        mainGl->addWidget(plot, rowIdx+2, boardIdx);

    } else {
        mainGl->addWidget(plot, rowIdx+1, boardIdx);
    }

    connect(plot, &StampPlot::clicked, [=] (QMouseEvent *event) {
        emit sigSingleChannelClicked(channelIdx, event);
    });
}

int ChessboardDockWidget::getIdealPlotWidth() {
    return idealPlotWidth;
}

int ChessboardDockWidget::getIdealPlotHeight() {
    return idealPlotHeight;
}
