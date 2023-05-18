#include "doubleclickmachine.h"

#include <QMouseEvent>

DoubleClickMachine::DoubleClickMachine(Qt::MouseButton btn) :
    QwtPickerMachine(PointSelection),
    btn(btn) {
}

QList <QwtPickerMachine::Command> DoubleClickMachine::transition(const QwtEventPattern &, const QEvent * event) {
    QList <QwtPickerMachine::Command> cmdList;
    if ((event->type() == QEvent::MouseButtonDblClick) &&
            (((const QMouseEvent *)event)->button() == btn)) {
        cmdList += Begin;
        cmdList += Append;
        cmdList += End;
    }

    return cmdList;
}
