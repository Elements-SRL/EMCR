#ifndef AUTODECLOGGERCONSUMER_H
#define AUTODECLOGGERCONSUMER_H

#include <QObject>
#include "devicedataconsumer.h"
#include "autodeclogger/autodecloggermodel.h"
#include <optional>

#define MIN_UPDATE_PLOT_TIME_MS (100) /*!< 100ms */

class AutodecloggerConsumer : public DeviceDataConsumer {
    Q_OBJECT

public:
    AutodecloggerConsumer(ApplicationStatus* appStatus, DeviceDataProducer* producer);
    ~AutodecloggerConsumer();
    void setModel(AutoDecloggerModel* model);
    void setThresholds(std::map<int, double> thresholds);
    void setTimes(std::map<int, double> thresholds);
    void setVoltages(std::map<int, double> thresholds);

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
    std::vector<double> buffer{};
    std::vector<std::vector<double>> currentValues;
    std::map<int, std::optional<double>> originalVoltages;
    std::map<int, std::optional<Measurement>> tunerResetValues;
    std::map<int, std::optional<QElapsedTimer*>> timers;
    QMutex timeAxisMtx;
    QMutex rangeAxisMtx;
    template <typename Lambda>
    void safeUpdate(Lambda lambda);

protected:
    void run() override;
};

#endif // AUTODECLOGGERCONSUMER_H
