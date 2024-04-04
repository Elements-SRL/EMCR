#ifndef UPGRADEFWVIEW_H
#define UPGRADEFWVIEW_H

#include <QDialog>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

class UpgradeFwView : public QDialog {
    Q_OBJECT

public:
    UpgradeFwView(QWidget * parent = nullptr);

public slots:
    void onUpgradeAvailable(bool flag);
    void onSetMessage(QString message);

private:
    QLabel * messageLbl;
    QPushButton * okBtn;
    QPushButton * upgradeBtn;
    QPushButton * cancelBtn;

signals:
    void sigUpgradeFw();
};

#endif // UPGRADEFWVIEW_H
