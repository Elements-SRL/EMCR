#ifndef PLOTDETAIL_H
#define PLOTDETAIL_H

#include <QWidget>
#include "plotdetailmodel.h"
#include "baseplot.h"

class PlotDetail : public QWidget
{
    Q_OBJECT

private:
    PlotDetailModel * pdm;
    BasePlot* plot;

protected:
    void closeEvent(QCloseEvent *event) override;

public:
    PlotDetail(PlotDetailModel * pdm, QWidget * parent = nullptr);
    uint16_t getChannel();
    void replot();
signals:
    void close();
};

#endif // PLOTDETAIL_H
