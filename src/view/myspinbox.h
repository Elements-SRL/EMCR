#ifndef MYSPINBOX_H
#define MYSPINBOX_H


#include <QWidget>
#include <QDoubleSpinBox>
#include <QDebug>
class MySpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit MySpinBox(QWidget *parent = nullptr);
private:
    void wheelEvent(QWheelEvent *event);
signals:
public slots:
};

#endif // MYSPINBOX_H
