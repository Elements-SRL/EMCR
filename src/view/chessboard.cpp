#include "chessboard.h"

#include <QBoxLayout>

#include "globaldefines.h"

Chessboard::Chessboard(ModelDevice * mDev, QWidget * parent) :
    QWidget(parent) {

    int boardsNum;

    mDev->getChannelsNumberFeatures(voltageChannelsNum, currentChannelsNum);
    mDev->getBoardsNumberFeatures(boardsNum);
    int channelsPerBoard = currentChannelsNum/boardsNum;

    QGridLayout * mainGl = new QGridLayout;
    mainGl->setMargin(0);
    mainGl->setSpacing(1);
    this->setLayout(mainGl);

    allChannelsSelector = new MyLeftRightMousePushButton();
    allChannelsSelector->setText("ALL");
    allChannelsSelector->setFixedSize(STAMP_PLOT_SIZE, STAMP_PLOT_SIZE);
    connect(allChannelsSelector, &MyLeftRightMousePushButton::clicked, this, &Chessboard::allChannelsClicked);
    connect(allChannelsSelector, &MyLeftRightMousePushButton::clicked, this, [=](bool selected) {
        for(int ii = 0; ii < currentChannelsNum; ii++){
            if(selected){
                plots[ii]->setStyleSheet("StampPlot { border: 3px solid green; }");
            } else {
                plots[ii]->setStyleSheet("StampPlot { border: 3px solid black; }");
            }
        }
    });


    mainGl->addWidget(allChannelsSelector, 0, 0);

    boardSelectors.resize(boardsNum);
    for (int boardIdx = 0; boardIdx < boardsNum; boardIdx++) {
        MyLeftRightMousePushButton * btn = new MyLeftRightMousePushButton();
        btn->setText(QString("%1").arg(boardIdx+1));
        btn->setFixedSize(STAMP_PLOT_SIZE, STAMP_PLOT_SIZE);
        connect(btn, &MyLeftRightMousePushButton::clicked, this, [=] (bool selected) {
            for(int ii = boardIdx*channelsPerBoard; ii < boardIdx*channelsPerBoard + channelsPerBoard; ii++){
                if(selected){
                    plots[ii]->setStyleSheet("StampPlot { border: 3px solid green; }");
                } else {
                    plots[ii]->setStyleSheet("StampPlot { border: 3px solid black; }");
                }
            }
            emit oneBoardClicked(boardIdx, selected);
        });

        mainGl->addWidget(btn, 0, boardIdx+1);
        boardSelectors[boardIdx] = btn;
    }

    rowSelectors.resize(channelsPerBoard);
    for (int rowIdx = 0; rowIdx < channelsPerBoard; rowIdx++) {
        MyLeftRightMousePushButton * btn = new MyLeftRightMousePushButton();
        btn->setText(QString("%1").arg(rowIdx+1));
        btn->setFixedSize(STAMP_PLOT_SIZE, STAMP_PLOT_SIZE);
        connect(btn, &MyLeftRightMousePushButton::clicked, this, [=] (bool selected) {
            for(int ii = rowIdx; ii < rowIdx+channelsPerBoard*(boardsNum); ii = ii+channelsPerBoard){
                if(selected){
                    plots[ii]->setStyleSheet("StampPlot { border: 3px solid green; }");
                } else {
                    plots[ii]->setStyleSheet("StampPlot { border: 3px solid black; }");
                }
            }
            emit oneRowClicked(rowIdx, selected);
        });

        mainGl->addWidget(btn, rowIdx+1, 0);
        rowSelectors[rowIdx] = btn;
    }

    int boardIdx = 0;
    int rowIdx = 0;
    plots.resize(currentChannelsNum);
    currentCurves.resize(currentChannelsNum);
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        StampPlot * plot = new StampPlot();
        plot->setFixedSize(STAMP_PLOT_SIZE, STAMP_PLOT_SIZE);
        plot->setToolTip(QString("Ch %1\nRight click: select\nLeft click: deselect").arg(channelIdx+1));
        connect(plot, &StampPlot::selected, this, [=] (bool selected) {
            if(selected){
                plot->setStyleSheet("StampPlot { border: 3px solid green; }");
            } else {
                plot->setStyleSheet("StampPlot { border: 3px solid black; }");
            }

            emit singleChannelClicked(channelIdx, selected);
        });

        mainGl->addWidget(plot, rowIdx+1, boardIdx+1);
        rowIdx++;
        if (rowIdx == channelsPerBoard) {
            rowIdx = 0;
            boardIdx++;
        }
        plots[channelIdx] = plot;

        Curve * curve = new Curve(CurveType_t::CurveTypeStampPlotSolid);
        curve->attach(plot);
        currentCurves[channelIdx] = curve;
    }
}

void Chessboard::initializeRange(RangedMeasurement_t newRange) {
    for (auto plot : plots) {
        plot->initializeRange(newRange);
    }
}

void Chessboard::clearCurves() {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves[idx]->detach();
        delete currentCurves[idx];
        delete [] currentCurves[idx];
    }
    currentCurves.clear();
}

void Chessboard::onRangeUpdated(RangedMeasurement_t newRange) {
    for (auto plot : plots) {
        plot->onRangeUpdated(newRange);
    }
}

void Chessboard::onDurationUpdated(Measurement_t duration) {
    for (auto plot : plots) {
        plot->onDurationUpdated(duration);
    }
}

void Chessboard::onSetGapFreePlotData(double * timeValues, QVector <double *> * voltageValues, QVector <double *> * currentValues, int dataSize) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves.at(idx)->setRawSamples(timeValues, currentValues->at(idx), dataSize);
    }
}

void Chessboard::onReplot() {
    for (auto plot : plots) {
        plot->replot();
    }
}
