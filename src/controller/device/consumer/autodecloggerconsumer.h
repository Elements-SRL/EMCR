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

public slots:
    virtual void onStartConsuming() override;
    virtual void onStopConsuming() override;
    virtual void onSamplingRateChanged(Measurement_t samplingRate) override;
    virtual void onDownsamplingRatioChanged(unsigned int downsamplingRatio) override;
    virtual void onVoltageRangeChanged(RangedMeasurement_t range) override;
    virtual void onCurrentRangeChanged(RangedMeasurement_t range) override;

signals:
    void sigDecloggingStarted(std::vector<int>);
    void sigDecloggingCompleted(std::vector<int>);

private:
    AutoDecloggerModel* model = nullptr;
    std::vector<double> buffer{};
    std::vector<std::vector<double>> currentValues{};
    std::vector<std::optional<QElapsedTimer*>> timers;
    std::vector<double> originalStimului;
    QMutex timeAxisMtx;
    QMutex rangeAxisMtx;

protected:
    void run() override;
};

#endif // AUTODECLOGGERCONSUMER_H
