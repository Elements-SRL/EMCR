#include "protocolitem.h"

#define ONE_MS {1.0, UnitPfxMilli, "s"}

ProtocolItem::ProtocolItem(ProtocolItemType_t type, int nextItem, int repsNum, bool applySteps, bool visible, bool stimHalfFlag) :
    type(type),
    nextItem(nextItem),
    repsNum(repsNum),
    applySteps(applySteps),
    visible(visible),
    stimHalfFlag(stimHalfFlag){
}

ProtocolItem::ProtocolItem(ProtocolItemType_t type, int nextItem, bool visible) :
    type(type),
    nextItem(nextItem),
    visible(visible) {
    repsNum = 1;
    applySteps = false;
}

ProtocolItem::~ProtocolItem() {

}

Measurement_t ProtocolItem::duration(int, bool withHidden) {
    return this->duration(withHidden);
}

Measurement_t ProtocolItem::maxDuration(int, bool withHidden) {
    /*! This function is meant to be used in episodic protocols, where only the longest item counts */
    return this->duration(withHidden);
}

Measurement_t ProtocolItem::totalDuration(int stepsNum, bool withHidden) {
    /*! This function is meant to be used in gap free protocols, where the sum of the items counts */
    return this->duration(withHidden)*(stepsNum == 0 ? 1.0 : (double)stepsNum);
}

void ProtocolItem::print() {
    qDebug() << "type:" << type;
    qDebug() << "nextItem" << nextItem;
    qDebug() << "repsNum" << repsNum;
    qDebug() << "visible" << visible;
    qDebug() << "";
}

ProtocolXStepTStepItem::ProtocolXStepTStepItem(ProtocolDropItem * item, double hold, int nextItem) :
    ProtocolItem(ProtocolItemXStepTStep, nextItem, item->hasVisibleData()) {
    ProtocolDropVStepTStepItem * castItem = static_cast <ProtocolDropVStepTStepItem *> (item);
    x0 = castItem->getX0()+hold;
    xStep = castItem->getXStep();
    t0 = castItem->getT0();
    tStep = castItem->getTStep();
    stimHalfFlag = castItem->getStimHalf();
}

ProtocolXStepTStepItem::ProtocolXStepTStepItem(ProtocolItem * item, int nextItem) :
    ProtocolItem(item->type, nextItem, item->repsNum, item->applySteps, item->visible, item->stimHalfFlag) {
    ProtocolXStepTStepItem * castItem = static_cast <ProtocolXStepTStepItem *> (item);
    x0 = castItem->x0;
    xStep = castItem->xStep;
    t0 = castItem->t0;
    tStep = castItem->tStep;
}

ProtocolXStepTStepItem::ProtocolXStepTStepItem(ProtocolItem * item) :
    ProtocolItem(item->type, item->nextItem, item->repsNum, item->applySteps, item->visible, item->stimHalfFlag) {
    ProtocolXStepTStepItem * castItem = static_cast <ProtocolXStepTStepItem *> (item);
    x0 = castItem->x0;
    xStep = castItem->xStep;
    t0 = castItem->t0;
    tStep = castItem->tStep;
}

ProtocolXStepTStepItem::ProtocolXStepTStepItem(double x0, double xStep, double t0, double tStep,
                                               ProtocolItemType_t type, int nextItem, bool visible) :
    ProtocolItem(type, nextItem, visible),
    x0(x0),
    xStep(xStep),
    t0(t0),
    tStep(tStep) {

}

ProtocolXStepTStepItem::~ProtocolXStepTStepItem() {

}

Measurement_t ProtocolXStepTStepItem::duration(bool withHidden) {
    Measurement_t one = ONE_MS;
    return one*((visible | withHidden) ? t0 : 0.0);
}

Measurement_t ProtocolXStepTStepItem::duration(int repsIdx, bool withHidden) {
    Measurement_t one = ONE_MS;
    return one*((visible | withHidden) ? t0+tStep*(((double)repsIdx)-1.0) : 0.0);
}

Measurement_t ProtocolXStepTStepItem::maxDuration(int stepsNum, bool withHidden) {
    /*! This function is meant to be used in episodic protocols, where only the longest item counts */
    if (tStep > 0.0) {
        return this->duration(stepsNum, withHidden);

    } else {
        return this->duration(withHidden);
    }
}

Measurement_t ProtocolXStepTStepItem::totalDuration(int stepsNum, bool withHidden) {
    /*! This function is meant to be used in gap free protocols, where the sum of the items counts */
    double stepsNumF = (double)(stepsNum == 0 ? 1 : stepsNum);
    double steppingAddend = (applySteps ? tStep*(stepsNumF-1.0)/2.0 : 0.0);
    Measurement_t one = ONE_MS;
    return one*((visible | withHidden) ? (t0+steppingAddend)*stepsNumF : 0.0);
}

