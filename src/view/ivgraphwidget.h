#ifndef IVGRAPHWIDGET_H
#define IVGRAPHWIDGET_H

#include <QDockWidget>
#include <QBoxLayout>
#include <QPushButton>

class IvGraphWidget : public QDockWidget
{
    Q_OBJECT
private:
    QPushButton exportButton;

public:
    explicit IvGraphWidget(QWidget *parent = nullptr);

signals:
    void exportIvGraph();
};

#endif // IVGRAPHWIDGET_H
