#include "chessboarddockwidget.h"

#include <QBoxLayout>
#include <QComboBox>
#include <QApplication>
#include "globaldefines.h"
#include "qscrollarea.h"
#include <QScreen>

ChessboardDockWidget::ChessboardDockWidget(ApplicationStatus * appStatus, QWidget * parent) :
    QDockWidget(parent) {

    this->setObjectName("chessboard");
    this->setWindowTitle("Channels overview");

    this->voltageChannelsNum = appStatus->getVoltageChannelsNum();
    this->currentChannelsNum = appStatus->getCurrentChannelsNum();
    this->boardsNum = appStatus->getBoardsNum();
    channelsPerBoard = currentChannelsNum/boardsNum;

    QWidget * mainWg = new QWidget(parent);
    mainWg->setObjectName("chessboardWg");
    this->setWidget(mainWg);
    QVBoxLayout * topLevelLayout = new QVBoxLayout(mainWg);
    topLevelLayout->setContentsMargins(10, 0, 10, 10);
    topLevelLayout->setSpacing(0);

    // TOP BAR
    QWidget* customTitleBar = new QWidget();
    customTitleBar->setObjectName("customTitleBar");
    QHBoxLayout* topBarLayout = new QHBoxLayout(customTitleBar);

    // Button ALL
    allChannelsSelector = new LeftRightMousePushButton(this);
    allChannelsSelector->setText("ALL");
    connect(allChannelsSelector, &LeftRightMousePushButton::clicked, this, &ChessboardDockWidget::sigAllChannelsClicked);
    topBarLayout->addWidget(allChannelsSelector);

    // Button NONE
    QPushButton * noneBtn = new QPushButton("NONE", this);
    connect(noneBtn, &QPushButton::clicked, this, [=]() {
        emit sigAllChannelsClicked(false);
    });
    topBarLayout->addWidget(noneBtn);

    // Botton INVERT
    QPushButton * invertBtn = new QPushButton("INVERT", this);
    connect(invertBtn, &QPushButton::clicked, this, [=]() {
        emit sigInvertSelectionClicked();
    });
    topBarLayout->addWidget(invertBtn);

    topLevelLayout->addWidget(customTitleBar);

    // --- SCROLL AREA --- viewport mode to enable
    // navigation when there are many channels active
    QScrollArea * scrollArea = new QScrollArea(this);
    scrollArea->setObjectName("chessboardScrollArea");
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QWidget * containerWg = new QWidget();
    containerWg->setObjectName("gridContainer");
    containerWg->setContentsMargins(12, 12, 12, 12);

    mainGl = new QGridLayout(containerWg);
    mainGl->setContentsMargins(0, 0, 0, 0);
    mainGl->setSpacing(8);

    scrollArea->setWidget(containerWg);
    topLevelLayout->addWidget(scrollArea);

    // --- Legenda footer ---
    QWidget* legenda = new QWidget();
    legenda->setObjectName("customFooter");
    QHBoxLayout * legendaLayout = new QHBoxLayout(legenda);
    legendaLayout->setContentsMargins(4, 4, 4, 4);

    // TODO move in QSS
    QLabel * selectedDot = new QLabel(this);
    selectedDot->setFixedSize(12, 12);
    selectedDot->setStyleSheet("background-color: #3b82f6; border-radius: 3px;"); // Azzurro
    QLabel * selectedText = new QLabel("Selected", this);
    selectedText->setStyleSheet("color: #8a92a3;");

    legendaLayout->addWidget(selectedDot);
    legendaLayout->addWidget(selectedText);
    legendaLayout->addSpacing(15);

    legendaLayout->addStretch();
    topLevelLayout->addWidget(legenda);

    if (boardsNum > 1 && channelsPerBoard > 1) {
        boardSelectors.resize(boardsNum);
        for (int boardIdx = 0; boardIdx < boardsNum; boardIdx++) {
            LeftRightMousePushButton * btn = new LeftRightMousePushButton();
            btn->setText(QString("%1").arg(boardIdx+1));
            connect(btn, &LeftRightMousePushButton::clicked, this, [=] (bool selected) {
                emit sigOneBoardClicked(boardIdx, selected);
            });

            mainGl->addWidget(btn, 1, boardIdx+1, Qt::AlignCenter);
            boardSelectors[boardIdx] = btn;
        }

        rowSelectors.resize(channelsPerBoard);
        for (int rowIdx = 0; rowIdx < channelsPerBoard; rowIdx++) {
            LeftRightMousePushButton * btn = new LeftRightMousePushButton();
            btn->setText(QString("%1").arg(rowIdx+1));
            connect(btn, &LeftRightMousePushButton::clicked, this, [=] (bool selected) {
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
    }
    else if (boardsNum > 1) {
        mainGl->addWidget(plot, rowIdx+1, boardIdx+1);
    }
    else if (channelsPerBoard > 1) {
        mainGl->addWidget(plot, rowIdx+2, boardIdx);
    }
    else {
        mainGl->addWidget(plot, rowIdx+1, boardIdx);
    }

    plot->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    connect(plot, &StampPlot::clicked, this, [=] (QMouseEvent *event) {
        emit sigSingleChannelClicked(channelIdx, event);
    });
}

int ChessboardDockWidget::getIdealPlotWidth() {
    QScreen * screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    double height = screenGeometry.height() * CHB_SCREEN_PERCENTAGE_HEIGHT / channelsPerBoard;
    double width = screenGeometry.width() * CHB_SCREEN_PERCENTAGE_WIDTH / boardsNum;
    return qRound(std::min(width, height / CHB_PLOT_ASPECT_RATIO));
}

int ChessboardDockWidget::getIdealPlotHeight() {
    QScreen * screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    double height = screenGeometry.height() * CHB_SCREEN_PERCENTAGE_HEIGHT / channelsPerBoard;
    double width = screenGeometry.width() * CHB_SCREEN_PERCENTAGE_WIDTH / boardsNum;
    return qRound(std::min(width * CHB_PLOT_ASPECT_RATIO, height));
}

void ChessboardDockWidget::updateBoardMappings(std::set <int> visibleBoards){
    for (int i=0; i<boardSelectors.size(); i++) {
        auto it = visibleBoards.find(i);
//      The element is not present in the set, so we can hide it
        boardSelectors[i]->setVisible(it != visibleBoards.end());
    }
    emit sigAllChannelsClicked(false);
}
