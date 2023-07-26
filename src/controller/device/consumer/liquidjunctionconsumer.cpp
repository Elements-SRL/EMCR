#include "liquidjunctionconsumer.h"

LiquidJunctionConsumer::LiquidJunctionConsumer(MessageDispatcher * msgDisp, DeviceDataProducer * producer) :
    DeviceDataConsumer(msgDisp, producer) {

}

LiquidJunctionConsumer::~LiquidJunctionConsumer() {

}

void LiquidJunctionConsumer::onStartConsuming() {
    ljHook = producer->getLiquidJunctionHook();
    if (ljHook != nullptr) {
        this->start();
    }
}

void LiquidJunctionConsumer::onStopConsuming() {
    if (this->isRunning()) {
        QMutexLocker consumptionLock(&consumptionMtx);
        consumptionStopped = true;
        while (!exitedDataConsumingLoop) {
            exitedDataConsumingLoopCv.wait(&consumptionMtx, 100);
        }
    }

    if (ljHook != nullptr) {
        delete ljHook;
        ljHook = nullptr;
    }
}

void LiquidJunctionConsumer::run() {
    QMutexLocker consumptionLock(&consumptionMtx);
    consumptionStopped = false;
    exitedDataConsumingLoop = false;
    consumptionLock.unlock();

    QVector <double> buffer;
    QVector <Measurement_t> bufferMeas;
    bufferMeas.resize(currentChannelsNum);
    RangedMeasurement_t range;

    while (true) {
        consumptionLock.relock();
        if (consumptionStopped) {
            break;
        }
        consumptionLock.unlock();

        if (ljHook->getLiquidJunctionValues(buffer)) {
            msgDisp->getLiquidJunctionRange(range);
            for (int idx = 0; idx < buffer.size(); idx++) {
                bufferMeas[idx] = {buffer[idx], range.prefix, "V"};
            }
            emit sigResult(bufferMeas);
        }
    }

    exitedDataConsumingLoop = true;
    exitedDataConsumingLoopCv.wakeAll();
}
