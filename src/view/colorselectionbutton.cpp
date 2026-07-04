#include "colorselectionbutton.h"

ColorSelectionButton::ColorSelectionButton(QWidget * parent) :
    QPushButton(parent) {
    this->setFixedWidth(32);
    this->setFixedHeight(32);
}

void ColorSelectionButton::setColor(QColor color) {
    QPalette pal = this->palette();
    pal.setColor(QPalette::Button, color);
    this->setPalette(pal);
    this->update();
}
