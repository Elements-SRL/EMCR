#ifndef LEFTRIGHTMOUSEPUSHBUTTON_H
#define LEFTRIGHTMOUSEPUSHBUTTON_H

#include <QPushButton>
#include <QMouseEvent>

class LeftRightMousePushButton: public QPushButton
{
    Q_OBJECT

public:
    explicit LeftRightMousePushButton(QWidget *parent = 0);

private slots:
    void mousePressEvent(QMouseEvent *e);

signals:
    void rightClicked();

public slots:

};

#endif // LEFTRIGHTMOUSEPUSHBUTTON_H
