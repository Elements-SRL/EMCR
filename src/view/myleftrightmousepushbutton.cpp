#include "myleftrightmousepushbutton.h"

MyLeftRightMousePushButton::MyLeftRightMousePushButton(QWidget *parent) :
    QPushButton(parent)
{
}

void MyLeftRightMousePushButton::mousePressEvent(QMouseEvent *e)
{
    if(e->button()==Qt::RightButton)
        emit clicked(false);
    if(e->button()==Qt::LeftButton)
        emit clicked(true);
}

