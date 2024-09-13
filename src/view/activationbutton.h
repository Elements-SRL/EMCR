#ifndef ACTIVATIONBUTTON_H
#define ACTIVATIONBUTTON_H

#include <QLabel>

class ActivationButton : public QLabel {
    Q_OBJECT

public:
    explicit ActivationButton(QWidget * parent = nullptr);

    bool isChecked();

public slots:
    void click();
    void click(bool flag);
    void setChecked(bool flag);

protected:
    void mousePressEvent(QMouseEvent * event) override;

private:
    bool activated = false;
    QPixmap onPix;
    QPixmap offPix;

signals:
    void clicked(bool);
};

#endif // ACTIVATIONBUTTON_H
