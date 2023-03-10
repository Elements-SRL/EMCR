#ifndef MYLEFTRIGHTMOUSEPUSHBUTTON_H
#define MYLEFTRIGHTMOUSEPUSHBUTTON_H

#include <QPushButton>
#include <QMouseEvent>

class MyLeftRightMousePushButton: public QPushButton
{
    Q_OBJECT

public:
    explicit MyLeftRightMousePushButton(QWidget *parent = 0);

private slots:
    void mousePressEvent(QMouseEvent *e);

signals:
    void rightClicked();

public slots:

};

#endif // MYLEFTRIGHTMOUSEPUSHBUTTON_H
