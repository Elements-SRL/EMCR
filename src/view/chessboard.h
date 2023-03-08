#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QWidget>
#include <QPushButton>

#include "stampplot.h"
#include "modeldevice.h"

class Chessboard : public QWidget {
    Q_OBJECT

public:
    Chessboard(ModelDevice * mDev, QWidget * parent = nullptr);

private:
    QPushButton * allChannelsSelector = nullptr;
    QVector <QPushButton *> boardSelectors;
    QVector <QPushButton *> rowSelectors;
    QVector <StampPlot *> plots;

signals:
    void allChannelsClicked(bool newChannelState);
    void oneBoardClicked(uint16_t changedBoardIndex, bool newChannelState);
    void oneRowClicked(uint16_t changedRowIndex, bool newChannelState);
    void singleChannelClicked(uint16_t changedChannelIndex, bool newChannelState);
};

#endif // CHESSBOARD_H
