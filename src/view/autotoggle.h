#ifndef AUTOTOGGLE_H
#define AUTOTOGGLE_H

#include <QWidget>
#include <QLabel>
#include "activationbutton.h"

class AutoToggle : public QWidget {
    Q_OBJECT
public:
    explicit AutoToggle(QWidget *parent = nullptr);
    void setChecked(bool checked);
    bool isChecked() const;

signals:
    void toggled(bool checked);

private:
    ActivationButton *m_switch;
    QLabel *m_label;
};

#endif // AUTOTOGGLE_H
