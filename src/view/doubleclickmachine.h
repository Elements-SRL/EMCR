#ifndef DOUBLECLICKMACHINE_H
#define DOUBLECLICKMACHINE_H

#include "qwt_picker_machine.h"

class DoubleClickMachine: public QwtPickerMachine {
public:
    DoubleClickMachine(Qt::MouseButton btn);

    virtual QList <Command> transition(const QwtEventPattern &, const QEvent * event);

private:
    Qt::MouseButton btn;
};

#endif // DOUBLECLICKMACHINE_H
