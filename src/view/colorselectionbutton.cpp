#include "colorselectionbutton.h"

ColorSelectionButton::ColorSelectionButton(QWidget * parent) :
    QPushButton(parent) {

    this->setColor(QColor(Qt::white));
    this->setAutoFillBackground(true);
    this->setFixedWidth(30);
    this->setFixedHeight(30);
    this->setFlat(true);
}

void ColorSelectionButton::setColor(QColor color) {
    QPalette pal = this->palette();
    pal.setColor(QPalette::Button, color);
    this->setPalette(pal);
    this->update();
}
