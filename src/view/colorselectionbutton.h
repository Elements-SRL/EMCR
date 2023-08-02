#ifndef COLORSELECTIONBUTTON_H
#define COLORSELECTIONBUTTON_H

#include <QPushButton>

class ColorSelectionButton : public QPushButton {
    Q_OBJECT

public:
    explicit ColorSelectionButton(QWidget * parent = nullptr);
    void setColor(QColor color);
};


#endif // COLORSELECTIONBUTTON_H
