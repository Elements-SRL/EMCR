#ifndef IVGRAPHWIDGET_H
#define IVGRAPHWIDGET_H

#include <tuple>
#include <map>

#include <QDockWidget>
#include <QBoxLayout>
#include <QPushButton>
#include <QHeaderView>
#include <QString>

#include "copyabletable.h"
#include "messagedispatcher.h"
#include "e384commlib_global_addendum.h"

class IvGraphWidget : public QDockWidget {
    Q_OBJECT

private:
    QTableWidget * dataTable;

public:
    IvGraphWidget(uint32_t channelsNum, QWidget * parent = nullptr);
    void setParams(std::map <uint32_t, std::vector <Measurement>> params);

signals:
    void exportIvGraph();
    void calcMeanSquared();
    void startIvGraph();
    void stopIvGraph();
};

#endif // IVGRAPHWIDGET_H
