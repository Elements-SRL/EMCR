#include "chessboard.h"

#include <QBoxLayout>

Chessboard::Chessboard(ModelDevice * mDev, QWidget * parent) :
    QWidget(parent) {
    int voltageChannelsNum;
    int currentChannelsNum;
    int boardsNum;
    int channelsPerBoard = currentChannelsNum/boardsNum;

    mDev->getChannelsNumberFeatures(voltageChannelsNum, currentChannelsNum);
    mDev->getBoardsNumberFeatures(boardsNum);

    QGridLayout * mainGl = new QGridLayout;
    mainGl->setMargin(0);
    this->setLayout(mainGl);

    allChannelsSelector = new QPushButton("ALL");
    allChannelsSelector->setCheckable(true);
    allChannelsSelector->setChecked(false);
    allChannelsSelector->setFixedSize(30, 30);
    connect(allChannelsSelector, &QPushButton::clicked, this, &Chessboard::allChannelsClicked);

    mainGl->addWidget(allChannelsSelector, 0, 0);

    boardSelectors.resize(boardsNum);
    for (int boardIdx = 0; boardIdx < boardsNum; boardIdx++) {
        QPushButton * btn = new QPushButton(QString("%1").arg(boardIdx+1));
        btn->setCheckable(true);
        btn->setChecked(false);
        btn->setFixedSize(30, 30);
        connect(btn, &QPushButton::clicked, this, [=] (bool selected) {
            emit oneBoardClicked(boardIdx, selected);
        });

        mainGl->addWidget(btn, 0, boardIdx);
        boardSelectors[boardIdx] = btn;
    }

    rowSelectors.resize(channelsPerBoard);
    for (int rowIdx = 0; rowIdx < channelsPerBoard; rowIdx++) {
        QPushButton * btn = new QPushButton(QString("%1").arg(rowIdx+1));
        btn->setCheckable(true);
        btn->setChecked(false);
        btn->setFixedSize(30, 30);
        connect(btn, &QPushButton::clicked, this, [=] (bool selected) {
            emit oneRowClicked(rowIdx, selected);
        });

        mainGl->addWidget(btn, rowIdx, 0);
        rowSelectors[rowIdx] = btn;
    }

    int boardIdx = 0;
    int rowIdx = 0;
    plots.resize(currentChannelsNum);
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        StampPlot * plot = new StampPlot();
        plot->setFixedSize(30, 30);
        connect(plot, &StampPlot::selected, this, [=] (bool selected) {
            emit singleChannelsClicked(channelIdx, selected);
        });

        rowIdx++;
        if (rowIdx == channelsPerBoard) {
            rowIdx = 0;
            boardIdx++;
        }
        mainGl->addWidget(plot, rowIdx+1, boardIdx+1);
        plots[channelIdx] = plot;
    }
}
