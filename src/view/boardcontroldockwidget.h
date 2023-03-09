#ifndef BOARDCONTROLDOCKWIDGET_H
#define BOARDCONTROLDOCKWIDGET_H

#include <QDockWidget>
#include <QSpinBox>


#include "modeldevice.h"

class BoardControlDockWidget : public QDockWidget {
    Q_OBJECT

public:
    BoardControlDockWidget(ModelDevice * mDev, QWidget * parent = nullptr);

private:
    ModelDevice * mDev;
};

#endif // BOARDCONTROLDOCKWIDGET_H
