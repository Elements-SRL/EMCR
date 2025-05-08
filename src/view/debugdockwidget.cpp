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

    QCheckBox * debugRangeWordChx = new QCheckBox("Range");
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

    QCheckBox * debugRangeBitChx = new QCheckBox("Range");
    debugBitHl->addWidget(debugRangeBitChx);

    QSpinBox * debugLastBitSbx = new QSpinBox;
    debugLastBitSbx->setRange(0, 15);
    debugLastBitSbx->setValue(0);
    debugLastBitSbx->setEnabled(false);
    debugBitHl->addWidget(debugLastBitSbx);

    connect(debugRangeBitChx, &QCheckBox::clicked, debugLastBitSbx, &QWidget::setEnabled);

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
                    for (int bitIdx = debugBitSbx->value(); bitIdx <= debugLastBitSbx->value(); bitIdx++) {
                        emit setDebugBit(wordIdx, bitIdx, false);
                    }

                } else {
                    emit setDebugBit(wordIdx, debugBitSbx->value(), false);
                }
            }

        } else {
            if (debugRangeBitChx->isChecked()) {
                for (int bitIdx = debugBitSbx->value(); bitIdx <= debugLastBitSbx->value(); bitIdx++) {
                    emit setDebugBit(debugWordSbx->value(), bitIdx, false);
                }

            } else {
                emit setDebugBit(debugWordSbx->value(), debugBitSbx->value(), false);
            }
        }
    });

    connect(debugSetBitBtn, &QPushButton::clicked, this, [=] () {
        if (debugRangeWordChx->isChecked()) {
            for (int wordIdx = debugWordSbx->value(); wordIdx <= debugLastWordSbx->value(); wordIdx++) {
                if (debugRangeBitChx->isChecked()) {
                    for (int bitIdx = debugBitSbx->value(); bitIdx <= debugLastBitSbx->value(); bitIdx++) {
                        emit setDebugBit(wordIdx, bitIdx, true);
                    }

                } else {
                    emit setDebugBit(wordIdx, debugBitSbx->value(), true);
                }
            }

        } else {
            if (debugRangeBitChx->isChecked()) {
                for (int bitIdx = debugBitSbx->value(); bitIdx <= debugLastBitSbx->value(); bitIdx++) {
                    emit setDebugBit(debugWordSbx->value(), bitIdx, true);
                }

            } else {
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

    QWidget * spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    debugVl->addWidget(spacer);
}
