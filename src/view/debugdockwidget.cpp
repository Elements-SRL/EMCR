#include "debugdockwidget.h"

#include <QLayout>
#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QPushButton>

DebugDockWidget::DebugDockWidget(QWidget * parent) :
    QDockWidget(parent) {

    this->setObjectName("debugDw");
    this->setWindowTitle("Debug");

    QWidget * debugWid = new QWidget;
    this->setWidget(debugWid);

    QVBoxLayout * debugVl = new QVBoxLayout;
    debugWid->setLayout(debugVl);

    debugVl->addWidget(new QLabel("Word"));
    QHBoxLayout * debugWordHl = new QHBoxLayout;
    debugVl->addLayout(debugWordHl);

    QSpinBox * debugWordSbx = new QSpinBox;
    debugWordSbx->setRange(0, 32767);
    debugWordSbx->setValue(0);
    debugWordHl->addWidget(debugWordSbx);

    QCheckBox * debugRangeWordChx = new QCheckBox("Up to");
    debugWordHl->addWidget(debugRangeWordChx);

    QSpinBox * debugLastWordSbx = new QSpinBox;
    debugLastWordSbx->setRange(0, 32767);
    debugLastWordSbx->setValue(0);
    debugLastWordSbx->setEnabled(false);
    debugWordHl->addWidget(debugLastWordSbx);

    connect(debugRangeWordChx, &QCheckBox::clicked, debugLastWordSbx, &QWidget::setEnabled);

    debugVl->addWidget(new QLabel("Bit"));
    QHBoxLayout * debugBitHl = new QHBoxLayout;
    debugVl->addLayout(debugBitHl);

    QSpinBox * debugBitSbx = new QSpinBox;
    debugBitSbx->setRange(0, 15);
    debugBitSbx->setValue(0);
    debugBitHl->addWidget(debugBitSbx);

    QCheckBox * debugRangeBitChx = new QCheckBox("Down to");
    debugBitHl->addWidget(debugRangeBitChx);

    QSpinBox * debugFirstBitSbx = new QSpinBox;
    debugFirstBitSbx->setRange(0, 15);
    debugFirstBitSbx->setValue(0);
    debugFirstBitSbx->setEnabled(false);
    debugBitHl->addWidget(debugFirstBitSbx);

    connect(debugRangeBitChx, &QCheckBox::clicked, debugFirstBitSbx, &QWidget::setEnabled);

    QHBoxLayout * debugBitSetHl = new QHBoxLayout;
    debugVl->addLayout(debugBitSetHl);

    QPushButton * debugResetBitBtn = new QPushButton("RESET bit");
    debugResetBitBtn->setCheckable(false);
    debugBitSetHl->addWidget(debugResetBitBtn);

    QPushButton * debugSetBitBtn = new QPushButton("SET bit");
    debugSetBitBtn->setCheckable(false);
    debugBitSetHl->addWidget(debugSetBitBtn);

    connect(debugResetBitBtn, &QPushButton::clicked, this, [=] () {
        if (debugRangeWordChx->isChecked()) {
            for (int wordIdx = debugWordSbx->value(); wordIdx <= debugLastWordSbx->value(); wordIdx++) {
                if (debugRangeBitChx->isChecked()) {
                    for (int bitIdx = debugFirstBitSbx->value(); bitIdx <= debugBitSbx->value(); bitIdx++) {
                        emit setDebugBit(wordIdx, bitIdx, false);
                    }
                }
                else {
                    emit setDebugBit(wordIdx, debugBitSbx->value(), false);
                }
            }
        }
        else {
            if (debugRangeBitChx->isChecked()) {
                for (int bitIdx = debugFirstBitSbx->value(); bitIdx <= debugBitSbx->value(); bitIdx++) {
                    emit setDebugBit(debugWordSbx->value(), bitIdx, false);
                }
            }
            else {
                emit setDebugBit(debugWordSbx->value(), debugBitSbx->value(), false);
            }
        }
    });

    connect(debugSetBitBtn, &QPushButton::clicked, this, [=] () {
        if (debugRangeWordChx->isChecked()) {
            for (int wordIdx = debugWordSbx->value(); wordIdx <= debugLastWordSbx->value(); wordIdx++) {
                if (debugRangeBitChx->isChecked()) {
                    for (int bitIdx = debugFirstBitSbx->value(); bitIdx <= debugBitSbx->value(); bitIdx++) {
                        emit setDebugBit(wordIdx, bitIdx, true);
                    }
                }
                else {
                    emit setDebugBit(wordIdx, debugBitSbx->value(), true);
                }
            }
        }
        else {
            if (debugRangeBitChx->isChecked()) {
                for (int bitIdx = debugFirstBitSbx->value(); bitIdx <= debugBitSbx->value(); bitIdx++) {
                    emit setDebugBit(debugWordSbx->value(), bitIdx, true);
                }
            }
            else {
                emit setDebugBit(debugWordSbx->value(), debugBitSbx->value(), true);
            }
        }
    });

    debugVl->addWidget(new QLabel("Value"));
    QHBoxLayout * debugValueSetHl = new QHBoxLayout;
    debugVl->addLayout(debugValueSetHl);

    QSpinBox * debugValueSbx = new QSpinBox;
    debugValueSbx->setRange(0, 65535);
    debugValueSbx->setValue(0);
    debugValueSetHl->addWidget(debugValueSbx);

    QLabel * debugValueHexLbl = new QLabel("0x0000");
    debugValueSetHl->addWidget(debugValueHexLbl);
    connect(debugValueSbx, QOverload <int> ::of (&QSpinBox::valueChanged), this, [=] (int value) {
        debugValueHexLbl->setText(QString("0x%1").arg(value, 4, 16, QLatin1Char('0')));
    });

    QPushButton * debugApplyValueBtn = new QPushButton("SET value");
    debugApplyValueBtn->setCheckable(false);
    debugValueSetHl->addWidget(debugApplyValueBtn);

    connect(debugApplyValueBtn, &QPushButton::clicked, this, [=] () {
        if (debugRangeWordChx->isChecked()) {
            for (int wordIdx = debugWordSbx->value(); wordIdx <= debugLastWordSbx->value(); wordIdx++) {
                emit setDebugWord(wordIdx, debugValueSbx->value());
            }

        } else {
            emit setDebugWord(debugWordSbx->value(), debugValueSbx->value());
        }
    });

    debugVl->addWidget(new QLabel("Trigger"));
    QHBoxLayout * debugTriggerHl = new QHBoxLayout;
    debugVl->addLayout(debugTriggerHl);

    QSpinBox * debugTriggerSbx = new QSpinBox;
    debugTriggerSbx->setRange(0, 32767);
    debugTriggerSbx->setValue(0);
    debugTriggerHl->addWidget(debugTriggerSbx);

    QPushButton * debugTriggerBtn = new QPushButton("TRIGGER");
    debugTriggerBtn->setCheckable(false);
    debugTriggerHl->addWidget(debugTriggerBtn);

    connect(debugTriggerBtn, &QPushButton::clicked, this, [=] () {
        emit setDebugTrigger(debugTriggerSbx->value());
    });

    debugVl->addWidget(new QLabel("Eeprom Address"));
    QSpinBox * debugEepromAddressSbx = new QSpinBox;
    debugEepromAddressSbx->setRange(0, 2046);
    debugEepromAddressSbx->setValue(0);
    debugVl->addWidget(debugEepromAddressSbx);

    debugVl->addWidget(new QLabel("Eeprom Value"));
    QSpinBox * debugEepromValueSbx = new QSpinBox;
    debugEepromValueSbx->setRange(0, 65535);
    debugEepromValueSbx->setValue(0);
    debugVl->addWidget(debugEepromValueSbx);

    QPushButton * debugEepromWriteBtn = new QPushButton("Eeprom write");
    debugEepromWriteBtn->setCheckable(false);
    debugVl->addWidget(debugEepromWriteBtn);

    connect(debugEepromWriteBtn, &QPushButton::clicked, this, [=] () {
        std::vector <uint32_t> value(1, debugEepromValueSbx->value());
        std::vector <uint32_t> address(1, debugEepromAddressSbx->value());
        std::vector <uint32_t> size(1, 2);
        emit sigWriteCalibrationEeprom(value, address, size);
    });

    // QPushButton * debugEepromReadBtn = new QPushButton("Eeprom read");
    // debugEepromReadBtn->setCheckable(false);
    // debugVl->addWidget(debugEepromReadBtn);

    // connect(debugEepromReadBtn, &QPushButton::clicked, this, [=] () {
    //     std::vector <uint32_t> value(1);
    //     std::vector <uint32_t> address(1, debugEepromAddressSbx->value());
    //     std::vector <uint32_t> size(1, 2);
    //     er4cl::readCalibrationEeprom(value, address, size);
    //     debugEepromValueSbx->setValue(value[0]);
    // });

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    debugVl->addWidget(spacer);
}
