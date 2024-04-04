#ifndef UPGRADEFWCONTROLLER_H
#define UPGRADEFWCONTROLLER_H

#include <QThread>

#include "upgradefwview.h"

class UpgradeFwController : public QThread {
    Q_OBJECT

public:
    UpgradeFwController();
    ~UpgradeFwController();

    void openView(QString deviceId);

public slots:
    void onUpgradeFw();

protected:
    void run() override;

private:
    UpgradeFwView * view;
    QString deviceId = "";

signals:
    void sigEnableView(bool flag);
    void sigUpgradeAvilable(bool flag);
    void sigSetMessage(QString text);
};

#endif // UPGRADEFWCONTROLLER_H
