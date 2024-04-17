#ifndef NOWHEELSPINBOX_H
#define NOWHEELSPINBOX_H


#include <QWidget>
#include <QDoubleSpinBox>

class NoWheelSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit NoWheelSpinBox(QWidget *parent = nullptr);
private:
    void wheelEvent(QWheelEvent *event);
signals:
public slots:
};

#endif // NOWHEELSPINBOX_H
