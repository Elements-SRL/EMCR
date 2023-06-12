#include "chessboard.h"

#include <QBoxLayout>
#include <QComboBox>

#include "globaldefines.h"

Chessboard::Chessboard(ModelDevice * mDev, QWidget * parent) :
    QWidget(parent),
    mDev(mDev) {

    int boardsNum;

    mDev->getChannelsNumberFeatures(voltageChannelsNum, currentChannelsNum);
    mDev->getBoardsNumberFeatures(boardsNum);
    int channelsPerBoard = currentChannelsNum/boardsNum;

    QGridLayout * mainGl = new QGridLayout;
    mainGl->setMargin(0);
    mainGl->setSpacing(1);
    this->setLayout(mainGl);

    QComboBox * visualizationCbx = new QComboBox;
    visualizationCbx->addItem("Plot Overview");
    visualizationCbx->addItem("Noise Overview");

    mainGl->addWidget(visualizationCbx, 0, 0, 1, (boardsNum+1)/2);

    QPushButton * noiseExportBtn = new QPushButton("Export");
    noiseExportBtn->setVisible(false);
    noiseExportBtn->setCheckable(false);
    connect(noiseExportBtn, &QPushButton::clicked, this, &Chessboard::sigExportLiveNoiseEstimates);

    mainGl->addWidget(noiseExportBtn, 0, (boardsNum+1)/2, 1, (boardsNum+1)/2);

    allChannelsSelector = new MyLeftRightMousePushButton();
    allChannelsSelector->setText("ALL");
    allChannelsSelector->setFixedSize(STAMP_PLOT_SIZE, STAMP_PLOT_SIZE);
    connect(allChannelsSelector, &MyLeftRightMousePushButton::clicked, this, &Chessboard::allChannelsClicked);

    mainGl->addWidget(allChannelsSelector, 1, 0);

    boardSelectors.resize(boardsNum);
    for (int boardIdx = 0; boardIdx < boardsNum; boardIdx++) {
        MyLeftRightMousePushButton * btn = new MyLeftRightMousePushButton();
        btn->setText(QString("%1").arg(boardIdx+1));
        btn->setFixedSize(STAMP_PLOT_SIZE, STAMP_PLOT_SIZE);
        connect(btn, &MyLeftRightMousePushButton::clicked, this, [=] (bool selected) {
            emit oneBoardClicked(boardIdx, selected);
        });

        mainGl->addWidget(btn, 1, boardIdx+1);
        boardSelectors[boardIdx] = btn;
    }

    rowSelectors.resize(channelsPerBoard);
    for (int rowIdx = 0; rowIdx < channelsPerBoard; rowIdx++) {
        MyLeftRightMousePushButton * btn = new MyLeftRightMousePushButton();
        btn->setText(QString("%1").arg(rowIdx+1));
        btn->setFixedSize(STAMP_PLOT_SIZE, STAMP_PLOT_SIZE);
        connect(btn, &MyLeftRightMousePushButton::clicked, this, [=] (bool selected) {
            emit oneRowClicked(rowIdx, selected);
        });

        mainGl->addWidget(btn, rowIdx+2, 0);
        rowSelectors[rowIdx] = btn;
    }

    int boardIdx = 0;
    int rowIdx = 0;

    overviewWidgets.resize(currentChannelsNum);

    plots.resize(currentChannelsNum);
    currentCurves.resize(currentChannelsNum);
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        ChannelOverviewWidget * wid = new ChannelOverviewWidget;
        wid->setFixedSize(STAMP_PLOT_SIZE, STAMP_PLOT_SIZE);
        wid->setChannelIndex(channelIdx);


        connect(wid, &ChannelOverviewWidget::clicked, this, [=] (bool selected) {
            emit singleChannelClicked(channelIdx, selected);
        });

        connect(visualizationCbx, QOverload <int> ::of(&QComboBox::currentIndexChanged), this, [=] (int idx) {
            wid->setVisualizationOption((ChannelOverviewWidget::VisualizationOption_t)idx);
            noiseExportBtn->setVisible(idx == ChannelOverviewWidget::Noise);
        });

        mainGl->addWidget(wid, rowIdx+2, boardIdx+1);
        rowIdx++;
        if (rowIdx == channelsPerBoard) {
            rowIdx = 0;
            boardIdx++;
        }
        overviewWidgets[channelIdx] = wid;



        /*! buttare in una funziioncina di creazione del plot*/
        StampPlot * plot = new StampPlot();
        plot->setFixedSize(STAMP_PLOT_SIZE, STAMP_PLOT_SIZE);
        plot->setToolTip(QString("Ch %1\nRight click: select\nLeft click: deselect").arg(channelIdx+1));
        plot->setSelected(false);


        plots[channelIdx] = plot;

        Curve * curve = new Curve(CurveType_t::CurveTypeStampPlotSolid);
        curve->attach(plot);
        currentCurves[channelIdx] = curve;

        wid->setStampPlot(plot);




        wid->setVisualizationOption(ChannelOverviewWidget::Plot);
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

void Chessboard::onRangeUpdated(RangedMeasurement_t newRange, QwtPlot::Axis axisIdx) {
    for (auto plot : plots) {
        plot->onRangeUpdated(newRange, axisIdx);
    }
}

void Chessboard::onDurationUpdated(Measurement_t duration) {
    for (auto plot : plots) {
        plot->onDurationUpdated(duration);
    }
}

/*! channelsToPlotNumber is ignored by the chessBoard*/
void Chessboard::onSetGapFreePlotData(double * timeValues, QVector <double *> * voltageValues, QVector <double *> * currentValues, int dataSize, int channelsToPlotNumber) {
    for (int idx = 0; idx < currentChannelsNum; idx++) {
        currentCurves.at(idx)->setRawSamples(timeValues, currentValues->at(idx), dataSize);
    }
}

void Chessboard::onReplot() {
    for (auto plot : plots) {
        plot->replot();
    }
}

void Chessboard::onSelectedPlotsUdpated() {
    QVector <bool> selectedChannels = mDev->getSelectedChannelsIdxs();
    for(int ii = 0; ii < currentChannelsNum; ii++){
        plots[ii]->setSelected(selectedChannels[ii]);
    }
}

void Chessboard::onNoiseValueUpdated(LiveNoiseConsumer::Result_t result) {
    for (int channelIdx = 0; channelIdx < currentChannelsNum; channelIdx++) {
        overviewWidgets[channelIdx]->setNoiseValue({result.stdCurrent[channelIdx], UnitPfxNone, "A"});
    }
}
