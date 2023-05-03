#include "compensationcontroldockwidget.h"
#include "messagedispatcher.h"

#include <vector>
#include <QScrollBar>
#include <QScrollArea>
#include <QLabel>
#include <QPushButton>

CompensationControlDockWidget::CompensationControlDockWidget(ModelDevice * mDev, QWidget * parent) :
    QDockWidget(parent),
    mDev(mDev) {
    int localNumOfVoltChans;
    int localNumOfCurrChans;
    mDev->getChannelsNumberFeatures(localNumOfVoltChans, localNumOfCurrChans);

    bool anyControlFlag = false;

    QWidget * bigMainWg = new QWidget();
    bigMainWg->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    this->setWindowTitle("Compensation controls");

    QWidget * mainWg = new QWidget();

    this->setWidget(bigMainWg);
    QVBoxLayout * vLayout = new QVBoxLayout(bigMainWg);
    vLayout->setContentsMargins(0, 0, 0, 2);
    vLayout->setSpacing(1);
    vLayout->addWidget(mainWg);

    QGridLayout * mainGridLayout = this->getLayoutWithScrollBar(mainWg);

    for(int i = 1; i <= localNumOfCurrChans; i++){
        this->hbsCfast.push_back(new QHBoxLayout());
        this->hbsCslowRs.push_back(new QHBoxLayout());
        this->hbsRsCp.push_back(new QHBoxLayout());
        this->hbsRsPg.push_back(new QHBoxLayout());
        hbsCfast[i-1]->setContentsMargins(0,0,0,0);
        hbsCfast[i-1]->setSpacing(1);
        hbsCslowRs[i-1]->setContentsMargins(0,0,0,0);
        hbsCslowRs[i-1]->setSpacing(1);
        hbsRsCp[i-1]->setContentsMargins(0,0,0,0);
        hbsRsCp[i-1]->setSpacing(1);
        hbsRsPg[i-1]->setContentsMargins(0,0,0,0);
        hbsRsPg[i-1]->setSpacing(1);
    }

    /*! Cfast checkboxes and spinboxes*/
    RangedMeasurement_t compensationFeatures;
    if (mDev->getCompFeatures(0, MessageDispatcher::U_CpVc, compensationFeatures) == Success) {
        anyControlFlag = true;
        QCheckBox* cfastEnableCb;
        for(int i = 1; i <= localNumOfCurrChans; i++){
            cfastEnableCb = new QCheckBox();
            cfastEnableCb->setCheckable(true);
            cfastEnableCb->setChecked(false);
            this->cfastCheckBoxes.push_back(cfastEnableCb);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsCfast[i-1]->addWidget(spazietto);
            hbsCfast[i-1]->addWidget(cfastEnableCb);
        }

        MySpinBox* cfastSpinBox;
        QString cfastUnit = QString().fromStdString(compensationFeatures.getFullUnit());
        for(int i = 1; i <= localNumOfCurrChans; i++){
            cfastSpinBox = new MySpinBox();
            cfastSpinBox->setSuffix(QString(" ") + cfastUnit);
            cfastSpinBox->setRange(compensationFeatures.min, compensationFeatures.max);
            cfastSpinBox->setValue(0.0);
            cfastSpinBox->setDecimals(compensationFeatures.decimals());
            this->cfastSpinBoxes.push_back(cfastSpinBox);
            hbsCfast[i-1]->addWidget(cfastSpinBox);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsCfast[i-1]->addWidget(spazietto);
        }
    }

    /*! Cslow and Rs checkboxes and spinboxes*/
    if (mDev->getCompFeatures(0, MessageDispatcher::U_Cm, compensationFeatures) == Success && mDev->getCompFeatures(0, MessageDispatcher::U_Rs, compensationFeatures) == Success) {
        anyControlFlag = true;
        QCheckBox* cslowRsEnableCb;
        for(int i = 1; i <= localNumOfCurrChans; i++){
            cslowRsEnableCb = new QCheckBox();
            cslowRsEnableCb->setCheckable(true);
            cslowRsEnableCb->setChecked(false);
            this->cslowRsCheckBoxes.push_back(cslowRsEnableCb);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsCslowRs[i-1]->addWidget(spazietto);
            hbsCslowRs[i-1]->addWidget(cslowRsEnableCb);
        }

        MySpinBox* cslowSpinBox;
        QString cslowUnit = QString().fromStdString(compensationFeatures.getFullUnit());
        for(int i = 1; i <= localNumOfCurrChans; i++){
            cslowSpinBox = new MySpinBox();
            cslowSpinBox->setSuffix(QString(" ") + cslowUnit);
            cslowSpinBox->setRange(compensationFeatures.min, compensationFeatures.max);
            cslowSpinBox->setValue(0.0);
            cslowSpinBox->setDecimals(compensationFeatures.decimals());
            this->cslowSpinBoxes.push_back(cslowSpinBox);
            hbsCslowRs[i-1]->addWidget(cslowSpinBox);
        }

        MySpinBox* rsSpinBox;
        QString rsUnit = QString().fromStdString(compensationFeatures.getFullUnit());
        for(int i = 1; i <= localNumOfCurrChans; i++){
            rsSpinBox = new MySpinBox();
            rsSpinBox->setSuffix(QString(" ") + rsUnit);
            rsSpinBox->setRange(compensationFeatures.min, compensationFeatures.max);
            rsSpinBox->setValue(0.0);
            rsSpinBox->setDecimals(compensationFeatures.decimals());
            this->rsSpinBoxes.push_back(rsSpinBox);
            hbsCslowRs[i-1]->addWidget(rsSpinBox);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsCslowRs[i-1]->addWidget(spazietto);
        }
    }

    /*! RsCp checkboxes and spinboxes*/
    if (mDev->getCompFeatures(0, MessageDispatcher::U_RsCp, compensationFeatures) == Success) {
        anyControlFlag = true;
        QCheckBox* rsCpEnableCb;
        for(int i = 1; i <= localNumOfCurrChans; i++){
            rsCpEnableCb = new QCheckBox();
            rsCpEnableCb->setCheckable(true);
            rsCpEnableCb->setChecked(false);
            this->rsCpCheckBoxes.push_back(rsCpEnableCb);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsRsCp[i-1]->addWidget(spazietto);
            hbsRsCp[i-1]->addWidget(rsCpEnableCb);
        }

        MySpinBox* rsCpSpinBox;
        QString rsCpUnit = QString().fromStdString(compensationFeatures.getFullUnit());
        for(int i = 1; i <= localNumOfCurrChans; i++){
            rsCpSpinBox = new MySpinBox();
            rsCpSpinBox->setSuffix(QString(" ") + rsCpUnit);
            rsCpSpinBox->setRange(compensationFeatures.min, compensationFeatures.max);
            rsCpSpinBox->setValue(0.0);
            rsCpSpinBox->setDecimals(compensationFeatures.decimals());
            this->rsCpSpinBoxes.push_back(rsCpSpinBox);
            hbsRsCp[i-1]->addWidget(rsCpSpinBox);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsRsCp[i-1]->addWidget(spazietto);
        }
    }

    /*! RsPg checkboxes and spinboxes*/
    if (mDev->getCompFeatures(0, MessageDispatcher::U_RsPg, compensationFeatures) == Success) {
        anyControlFlag = true;

        QCheckBox* rsPgEnableCb;
        for(int i = 1; i <= localNumOfCurrChans; i++){
            rsPgEnableCb = new QCheckBox();
            rsPgEnableCb->setCheckable(true);
            rsPgEnableCb->setChecked(false);
            this->rsPgCheckBoxes.push_back(rsPgEnableCb);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsRsPg[i-1]->addWidget(spazietto);
            hbsRsPg[i-1]->addWidget(rsPgEnableCb);
        }

        MySpinBox* rsPgSpinBox;
        QString rsPgUnit = QString().fromStdString(compensationFeatures.getFullUnit());
        for(int i = 1; i <= localNumOfCurrChans; i++){
            rsPgSpinBox = new MySpinBox();
            rsPgSpinBox->setSuffix(QString(" ") + rsPgUnit);
            rsPgSpinBox->setRange(compensationFeatures.min, compensationFeatures.max);
            rsPgSpinBox->setValue(0.0);
            rsPgSpinBox->setDecimals(compensationFeatures.decimals());
            this->rsPgSpinBoxes.push_back(rsPgSpinBox);
            hbsRsPg[i-1]->addWidget(rsPgSpinBox);
            QWidget* spazietto = new QWidget();
            spazietto->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
            hbsRsPg[i-1]->addWidget(spazietto);
        }
    }

    // Column captions
    mainGridLayout->addWidget(new QLabel("Ch"), 0, 0, Qt::AlignHCenter);
//    mainGridLayout->addWidget(new QLabel(""), 0, 1);
    mainGridLayout->addWidget(new QLabel("Cfast"), 0, 1, Qt::AlignHCenter);
//    mainGridLayout->addWidget(new QLabel("En"), 0, 3);
    mainGridLayout->addWidget(new QLabel("Cslow - Rs"), 0, 2, Qt::AlignHCenter);
//    mainGridLayout->addWidget(new QLabel("Rs"), 0, 5);
//    mainGridLayout->addWidget(new QLabel("En"), 0, 6);
    mainGridLayout->addWidget(new QLabel("RsCorr"), 0, 3, Qt::AlignHCenter);
//    mainGridLayout->addWidget(new QLabel("En"), 0, 8);
    mainGridLayout->addWidget(new QLabel("RsPred"), 0, 4, Qt::AlignHCenter);
    mainGridLayout->addWidget(new QLabel("RsBW"), 0, 5, Qt::AlignHCenter);

    for(int i = 1; i <= localNumOfCurrChans; i++){
        QString channelLabel= QString("    %1").arg(i);
        mainGridLayout->addWidget(new QLabel(channelLabel), i, 0);
        mainGridLayout->addLayout(hbsCfast[i-1], i, 1);
        mainGridLayout->addLayout(hbsCslowRs[i-1], i, 2);
        mainGridLayout->addLayout(hbsRsCp[i-1], i, 3);
        mainGridLayout->addLayout(hbsRsPg[i-1], i, 4);
    }

    /*! RsBW comboboxes*/
    std::vector <std::string> rsBwStringArray;
    if (mDev->getCompOptionsFeatures(MessageDispatcher::CompRsCorr, rsBwStringArray) == Success) {
        anyControlFlag = true;
        QComboBox* rsBwCb;
        for(int i = 1; i <= localNumOfCurrChans; i++){
            rsBwCb = new QComboBox();
            for(int j = 0; j < rsBwStringArray.size(); j++){
                rsBwCb->addItem(QString::fromStdString(rsBwStringArray[j]));
            }
            this->rsBwComboBoxes.push_back(rsBwCb);
            mainGridLayout->addWidget(rsBwCb,i, 5);
        }
    }

    QPushButton* applyButton = new QPushButton("Apply");
    vLayout->addWidget(applyButton);
    connect(applyButton, &QPushButton::clicked, this, &CompensationControlDockWidget::onApplyButtonClicked);

    if (!anyControlFlag) {
        this->setEnabled(false);
        this->setVisible(false);
    }
}

