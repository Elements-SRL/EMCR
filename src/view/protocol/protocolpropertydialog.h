#ifndef PROTOCOLPROPERTYDIALOG_H
#define PROTOCOLPROPERTYDIALOG_H

#include <QWidget>
#include <QBoxLayout>

#include "protocolpreview.h"

#define PPD_MAX_PARAMS 32
#define PPD_NAME_ROW 0
#define PPD_HOLD_ROW (PPD_NAME_ROW+1)
#define PPD_HOLD_REF_ROW (PPD_HOLD_ROW+1)
#define PPD_SWEEPS_ROW (PPD_HOLD_REF_ROW+1)
#define PPD_CURRENT_RANGE_ROW (PPD_SWEEPS_ROW+1)
#define PPD_VOLTAGE_RANGE_ROW (PPD_CURRENT_RANGE_ROW+1)
#define PPD_SAMPLING_RATE_ROW (PPD_VOLTAGE_RANGE_ROW+1)
#define PPD_CTRL_FIRST_ROW (PPD_SAMPLING_RATE_ROW+1)
#define PPD_SPACER_ROW (PPD_CTRL_FIRST_ROW+PPD_MAX_PARAMS)
#define PPD_OK_CANC_BTN_ROW (PPD_SPACER_ROW+1)
#define PPD_LABEL_COLUMN 0
#define PPD_EDIT_COLUMN (PPD_LABEL_COLUMN+1)
#define PPD_UNIT_COLUMN (PPD_EDIT_COLUMN+1)
#define PPD_COLUMNS_NUM (PPD_UNIT_COLUMN+1)
#define PPD_HOLD_DISPATCHER_IDX 0
#define PPD_HOLD_REF_DISPATCHER_IDX (PPD_HOLD_DISPATCHER_IDX+1)
#define PPD_SWEEPSNUM_DISPATCHER_IDX (PPD_HOLD_REF_DISPATCHER_IDX+1)
#define PPD_CURRENT_RANGE_DISPATCHER_IDX (PPD_SWEEPSNUM_DISPATCHER_IDX+1)
#define PPD_VOLTAGE_RANGE_DISPATCHER_IDX (PPD_CURRENT_RANGE_DISPATCHER_IDX+1)
#define PPD_SAMPLING_RATE_DISPATCHER_IDX (PPD_VOLTAGE_RANGE_DISPATCHER_IDX+1)
#define PPD_CTRL_DISPATCHER_IDX (PPD_SAMPLING_RATE_DISPATCHER_IDX+1)

class ProtocolWidget;

class ProtocolPropertyDialog : public QWidget {
public:
    ProtocolPropertyDialog(e4gCommLib::CommLib * commLib, e4gcl::RangedMeasurement_t timeRange, e4gcl::RangedMeasurement_t stimulusRange);
    ~ProtocolPropertyDialog();

    QGridLayout * getControlsLayout();
    QHBoxLayout * getButtonsLayout();
    ProtocolPreview * getProtocolPreview();
    bool isOpened(QString &protocolName);
    void setOwner(ProtocolWidget * owner);
    ProtocolWidget * getOwner();

private:
    QGridLayout * controlsLo;
    QHBoxLayout * buttonsHl;
    ProtocolPreview * preview = nullptr;
    ProtocolWidget * ownerPtr = nullptr;
};

#endif // PROTOCOLPROPERTYDIALOG_H
