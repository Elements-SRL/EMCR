#include "activationbutton.h"
#include <QStyle>

ActivationButton::ActivationButton(QWidget * parent) :
    QLabel(parent) {
    this->setObjectName("activationSwitch");
    this->setFixedSize(32, 32);
    this->setScaledContents(true);
    this->setChecked(false);
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
    this->setProperty("checked", activated);
    this->style()->unpolish(this);
    this->style()->polish(this);
    this->update();
}

void ActivationButton::mousePressEvent(QMouseEvent *) {
    this->click();
}
