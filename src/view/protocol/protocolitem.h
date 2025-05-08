#ifndef PROTOCOLITEM_H
#define PROTOCOLITEM_H

#include "protocoldropitem.h"

typedef enum {
    ProtocolItemXStepTStep,
    ProtocolItemXRamp,
    ProtocolItemXSin
} ProtocolItemType_t;

class ProtocolItem {
public:
    ProtocolItem(ProtocolItemType_t type, int nextItem, int repsNum, bool applySteps, bool visible, bool stimHalfFlag);
    ProtocolItem(ProtocolItemType_t type, int nextItem, bool visible);
    virtual ~ProtocolItem();

    virtual Measurement_t duration(bool withHidden = false) = 0;
    virtual Measurement_t duration(int repsIdx, bool withHidden = false);
    virtual Measurement_t maxDuration(int stepsNum, bool withHidden = false);
    virtual Measurement_t totalDuration(int stepsNum, bool withHidden = false);
    virtual void applyPNScaling(double sign, double scale, double hold, double holdLeak) = 0;

    ProtocolItemType_t type;
    int nextItem = 0;
    int repsNum = 1;
    bool applySteps = false;
    bool visible = true;
    bool stimHalfFlag = false;
};

class ProtocolXStepTStepItem : public ProtocolItem {
public:
    ProtocolXStepTStepItem(ProtocolDropItem * item, double hold, int nextItem);
    ProtocolXStepTStepItem(ProtocolItem * item, int nextItem);
    ProtocolXStepTStepItem(ProtocolItem * item);
    ProtocolXStepTStepItem(double v0, double vStep, double t0, double tStep,
                           ProtocolItemType_t type, int nextItem, bool visible);
    virtual ~ProtocolXStepTStepItem();

    virtual Measurement_t duration(bool withHidden = false) override;
    virtual Measurement_t duration(int repsIdx, bool withHidden = false) override;
    virtual Measurement_t maxDuration(int stepsNum, bool withHidden = false) override;
    virtual Measurement_t totalDuration(int stepsNum, bool withHidden = false) override;
    virtual void applyPNScaling(double sign, double scale, double hold, double holdLeak) override;

    double x0 = 0.0;
    double xStep = 0.0;
    double t0 = 0.0;
    double tStep = 0.0;

};

class ProtocolXHoldItem : public ProtocolXStepTStepItem {
public:
    ProtocolXHoldItem(ProtocolDropItem * item, int nextItem);
    virtual ~ProtocolXHoldItem();
};

class ProtocolXRestItem : public ProtocolXStepTStepItem {
public:
    ProtocolXRestItem(double x0, double t0, int nextItem);
    virtual ~ProtocolXRestItem();
};

class ProtocolXRampItem : public ProtocolItem {
public:
    ProtocolXRampItem(ProtocolDropItem * item, double hold, int nextItem);
    ProtocolXRampItem(ProtocolItem * item, int nextItem);
    ProtocolXRampItem(ProtocolItem * item);
    virtual ~ProtocolXRampItem();

    virtual Measurement_t duration(bool withHidden = false) override;
    virtual void applyPNScaling(double sign, double scale, double hold, double holdLeak) override;

    double x0 = 0.0;
    double xFinal = 0.0;
    double t0 = 0.0;
};

class ProtocolXSinItem : public ProtocolItem {
public:
    ProtocolXSinItem(ProtocolDropItem * item, double hold, int nextItem);
    ProtocolXSinItem(ProtocolItem * item, int nextItem);
    ProtocolXSinItem(ProtocolItem * item);
    virtual ~ProtocolXSinItem();

    virtual Measurement_t duration(bool withHidden = false) override;
    virtual void applyPNScaling(double sign, double scale, double hold, double holdLeak) override;

    double x0 = 0.0;
    double xAmp = 0.0;
    double freq = 0.0;
};

#endif // PROTOCOLITEM_H
