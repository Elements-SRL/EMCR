#ifndef PLOTDETAIL_H
#define PLOTDETAIL_H

#include <QWidget>
#include "plotdetailmodel.h"

class PlotDetail : public QWidget
{
    Q_OBJECT

private:
    PlotDetailModel * pdm;

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    PlotDetail(PlotDetailModel * pdm, QWidget * parent = nullptr);
    uint16_t getChannel();
signals:
    void close();
};

#endif // PLOTDETAIL_H