QGridLayout * CompensationControlDockWidget::getLayoutWithScrollBar(QWidget * widget) {
    QVBoxLayout * vl = new QVBoxLayout;
    vl->setContentsMargins(0, 0, 0, 0);
    vl->setSpacing(1);
    widget->setLayout(vl);

    QScrollArea * scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    vl->addWidget(scrollArea);

    QWidget * scrollWg = new QWidget;
    scrollArea->setWidget(scrollWg);

    QGridLayout * scrollHl = new QGridLayout;
    scrollHl->setContentsMargins(0, 0, 20, 0);
    scrollHl->setSpacing(1);
    scrollWg->setLayout(scrollHl);

    return scrollHl;
}

void CompensationControlDockWidget::onApplyButtonClicked(){
    vector<uint16_t> channelIndexes;
    vector<bool> cfastEn;
    vector<bool> cslowRsEn;
    vector<bool> rsCpEn;
    vector<bool> rsPgEn;

    int localNumOfVoltChans;
    int localNumOfCurrChans;
    mDev->getChannelsNumberFeatures(localNumOfVoltChans, localNumOfCurrChans);
    for(int i = 0; i < localNumOfCurrChans; i++){
        channelIndexes.push_back(i);
        cfastEn.push_back(cfastCheckBoxes[i]->isChecked());
        cslowRsEn.push_back(cslowRsCheckBoxes[i]->isChecked());
        rsCpEn.push_back(rsCpCheckBoxes[i]->isChecked());
        rsPgEn.push_back(rsPgCheckBoxes[i]->isChecked());
    }

    emit sigCompensationsApplied(channelIndexes, cfastEn, cslowRsEn, rsCpEn, rsPgEn);

}
