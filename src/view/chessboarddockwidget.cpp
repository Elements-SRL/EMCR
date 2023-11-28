#include "chessboarddockwidget.h"

#include <QBoxLayout>
#include <QComboBox>
#include <QApplication>
#include "globaldefines.h"
#include <QScreen>


ChessboardDockWidget::ChessboardDockWidget(ApplicationStatus * appStatus, QWidget * parent) :
    QDockWidget(parent) {

    this->setObjectName("chessboard");

    this->voltageChannelsNum = appStatus->getVoltageChannelsNum();
    this->currentChannelsNum = appStatus->getCurrentChannelsNum();
    this->boardsNum = appStatus->getBoardsNum();
    channelsPerBoard = currentChannelsNum/boardsNum;

    QWidget * mainWg = new QWidget(parent);
    mainWg->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->setWindowTitle("Channels overview");
    this->setWidget(mainWg);

    mainGl = new QGridLayout;
    mainWg->setLayout(mainGl);

    mainGl->setMargin(0);
    mainGl->setSpacing(1);

    auto idealPlotHeight = getIdealPlotHeight();
    auto idealPlotWidth = getIdealPlotWidth();

    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();

    auto maxButtonHeight = qMin(idealPlotHeight, screenGeometry.height()/30);
    auto maxButtonWidth = qMin(idealPlotWidth, screenGeometry.width()/30);

    if (currentChannelsNum > 1) {
        allChannelsSelector = new MyLeftRightMousePushButton();
        allChannelsSelector->setText("ALL");
        allChannelsSelector->setFixedSize(maxButtonWidth, maxButtonHeight);
        connect(allChannelsSelector, &MyLeftRightMousePushButton::clicked, this, &ChessboardDockWidget::sigAllChannelsClicked);

        mainGl->addWidget(allChannelsSelector, 1, 0, Qt::AlignCenter);
    }

    if (boardsNum > 1 && channelsPerBoard > 1) {
        boardSelectors.resize(boardsNum);
        for (int boardIdx = 0; boardIdx < boardsNum; boardIdx++) {
            MyLeftRightMousePushButton * btn = new MyLeftRightMousePushButton();
            btn->setText(QString("%1").arg(boardIdx+1));
            btn->setFixedSize(maxButtonWidth, maxButtonHeight);
            connect(btn, &MyLeftRightMousePushButton::clicked, this, [=] (bool selected) {
                emit sigOneBoardClicked(boardIdx, selected);
            });

            mainGl->addWidget(btn, 1, boardIdx+1, Qt::AlignCenter);
            boardSelectors[boardIdx] = btn;
        }

        rowSelectors.resize(channelsPerBoard);
        for (int rowIdx = 0; rowIdx < channelsPerBoard; rowIdx++) {
            MyLeftRightMousePushButton * btn = new MyLeftRightMousePushButton();
            btn->setText(QString("%1").arg(rowIdx+1));
            btn->setFixedSize(maxButtonWidth, maxButtonHeight);
            connect(btn, &MyLeftRightMousePushButton::clicked, this, [=] (bool selected) {
                emit sigOneRowClicked(rowIdx, selected);
            });

            mainGl->addWidget(btn, rowIdx+2, 0, Qt::AlignCenter);
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
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    return qRound((screenGeometry.width() * SCREEN_PERCENTAGE_WIDTH) / channelsPerBoard);
}

int ChessboardDockWidget::getIdealPlotHeight() {
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    return  qRound((screenGeometry.height() * SCREEN_PERCENTAGE_HEIGHT) / (channelsPerBoard + 1));
}

void ChessboardDockWidget::updateBoardMappings(std::set <int> visibleBoards){
    for (int i=0; i<boardSelectors.size(); i++) {
        auto it = visibleBoards.find(i);
//      The element is not present in the set, so we can hide it
        boardSelectors[i]->setVisible(it != visibleBoards.end());
    }
}
