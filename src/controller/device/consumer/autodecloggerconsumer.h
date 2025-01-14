#ifndef AUTODECLOGGERCONSUMER_H
#define AUTODECLOGGERCONSUMER_H

#include <QObject>
#include "devicedataconsumer.h"
#include "autodeclogger/autodecloggermodel.h"
#include <optional>

#define MIN_UPDATE_PLOT_TIME_MS (100) /*!< 100ms */

struct ClogInfo {
    Measurement originalVoltageHoldTuner;
    Measurement vToApply;
    QElapsedTimer* timer;
    bool decloggingComplete;

    static ClogInfo create(Measurement originalVoltageHoldTuner, Measurement vToApply, QElapsedTimer* timer) {
        return { originalVoltageHoldTuner, vToApply, timer, false };
    }
};

class AutodecloggerConsumer : public DeviceDataConsumer {
    Q_OBJECT

public:
    AutodecloggerConsumer(ApplicationStatus* appStatus, DeviceDataProducer* producer);
    ~AutodecloggerConsumer();
    void setModel(AutoDecloggerModel* model);
    void setThresholds(std::map<int, double> );
    void setTimes(std::map<int, double> );
    void setCooldownTimes(std::map<int, double> );
    void setVoltages(std::map<int, double> );

public slots:
    virtual void onStartConsuming() override;
    virtual void onStopConsuming() override;
    virtual void onSamplingRateChanged(Measurement_t samplingRate) override;
    virtual void onDownsamplingRatioChanged(unsigned int downsamplingRatio) override;
    virtual void onVoltageRangeChanged(RangedMeasurement_t range) override;
    virtual void onCurrentRangeChanged(RangedMeasurement_t range) override;

signals:
    //void sigDecloggingStarted(std::vector<unsigned short>);
    //void sigDecloggingCompleted(std::vector<unsigned short>);
    void sigDecloggingStarted(std::vector<unsigned short>);
    void sigDecloggingCompleted(std::vector<unsigned short>);

private:
    AutoDecloggerModel* model = nullptr;
    std::vector<double> buffer;
    std::map<int, std::optional<Measurement>> originalVoltages;
    std::map<int, std::optional<ClogInfo>> clogInfo;
    QMutex timeAxisMtx;
    QMutex rangeAxisMtx;

    template <typename Lambda>
    void safeUpdate(Lambda lambda);
    void complete();
    void resetStim();

protected:
    void run() override;
};

#endif // AUTODECLOGGERCONSUMER_H
