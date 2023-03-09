#ifndef BOARDCONTROLDOCKWIDGET_H
#define BOARDCONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QSpinBox>
#include <QHBoxLayout>

#include "modeldevice.h"

class BoardControlDockWidget : public QDockWidget {
    Q_OBJECT

public:
    BoardControlDockWidget(ModelDevice * mDev, QWidget * parent = nullptr);

private:
    QGridLayout * getLayoutWithScrollBar(QWidget * widget);

    ModelDevice * mDev;
};

#endif // BOARDCONTROLDOCKWIDGET_H
