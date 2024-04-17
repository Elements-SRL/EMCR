#include "leftrightmousepushbutton.h"

LeftRightMousePushButton::LeftRightMousePushButton(QWidget *parent) :
    QPushButton(parent)
{
}

void LeftRightMousePushButton::mousePressEvent(QMouseEvent *e)
{
    if(e->button()==Qt::RightButton)
        emit clicked(false);
    if(e->button()==Qt::LeftButton)
        emit clicked(true);
}

