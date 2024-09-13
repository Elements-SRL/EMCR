#include "activationbutton.h"

ActivationButton::ActivationButton(QWidget * parent) :
    QLabel(parent) {

    onPix = QPixmap(":/imgs/activation button on.png").scaledToHeight(20, Qt::SmoothTransformation);
    offPix = QPixmap(":/imgs/activation button off.png").scaledToHeight(20, Qt::SmoothTransformation);
    this->setPixmap(offPix);
    this->setFixedSize(offPix.width(), offPix.height());
}

bool ActivationButton::isChecked() {
    return activated;
}

void ActivationButton::click() {
    this->click(!activated);
}

void ActivationButton::click(bool flag) {
    this->setChecked(flag);
    emit clicked(activated);
}

void ActivationButton::setChecked(bool flag) {
    activated = flag;
    if (activated) {
        this->setPixmap(onPix);

    } else {
        this->setPixmap(offPix);
    }
}

void ActivationButton::mousePressEvent(QMouseEvent *) {
    this->click();
}
