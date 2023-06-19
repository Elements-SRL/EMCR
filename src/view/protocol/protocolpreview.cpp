#include "protocolpreview.h"

#include <QBoxLayout>

#include "protocolwidget.h"

static double sinTable[PPW_MAX_PTS_PER_ITEM];

ProtocolPreview::ProtocolPreview(ModelDevice * mDev, RangedMeasurement_t timeRange, RangedMeasurement_t stimulusRange, QString title) :
    QWidget(),
    mDev(mDev),
    timeRange(timeRange) {

    QVBoxLayout * mainVl = new QVBoxLayout();
    mainVl->setContentsMargins(1, 1, 1, 1);
    mainVl->setSpacing(2);
    this->setLayout(mainVl);

    /*! Plot */
    protocolPlot = new ProtocolPlot(mDev, title, QString::fromStdString(timeRange.getFullUnit()), "");
    /*! y unit is set in setStimulusRange */
    protocolPlot->setAxisAutoScale(QwtPlot::yLeft);
    protocolPlot->setAxisAutoScale(QwtPlot::xBottom);

    this->setStimulusRange(stimulusRange);

    errorText = new QwtText("ERROR");
    errorText->setRenderFlags(Qt::AlignHCenter | Qt::AlignVCenter);

    QFont font;
    font.setBold(true);
    errorText->setFont(font);
    errorText->setColor(QColor(Qt::red));

    errorItem = new QwtPlotTextLabel();
    errorItem->attach(protocolPlot);

    errorItem->setVisible(false);

//    mDev->getMaxOutputTriggers(maxOutputTriggers);
    if (!minimal) {
        /*! Cursors */
        cursorsManager = new CursorsManager(protocolPlot, maxOutputTriggers);
    }

    cursorsWid = new QFrame;
    cursorsWid->setFrameStyle(QFrame::Box | QFrame::Sunken);
    cursorsWid->setLineWidth(1);
    cursorsWid->setMidLineWidth(0);
#ifndef GLB_CURSORS_IN_PROTOCOL_EDITOR
    cursorsWid->setVisible(false);
#endif

    QHBoxLayout * cursorsHl = new QHBoxLayout;
    cursorsHl->setContentsMargins(0, 0, 0, 0);
    cursorsHl->setSpacing(2);
    cursorsWid->setLayout(cursorsHl);

    manageCursorBtn = new QPushButton("Manage cursors");
    manageCursorBtn->setCheckable(true);
    manageCursorBtn->setChecked(false);
    cursorsHl->addWidget(manageCursorBtn);

    QLabel * cursorsHelpLbl = new QLabel;
    cursorsHelpLbl->setPixmap(QPixmap(":/imgs/help icon.png").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    cursorsHelpLbl->setFixedSize(26, 26);
    cursorsHelpLbl->setToolTip("Click Manage Cursors to interact with the protocol cursors:\n"
                               "- double click on the plot to create a new cursor\n"
                               "- right click on a cursor to open its properties panel\n"
                               "- left click and drag to move a cursor\n"
                               "- drag a cursor below the plot to delete it (WARNING: this can't be undone)");
    cursorsHl->addWidget(cursorsHelpLbl);

    cursorsWarningLbl = new QLabel;
    cursorsWarningLbl->setPixmap(QPixmap(":/imgs/warning icon.png").scaled(24, 24, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    cursorsWarningLbl->setFixedSize(26, 26);
    if (maxOutputTriggers > 1) {
        cursorsWarningLbl->setToolTip("Too many output digital trigger events set.\n"
                                      "Only the first " + QString::number(maxOutputTriggers-1) + " triggers will be applied.");

    } else {
        cursorsWarningLbl->setToolTip("Output digital trigger events are not available for this device.");
    }
    cursorsWarningLbl->setVisible(false);
    cursorsHl->addWidget(cursorsWarningLbl);

    connect(manageCursorBtn, &QPushButton::clicked, protocolPlot, &ProtocolPlot::onEnableCursorManagement);

    QLabel * addCursorReminderLbl = new QLabel(" Double-click to add cursors ");
    addCursorReminderLbl->setStyleSheet("color:red");
    QFontMetrics fm((QFont(addCursorReminderLbl->font())));
    addCursorReminderLbl->setFixedWidth(fm.width(addCursorReminderLbl->text()));
    addCursorReminderLbl->setText("");
    cursorsHl->addWidget(addCursorReminderLbl);

    connect(manageCursorBtn, &QPushButton::clicked, this, [=] (bool flag) {
        if (flag) {
            addCursorReminderLbl->setText(" Double-click to add cursors ");

        } else {
            addCursorReminderLbl->setText("");
        }
    });

    cursorsWarningPlaceHolderLbl = new QLabel;
    cursorsWarningPlaceHolderLbl->setFixedSize(26, 26);
    /*! If I don't make it invisible before adding it to the layout there's a strange flicker while connecting to the device */
    cursorsWarningPlaceHolderLbl->setVisible(false);
    cursorsHl->addWidget(cursorsWarningPlaceHolderLbl);
    cursorsWarningPlaceHolderLbl->setVisible(true);

    {
        QWidget * spacer = new QWidget();
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        cursorsHl->addWidget(spacer);
    }

    cursorsBinLbl = new QLabel;
    cursonBinClosedPxm = QPixmap(":/imgs/trash bin closed.png").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    cursonBinOpenPxm = QPixmap(":/imgs/trash bin open.png").scaled(30, 30, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    cursorsBinLbl->setPixmap(cursonBinClosedPxm);
    cursorsBinLbl->setFixedSize(QSize(32, 32));
    cursorsBinLbl->setToolTip("Cursors bin: drag cursors here to delete them");
    cursorsHl->addWidget(cursorsBinLbl);

    connect(protocolPlot, &ProtocolPlot::openCursorBin, this, [=] (bool open) {
        if (open) {
            cursorsBinLbl->setPixmap(cursonBinOpenPxm);

        } else {
            cursorsBinLbl->setPixmap(cursonBinClosedPxm);
        }
    });

    {
        QWidget * spacer = new QWidget();
        spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        cursorsHl->addWidget(spacer);
    }

    {
        QWidget * spacer = new QWidget();
        spacer->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        spacer->setFixedWidth(manageCursorBtn->sizeHint().width()+cursorsHelpLbl->width()+cursorsWarningLbl->width()+addCursorReminderLbl->width());
        cursorsHl->addWidget(spacer);
    }

    mainVl->addWidget(protocolPlot);
    mainVl->addWidget(cursorsWid);

    /*! Initialize sin table */
    for (int ptsIdx = 0; ptsIdx < PPW_MAX_PTS_PER_ITEM; ptsIdx++) {
        sinTable[ptsIdx] = sin(((double)ptsIdx)*2.0*M_PI/(double)(PPW_MAX_PTS_PER_ITEM-1));
    }

    mDev->getMessageDispatcher()->getMaxProtocolItemsFeature(maxProtocolItems);

    /*! stimulusUnit is set in setStimulusRange */
    timeUnit = QString::fromStdString(timeRange.getFullUnit());
}

ProtocolPreview::~ProtocolPreview() {
    unsigned int curvesNum = protocolPlotCurve.size();
    for (unsigned int curvesIdx = 0; curvesIdx < curvesNum; curvesIdx++) {
        protocolPlotCurve[curvesIdx]->detach();
        delete protocolPlotData[curvesIdx*2];
        delete protocolPlotData[curvesIdx*2+1];
    }
    protocolPlotCurve.clear();
    protocolPlotData.clear();
}

void ProtocolPreview::setProtocol(ProtocolWidget * protocol) {
    this->protocol = protocol;
    if (!minimal) {
        cursorsManager->setProtocol(protocol);
    }
}

void ProtocolPreview::setStimulusRange(RangedMeasurement_t &range) {
    stimulusRange = range;
    this->setHoldingDelta(holdingDelta);
    stimulusUnit = QString::fromStdString(stimulusRange.getFullUnit());
    protocolPlot->setYUnit(stimulusUnit);
}

void ProtocolPreview::setHoldingDelta(Measurement_t &newHoldingDelta) {
    holdingDelta = newHoldingDelta;
    holdingDelta.convertValue(stimulusRange.prefix);
    if (holdingDelta.value < 0.0) {
        stimulusActiveRange.min = stimulusRange.min-holdingDelta.value;
        stimulusActiveRange.max = stimulusRange.max;

    } else {
        stimulusActiveRange.min = stimulusRange.min;
        stimulusActiveRange.max = stimulusRange.max-holdingDelta.value;
    }
}

void ProtocolPreview::setAnalysisPidl(AnalysisProtocolItemDropList * analysisPidl) {
    cursorsManager->setAnalysisPidl(analysisPidl);
}

void ProtocolPreview::setTooManyTriggersWarning(bool flag) {
    cursorsWarningLbl->setVisible(flag);
    cursorsWarningPlaceHolderLbl->setVisible(!flag);
}

std::vector <YAML::Cursor> ProtocolPreview::getYamlCursors() {
    return protocolPlot->getYamlCursors();
}

void ProtocolPreview::setCursorsFromYaml(const std::vector <YAML::Cursor> &yamlCursors) {
    protocolPlot->setCursorsFromYaml(yamlCursors);
}

void ProtocolPreview::updateView() {
    /*! Clear previous curves */
    unsigned int curvesNum = protocolPlotCurve.size();
    for (unsigned int curvesIdx = 0; curvesIdx < curvesNum; curvesIdx++) {
        protocolPlotCurve[curvesIdx]->detach();
        delete protocolPlotData[curvesIdx*2];
        delete protocolPlotData[curvesIdx*2+1];
        delete protocolPlotCurve[curvesIdx];
    }
    protocolPlotCurve.clear();
    protocolPlotData.clear();

    for (int idx = 0; idx < protocolItems.size(); idx++) {
        delete protocolItems[idx];
    }
    protocolItems.clear();

    /*! Preprocess new protocol */
    ItemsProcStatus_t status = this->interpretProtocolItems(protocol);
    Measurement_t hold = protocol->getHold();
    int sweepsNum = protocol->getSweepsNum();

    if (!minimal) {
        if (status != ItemsProcOk) {
            protocol->pushProtocolItems(protocolItems);
            protocol->pushProcessingStatus(status);
            protocolPlot->setCursorsVisibility(status == ItemsProcOk);
            if ((status != ItemsProcOk) && (manageCursorBtn->isEnabled())) {
                manageCursorBtn->click();
            }
            cursorsManager->enableAnalysis(status == ItemsProcOk);
            protocol->setProtocolValid(status == ItemsProcOk);
        }
    }

    if (status != ItemsProcOk) {
        return;
    }

    /*! Process items */
    CurveType_t newCurveType;
    CurveType_t currentCurveType = CurveTypeNone;
    double lastPtX = 0.0;
    double lastPtY = hold.value;

    curvesNum = 0;
    int ptsNum = 0;
    unsigned int xDataIdx = 0;
    unsigned int yDataIdx = 1;

    double timeOffset = 0.0;
    int protocolItemIdx;
    int repsIdx;
    int loopIdx;
    bool lastRep;
    double stepFactor;
    double minStimulusApplied = std::numeric_limits <double> ::max();
    double maxStimulusApplied = -std::numeric_limits <double> ::max();

    if (!minimal) {
        if (protocolSections != nullptr) {
            for (int sectionIdx = 0; sectionIdx < protocolSections->size(); sectionIdx++) {
                delete protocolSections->at(sectionIdx);
            }
            protocolSections->clear();
            delete protocolSections;
        }
        protocolSections = new ProtocolSections;
    }

    for (int sweepsIdx = 0; sweepsIdx < sweepsNum; sweepsIdx++) {
        timeOffset = 0.0;
        protocolItemIdx = 0;
        repsIdx = 0;
        loopIdx = 0;
        lastRep = false;
        while (protocolItemIdx < protocolItems.size()) {
            ProtocolItem * item = protocolItems[protocolItemIdx];

            newCurveType = (item->visible ?
                                (!lastRep ? CurveTypeProtocolPreviewSolid : CurveTypeProtocolPreviewDashed) :
                                CurveTypeProtocolPreviewDotted);
            if ((currentCurveType != newCurveType) || (PPW_MAX_PLOT_DATA <= ptsNum+PPW_MAX_PTS_PER_ITEM)) {
                if (curvesNum > 0) {
                    protocolPlotCurve.back()->setRawSamples(protocolPlotData[xDataIdx], protocolPlotData[yDataIdx], ptsNum);
                }

                xDataIdx = curvesNum*2;
                yDataIdx = curvesNum*2+1;

                curvesNum++;
                ptsNum = 0;

                currentCurveType = newCurveType;
                Curve * curve = new Curve(currentCurveType, sweepsIdx == 0 ? 2.0 : 1.0);
                protocolPlotCurve.push_back(curve);
                protocolPlotData.push_back(new double[PPW_MAX_PLOT_DATA]); /*!< x */
                protocolPlotData.push_back(new double[PPW_MAX_PLOT_DATA]); /*!< y */

                protocolPlotData[xDataIdx][ptsNum] = lastPtX;
                protocolPlotData[yDataIdx][ptsNum] = lastPtY;

                ptsNum++;

                curve->attach(protocolPlot);
            }

            double startingTime = timeOffset;
            if (item->type == ProtocolItemXStepTStep) {
                stepFactor = (double)(sweepsIdx+(item->applySteps ? repsIdx : 0));

                ProtocolXStepTStepItem * castItem = static_cast <ProtocolXStepTStepItem *> (item);
                double stimulusValue = castItem->x0+stepFactor*castItem->xStep;

                protocolPlotData[xDataIdx][ptsNum] = timeOffset;
                protocolPlotData[yDataIdx][ptsNum] = stimulusValue;
                ptsNum++;
                timeOffset += castItem->t0+stepFactor*castItem->tStep;

                protocolPlotData[xDataIdx][ptsNum] = timeOffset;
                protocolPlotData[yDataIdx][ptsNum] = stimulusValue;
                ptsNum++;

                if (stimulusValue < minStimulusApplied) {
                    minStimulusApplied = stimulusValue;
                }

                if (stimulusValue > maxStimulusApplied) {
                    maxStimulusApplied = stimulusValue;
                }

                if (stimulusValue > stimulusActiveRange.max) {
                    status = ItemsOverStimulus;
                }

                if (stimulusValue < stimulusActiveRange.min) {
                    status = ItemsUnderStimulus;
                }

                if (castItem->t0+stepFactor*castItem->tStep < timeRange.min) {
                    status = ItemsUnderDuration;
                }

            } else if (item->type == ProtocolItemXRamp) {
                ProtocolXRampItem * castItem = static_cast <ProtocolXRampItem *> (item);
                protocolPlotData[xDataIdx][ptsNum] = timeOffset;
                protocolPlotData[yDataIdx][ptsNum] = castItem->x0;
                ptsNum++;
                timeOffset += castItem->t0;

                protocolPlotData[xDataIdx][ptsNum] = timeOffset;
                protocolPlotData[yDataIdx][ptsNum] = castItem->xFinal;
                ptsNum++;

                if (castItem->x0 < minStimulusApplied) {
                    minStimulusApplied = castItem->x0;
                }

                if (castItem->xFinal < minStimulusApplied) {
                    minStimulusApplied = castItem->xFinal;
                }

                if (castItem->x0 > maxStimulusApplied) {
                    maxStimulusApplied = castItem->x0;
                }

                if (castItem->xFinal > maxStimulusApplied) {
                    maxStimulusApplied = castItem->xFinal;
                }

                if ((castItem->xFinal > stimulusActiveRange.max) ||
                        (castItem->x0 > stimulusActiveRange.max)) {
                    status = ItemsOverStimulus;
                }

                if ((castItem->xFinal < stimulusActiveRange.min) ||
                        (castItem->x0 < stimulusActiveRange.min)) {
                    status = ItemsUnderStimulus;
                }

                if (castItem->t0 < timeRange.min) {
                    status = ItemsUnderDuration;
                }

            } else if (item->type == ProtocolItemXSin) {
                ProtocolXSinItem * castItem = static_cast <ProtocolXSinItem *> (item);
                for (int ptsIdx = 0; ptsIdx < PPW_MAX_PTS_PER_ITEM; ptsIdx++) {
                    protocolPlotData[xDataIdx][ptsNum+ptsIdx] = timeOffset+((double)ptsIdx)/castItem->freq/((double)(PPW_MAX_PTS_PER_ITEM-1));
                    protocolPlotData[yDataIdx][ptsNum+ptsIdx] = castItem->x0+castItem->xAmp*sinTable[ptsIdx];
                }
                ptsNum += PPW_MAX_PTS_PER_ITEM;
                timeOffset += 1.0/castItem->freq;

                /*! \todo FCON qui si sta dando per scontato che castItem.xAmp sia positivo, verificare o aggiugnere le altre condizioni qui sotto */

                if (castItem->x0-castItem->xAmp < minStimulusApplied) {
                    minStimulusApplied = castItem->x0-castItem->xAmp;
                }

                if (castItem->x0+castItem->xAmp > maxStimulusApplied) {
                    maxStimulusApplied = castItem->x0+castItem->xAmp;
                }

                if (castItem->x0+castItem->xAmp > stimulusActiveRange.max) {
                    status = ItemsOverStimulus;
                }

                if (castItem->x0-castItem->xAmp < stimulusActiveRange.min) {
                    status = ItemsUnderStimulus;
                }

                if (1.0/castItem->freq < timeRange.min) {
                    status = ItemsUnderDuration;
                }
            }

            if (!minimal) {
                ProtocolSection * protocolSection = new ProtocolSection;
                protocolSection->itemIdx = protocolItemIdx;
                protocolSection->repsIdx = repsIdx;
                protocolSection->repsNum = item->repsNum;
                protocolSection->loopIdx = loopIdx;
                protocolSection->sweepIdx = sweepsIdx;
                protocolSection->sweepsNum = protocol->getSweepsNum();
                protocolSection->applySteps = item->applySteps;
                protocolSection->startingTime = startingTime;
                protocolSection->endingTime = timeOffset;
                protocolSections->appendSection(protocolSection);
            }

            lastPtX = protocolPlotData[xDataIdx][ptsNum-1];
            lastPtY = protocolPlotData[yDataIdx][ptsNum-1];

            if (item->nextItem > protocolItemIdx) {
                /*! No loop terminating item */
                protocolItemIdx = item->nextItem;

            } else {
                /*! Loop terminating item */
                if (((repsIdx+1 < item->repsNum) || (item->repsNum == 0)) && !lastRep) {
                    /*! No last repetition (not last of finite sequence or infinite sequence and not forced last repetition to plot) */
                    protocolItemIdx = item->nextItem;

                    if (minimal) {
                        if (repsIdx < PPW_MAX_REPS) {
                            /*! Limit to repetitions to plot not reached */
                            repsIdx++;

                        } else {
                            /*! Limit to repetitions to plot reached, force last repetition */
                            repsIdx = std::max(item->repsNum-1, 0);
                            lastRep = true;
                        }

                    } else {
                        repsIdx++;
                        if (item->repsNum == 0) {
                            /*! For infinite sequences force last repetition */
                            lastRep = true;
                        }
                    }

                } else {
                    /*! Last repetition (last of finite sequence infinite sequence and forced last repetition to plot) */
                    protocolItemIdx++;
                    repsIdx = 0;
                    loopIdx++;
                    lastRep = false;
                }
            }
        }

        currentCurveType = CurveTypeNone;
        lastPtX = 0.0;
        lastPtY = hold.value;
    }

    if (curvesNum > 0) {
        protocolPlotCurve.back()->setRawSamples(protocolPlotData[xDataIdx], protocolPlotData[yDataIdx], ptsNum);
    }

    if (!minimal) {
        protocol->pushProtocolItems(protocolItems);
        protocol->pushProcessingStatus(status);
        protocolPlot->setCursorsVisibility(status == ItemsProcOk);
        if ((status != ItemsProcOk) && (manageCursorBtn->isEnabled())) {
            manageCursorBtn->click();
        }
        protocol->setProtocolSections(protocolSections);
        protocolPlot->setProtocol(protocol);

        cursorsManager->enableAnalysis(status == ItemsProcOk);
        protocol->setProtocolValid(status == ItemsProcOk);
        RangedMeasurement_t appliedRange;
        appliedRange.min = minStimulusApplied;
        appliedRange.max = maxStimulusApplied;
        appliedRange.prefix = stimulusRange.prefix;
        appliedRange.unit = stimulusRange.unit;
        protocol->setAppliedRange(appliedRange);
    }

    switch (status) {
    case ItemsProcOk:
        errorItem->setVisible(false);
        break;

    case ItemsOverStimulus:
        errorText->setText(QString("Item stimulus above max value: %1 ").arg(stimulusActiveRange.max) + stimulusUnit);
        errorItem->setText(* errorText);
        errorItem->setVisible(true);
        protocolPlot->replot();
        break;

    case ItemsUnderStimulus:
        errorText->setText(QString("Item stimulus below min value: %1 ").arg(stimulusActiveRange.min) + stimulusUnit);
        errorItem->setText(* errorText);
        errorItem->setVisible(true);
        protocolPlot->replot();
        break;

    case ItemsUnderDuration:
        errorText->setText(QString("Item duration below min value: %1 ").arg(timeRange.min) + timeUnit);
        errorItem->setText(* errorText);
        errorItem->setVisible(true);
        protocolPlot->replot();
        break;

    default:
        break;
    }

    protocolPlot->replot();
}

ItemsProcStatus_t ProtocolPreview::interpretProtocolItems(ProtocolWidget * protocol) {
    QVector <ProtocolDropItem *> * dropItems = protocol->getProtocolDropItems();

    Measurement_t hold = protocol->getHold();
    bool holdRef = protocol->getHoldRef();
    double addedHold = (holdRef ? hold.value : 0.0);

    /*! Preprocess protocol items */
    int dropItemNum = dropItems->size();
    ItemsProcStatus_t status = ItemsProcOk;
    int lastSeqItem = 0;
    int protocolItemsNum = 0;
    int totalProtocolItems = dropItemNum;

    /*! \todo FCON questo sarebbe il posto corretto per fare un controllo che siano rispettati i range del protocollo.
                   i loop sono le cose più insidiose e vanno verificate esplicitamente */

    for (int dropItemIdx = 0; dropItemIdx < dropItemNum; dropItemIdx++) {
        if (status != ItemsProcOk) {
            break;
        }

        ProtocolDropItem * item = dropItems->at(dropItemIdx);
        switch (item->type()) {
        case PROT_DROP_LIST_VSTEP_TSTEP_ITEM_TYPE:
        case PROT_DROP_LIST_ISTEP_TSTEP_ITEM_TYPE:
        case PROT_DROP_LIST_VSTEP_ITEM_TYPE:
        case PROT_DROP_LIST_ISTEP_ITEM_TYPE:
        case PROT_DROP_LIST_VTSTEP_ITEM_TYPE:
        case PROT_DROP_LIST_ITSTEP_ITEM_TYPE:
        case PROT_DROP_LIST_VCONST_ITEM_TYPE:
        case PROT_DROP_LIST_ICONST_ITEM_TYPE:
        case PROT_DROP_LIST_VREST_ITEM_TYPE:
        case PROT_DROP_LIST_IREST_ITEM_TYPE: {
            protocolItems.push_back(new ProtocolXStepTStepItem(item, addedHold, ++protocolItemsNum));
            break;
        }

        case PROT_DROP_LIST_VHOLD_ITEM_TYPE:
        case PROT_DROP_LIST_IHOLD_ITEM_TYPE: {
            protocolItems.push_back(new ProtocolXHoldItem(item, ++protocolItemsNum));
            break;
        }

        case PROT_DROP_LIST_VRAMP_ITEM_TYPE:
        case PROT_DROP_LIST_IRAMP_ITEM_TYPE: {
            protocolItems.push_back(new ProtocolXRampItem(item, addedHold, ++protocolItemsNum));
            break;
        }

        case PROT_DROP_LIST_VSIN_ITEM_TYPE:
        case PROT_DROP_LIST_ISIN_ITEM_TYPE: {
            protocolItems.push_back(new ProtocolXSinItem(item, addedHold, ++protocolItemsNum));
            break;
        }

        case PROT_DROP_LIST_VREP_SEQ_SCALED_ITEM_TYPE:
        case PROT_DROP_LIST_IREP_SEQ_SCALED_ITEM_TYPE: {
            ProtocolDropXRepSeqScaledItem * castItem = static_cast <ProtocolDropXRepSeqScaledItem *> (item);
            ProtocolItem * protocolItem;

            double vHoldLeak = castItem->getHoldLeak();
            double scaleFactor = (double)(castItem->getScaleFactor());
            int repsNum = castItem->getRepsNum();
            int itemNum = castItem->getItemNum();
            double restStimulus = castItem->getRestStimulus()+addedHold;
            double restTime = castItem->getRestTime();
            bool preceding = castItem->getPreceding();
            bool reversed = castItem->getReversed();
            bool alternating = castItem->getAlternating();
            /*! applySteps not checked cause it can't be true in P/N */

            int itemOffset = protocolItems.size();
            totalProtocolItems--;

            QVector <ProtocolItem *> repeatingSequence;

            if (itemOffset < itemNum) {
                status = ItemsProcErrorNotEnoughItemsForSequence;

            } else if (itemOffset-itemNum < lastSeqItem) {
                status = ItemsProcErrorOverlappingSequences;

            } else {
                if (restTime != 0.0) {
                    /*! In case of rest time just add an invisible item */
                    protocolItems.push_back(new ProtocolXRestItem(restStimulus, restTime, ++protocolItemsNum));
                    itemOffset++;
                    itemNum++;
                    totalProtocolItems++;
                }

                if (alternating) {
                    /*! in case of alternating polarity both polarities are created so half repetitions are performed */
                    repsNum = (repsNum+1)/2; /*!< ceil rounding */
                }

                int alternatingRepNum = (alternating ? 2 : 1);
                for (int alternatingRepIdx = 0; alternatingRepIdx < alternatingRepNum; alternatingRepIdx++) {
                    double itemSign = (reversed ? -1.0 : 1.0)*(alternatingRepIdx % 2 == 0 ? 1.0 : -1.0);
                    for (int itemIdx = 0; itemIdx < itemNum; itemIdx++) {
                        int sourceItemIdx = itemOffset-itemNum+itemIdx;

                        switch (protocolItems[itemOffset-itemNum+itemIdx]->type) {
                        case ProtocolItemXStepTStep:
                            protocolItem = new ProtocolXStepTStepItem(protocolItems[sourceItemIdx], ++protocolItemsNum);
                            break;

                        case ProtocolItemXRamp:
                            protocolItem = new ProtocolXRampItem(protocolItems[sourceItemIdx], ++protocolItemsNum);
                            break;

                        case ProtocolItemXSin:
                            protocolItem = new ProtocolXSinItem(protocolItems[sourceItemIdx], ++protocolItemsNum);
                            break;
                        }

                        protocolItem->applyPNScaling(itemSign, scaleFactor, hold.value, vHoldLeak);
                        protocolItem->repsNum = (unsigned short)repsNum;

                        repeatingSequence.push_back(protocolItem);
                    }
                }

                /*! Set the last item to enter the loop */
                int repeatingSequenceLen = repeatingSequence.size();

                repeatingSequence.back()->nextItem -= repeatingSequenceLen;

                if (preceding) {
                    int insertion = itemOffset-itemNum;
                    protocolItems = protocolItems.mid(0, insertion)+repeatingSequence+protocolItems.mid(insertion, -1);

                    for (int itemIdx = insertion; itemIdx < insertion+repeatingSequenceLen; itemIdx++) {
                        protocolItems[itemIdx]->nextItem -= itemNum;
                    }

                    for (int itemIdx = insertion+repeatingSequenceLen; itemIdx < protocolItems.size(); itemIdx++) {
                        protocolItems[itemIdx]->nextItem += repeatingSequenceLen;
                    }

                } else {
                    protocolItems.append(repeatingSequence);
                }
            }
            lastSeqItem = protocolItems.size();
            break;
        }

        case PROT_DROP_LIST_VREP_SEQ_ITEM_TYPE:
        case PROT_DROP_LIST_IREP_SEQ_ITEM_TYPE:
        case PROT_DROP_LIST_VREP_SEQ_WITH_STEPS_ITEM_TYPE:
        case PROT_DROP_LIST_IREP_SEQ_WITH_STEPS_ITEM_TYPE:
        case PROT_DROP_LIST_VINF_REP_SEQ_ITEM_TYPE:
        case PROT_DROP_LIST_IINF_REP_SEQ_ITEM_TYPE: {
            ProtocolDropXRepSeqScaledItem * castItem = static_cast <ProtocolDropXRepSeqScaledItem *> (item);

            int repsNum = castItem->getRepsNum(); /*!< Recall that in these cases repsNum is reduced by 1 */
            int itemNum = castItem->getItemNum();
            double restStimulus = castItem->getRestStimulus()+addedHold;
            double restTime = castItem->getRestTime();
            bool applySteps = castItem->getApplySteps();

            int itemOffset = protocolItems.size();
            totalProtocolItems--;

            if (itemOffset < itemNum) {
                status = ItemsProcErrorNotEnoughItemsForSequence;

            } else if (itemOffset-itemNum < lastSeqItem) {
                status = ItemsProcErrorOverlappingSequences;

            } else if ((repsNum < 0) && (dropItemIdx < dropItemNum-1)) {
                status = ItemsProcErrorMidInfiniteSequence;

            } else {
                if (restTime != 0.0) {
                    /*! In case of rest time just add an invisible item */
                    protocolItems.push_back(new ProtocolXRestItem(restStimulus, restTime, ++protocolItemsNum));
                    itemOffset++;
                    itemNum++;
                    totalProtocolItems++;
                }

                /*! Set the last item to enter the loop */
                protocolItems.back()->nextItem -= itemNum;

                /*! Set loop reps and apply steps for all items in the loop */
                for (int itemIdx = 0; itemIdx < itemNum; itemIdx++) {
                    int sourceItemIdx = itemOffset-itemNum+itemIdx;
                    protocolItems[sourceItemIdx]->repsNum = (unsigned short)(repsNum+1);
                    protocolItems[sourceItemIdx]->applySteps = applySteps;
                }
            }

            lastSeqItem = protocolItems.size();
            break;
        }
        }
    }

    if ((unsigned int)totalProtocolItems > maxProtocolItems) {
        status = ItemsOverflow;

    } else if (totalProtocolItems  == 0) {
        status = ItemsNotFound;
    }

    switch (status) {
    case ItemsProcOk:
        errorItem->setVisible(false);
        break;

    case ItemsProcErrorNotEnoughItemsForSequence:
        errorText->setText("Not enough items for sequence");
        errorItem->setText(* errorText);
        errorItem->setVisible(true);
        protocolPlot->replot();
        break;

    case ItemsProcErrorOverlappingSequences:
        errorText->setText("Overlapping sequences");
        errorItem->setText(* errorText);
        errorItem->setVisible(true);
        protocolPlot->replot();
        break;

    case ItemsProcErrorMidInfiniteSequence:
        errorText->setText("Mid infinite sequence");
        errorItem->setText(* errorText);
        errorItem->setVisible(true);
        protocolPlot->replot();
        break;

    case ItemsOverflow:
        errorText->setText("Too many protocol items");
        errorItem->setText(* errorText);
        errorItem->setVisible(true);
        protocolPlot->replot();
        break;

    default:
        break;
    }
    return status;
}

MinimalProtocolPreview::MinimalProtocolPreview(ModelDevice * mDev, RangedMeasurement_t timeRange, RangedMeasurement_t stimulusRange, QString title) :
    ProtocolPreview(mDev, timeRange, stimulusRange, title) {

    cursorsWid->setVisible(false);
    minimal = true;
}
