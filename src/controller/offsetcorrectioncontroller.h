#ifndef OFFSETCORRECTIONCONTROLLER_H
#define OFFSETCORRECTIONCONTROLLER_H

#include <QThread>

#include "application_status.h"

class OffsetCorrectionController : public QThread {
    Q_OBJECT

public:
    explicit OffsetCorrectionController(ApplicationStatus * appStatus, QObject * parent = nullptr);

    typedef enum OffsetCorrectionCheck {
        CheckingNone,
        CheckingOffsetRecalibration,
        CheckingLiquidJunctionCorrection
    } OffsetCorrectionCheck_t;

public slots:
    void onStartChecking(OffsetCorrectionCheck_t check);

protected:
    void run() override;

private:
    ApplicationStatus * appStatus = nullptr;
    OffsetCorrectionCheck_t offsetCheck = CheckingNone;
};

#endif // OFFSETCORRECTIONCONTROLLER_H