void ProtocolXStepTStepItem::applyPNScaling(double sign, double scale, double hold, double holdLeak) {
    x0 = sign*(x0-hold)/scale+holdLeak;
    xStep = sign*xStep/scale;
}

ProtocolXHoldItem::ProtocolXHoldItem(ProtocolDropItem * item, int nextItem) :
    ProtocolXStepTStepItem(item, 0.0, nextItem) {

}

ProtocolXHoldItem::~ProtocolXHoldItem() {

}

ProtocolXRestItem::ProtocolXRestItem(double x0, double t0, int nextItem) :
    ProtocolXStepTStepItem(x0, 0.0, t0, 0.0, ProtocolItemXStepTStep, nextItem, false) {
}

ProtocolXRestItem::~ProtocolXRestItem() {

}

ProtocolXRampItem::ProtocolXRampItem(ProtocolDropItem * item, double hold, int nextItem) :
    ProtocolItem(ProtocolItemXRamp, nextItem, item->hasVisibleData()) {
    ProtocolDropXRampItem * castItem = static_cast <ProtocolDropXRampItem *> (item);
    x0 = castItem->getX0()+hold;
    xFinal = castItem->getXFinal()+hold;
    t0 = castItem->getT0();
    stimHalfFlag = castItem->getStimHalf();
}

ProtocolXRampItem::ProtocolXRampItem(ProtocolItem * item, int nextItem) :
    ProtocolItem(item->type, nextItem, item->repsNum, item->applySteps, item->visible, item->stimHalfFlag) {
    ProtocolXRampItem * castItem = static_cast <ProtocolXRampItem *> (item);
    x0 = castItem->x0;
    xFinal = castItem->xFinal;
    t0 = castItem->t0;
}

ProtocolXRampItem::ProtocolXRampItem(ProtocolItem * item) :
    ProtocolItem(item->type, item->nextItem, item->repsNum, item->applySteps, item->visible, item->stimHalfFlag) {
    ProtocolXRampItem * castItem = static_cast <ProtocolXRampItem *> (item);
    x0 = castItem->x0;
    xFinal = castItem->xFinal;
    t0 = castItem->t0;
}

ProtocolXRampItem::~ProtocolXRampItem() {

}

Measurement_t ProtocolXRampItem::duration(bool withHidden) {
    Measurement_t one = ONE_MS;
    return one*((visible | withHidden) ? t0 : 0.0);
}

void ProtocolXRampItem::applyPNScaling(double sign, double scale, double hold, double holdLeak) {
    x0 = sign*(x0-hold)/scale+holdLeak;
    xFinal = sign*(xFinal-hold)/scale+holdLeak;
}

ProtocolXSinItem::ProtocolXSinItem(ProtocolDropItem * item, double hold, int nextItem) :
    ProtocolItem(ProtocolItemXSin, nextItem, item->hasVisibleData()) {
    ProtocolDropXSinItem * castItem = static_cast <ProtocolDropXSinItem *> (item);
    x0 = castItem->getX0()+hold;
    xAmp = castItem->getXAmp();
    freq = castItem->getFreq();
    stimHalfFlag = castItem->getStimHalf();
}

ProtocolXSinItem::ProtocolXSinItem(ProtocolItem * item, int nextItem) :
    ProtocolItem(item->type, nextItem, item->repsNum, item->applySteps, item->visible, item->stimHalfFlag) {
    ProtocolXSinItem * castItem = static_cast <ProtocolXSinItem *> (item);
    x0 = castItem->x0;
    xAmp = castItem->xAmp;
    freq = castItem->freq;
}

ProtocolXSinItem::ProtocolXSinItem(ProtocolItem * item) :
    ProtocolItem(item->type, item->nextItem, item->repsNum, item->applySteps, item->visible, item->stimHalfFlag) {
    ProtocolXSinItem * castItem = static_cast <ProtocolXSinItem *> (item);
    x0 = castItem->x0;
    xAmp = castItem->xAmp;
    freq = castItem->freq;
}

ProtocolXSinItem::~ProtocolXSinItem() {

}

Measurement_t ProtocolXSinItem::duration(bool withHidden) {
    Measurement_t one = ONE_MS;
    return one*((visible | withHidden) ? 1.0/freq : 0.0);
}

void ProtocolXSinItem::applyPNScaling(double sign, double scale, double hold, double holdLeak) {
    x0 = sign*(x0-hold)/scale+holdLeak;
    xAmp = sign*xAmp/scale;
}
