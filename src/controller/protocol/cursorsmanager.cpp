#include "cursorsmanager.h"
#include "protocolwidget.h"

CursorsManager::CursorsManager(ProtocolPlot * plot, unsigned int maxTriggerEvents) :
    plot(plot),
    maxTriggerEvents(maxTriggerEvents) {

    cursors = plot->getProtocolCursors();
    connect(plot, &ProtocolPlot::addCursors, this, &CursorsManager::onAddCursors);
    connect(plot, &ProtocolPlot::moveCursors, this, &CursorsManager::onUpdateCursors);
    connect(plot, &ProtocolPlot::editCursors, this, &CursorsManager::onUpdateCursors);
    connect(plot, &ProtocolPlot::removeCursors, this, &CursorsManager::onRemoveCursors);
}

void CursorsManager::setProtocol(ProtocolWidget * protocol) {
    this->protocol = protocol;
}

void CursorsManager::setAnalysisPidl(AnalysisProtocolItemDropList * analysisPidl) {
    this->analysisPidl = analysisPidl;
    connect(analysisPidl, &AnalysisProtocolItemDropList::requestCursors, this, &CursorsManager::onRequestCursors);
}

void CursorsManager::enableAnalysis(bool enabled) {
    analysisPidl->enableAnalysis(enabled);
}

void CursorsManager::onRequestCursors(ProtocolDropAnalysisItem * item) {
    cursors = plot->getProtocolCursors();
    item->addCursors(cursors);
}

void CursorsManager::onAddCursors() {
    cursors = plot->getProtocolCursors();
    analysisPidl->addCursors(cursors);

    this->onUpdateCursors();
}

void CursorsManager::onRemoveCursors(QVector <int> cursorsMap) {
    if (cursorsMap.size() > 0) {
        if (cursorsMap.back() != cursorsMap.size()-1) {
            cursors = plot->getProtocolCursors();
            analysisPidl->removeCursors(cursors, cursorsMap);
        }
    }

    this->onUpdateCursors();
}

void CursorsManager::onUpdateCursors() {
    analysisPidl->updateCursors();

    this->interpretCursors();

    protocol->pushAnalysisCursors(analysisCursors);
    protocol->pushTriggerCursors(triggerCursors);
}

void CursorsManager::interpretCursors() {
    analysisCursors.clear();
    triggerCursors.clear();
    ProtocolCursor * protocolCursor;
    AnalysisCursor * analysisCursor;
    QVector <double> delays;

    int repType;
    int repsIdx;
    int repsNum;
    int sweepType;
    int sweepsIdx;
    int sweepsNum;
    int trigType;
    bool trigPolarity;
    int trigId;

    for (int cursorIdx = 0; cursorIdx < cursors->size(); cursorIdx++) {
        protocolCursor = cursors->at(cursorIdx);
        repType = protocolCursor->getRepetitionType();
        repsIdx = protocolCursor->getRepetitionIdx();
        repsNum = protocolCursor->getRepetitionsNum();

        trigType = protocolCursor->getTriggerType();
        trigPolarity = (trigType == ProtocolCursor::TriggerRising ? true : false);
        trigId = protocolCursor->getTriggerId();

        if (protocolCursor->getProtocolType() == ProtocolTypeGapfree) {
            if ((repType != ProtocolCursor::RepetitionAll) && (repsNum > 0)) {
                analysisCursor = new AnalysisGapFreeCursor();
                delays.resize(1);
                delays[0] = protocolCursor->getOffset()/1000.0; /*! \todo FCON conversione da ms a s, brutto */

                if (trigType != ProtocolCursor::TriggerNone) {
                    triggerCursors.append(new TriggerCursor(trigId, trigPolarity, protocolCursor->getAbsoluteOffset(repsIdx, 0)/1000.0));
                }

            } else if ((repType == ProtocolCursor::RepetitionAll) && (repsNum > 0)) {
                analysisCursor = new AnalysisGapFreeAllRepsCursor();
                delays.resize(repsNum);
                for (repsIdx = 0; repsIdx < repsNum; repsIdx++) {
                    delays[repsIdx] = protocolCursor->getOffset(repsIdx, 0)/1000.0; /*! \todo FCON conversione da ms a s, brutto */

                    if (trigType != ProtocolCursor::TriggerNone) {
                        triggerCursors.append(new TriggerCursor(trigId, trigPolarity, protocolCursor->getAbsoluteOffset(repsIdx, 0)/1000.0));
                    }
                }

            } else if ((repType != ProtocolCursor::RepetitionAll) && (repsNum == 0)) {
                analysisCursor = new AnalysisGapFreeInfRepsCursor();
                delays.resize(1);
                delays[0] = protocolCursor->getOffset()/1000.0; /*! \todo FCON conversione da ms a s, brutto */

                if (trigType != ProtocolCursor::TriggerNone) {
                    triggerCursors.append(new TriggerCursor(trigId, trigPolarity, protocolCursor->getAbsoluteOffset(repsIdx, 0)/1000.0));
                }

            } else {
                analysisCursor = new AnalysisGapFreeInfAllRepsCursor();
                delays.resize(1);
                delays[0] = protocolCursor->getOffset()/1000.0; /*! \todo FCON conversione da ms a s, brutto */

                /*! Can only create a finite number of digital triggers, so limit repetitions */
                for (repsIdx = 0; repsIdx < (int)maxTriggerEvents; repsIdx++) {
                    if (trigType != ProtocolCursor::TriggerNone) {
                        triggerCursors.append(new TriggerCursor(trigId, trigPolarity, protocolCursor->getAbsoluteOffset(repsIdx, sweepsIdx)/1000.0));
                    }
                }
            }

        } else {
            sweepType = protocolCursor->getSweepType();
            sweepsIdx = protocolCursor->getSweepIdx();
            sweepsNum = protocolCursor->getSweepsNum();

            if ((repType != ProtocolCursor::RepetitionAll) && (sweepType != ProtocolCursor::SweepAll)) {
                analysisCursor = new AnalysisEpisodicCursor();
                delays.resize(1);
                delays[0] = protocolCursor->getOffset()/1000.0; /*! \todo FCON conversione da ms a s, brutto */

                if (trigType != ProtocolCursor::TriggerNone) {
                    triggerCursors.append(new TriggerCursor(trigId, trigPolarity, protocolCursor->getAbsoluteOffset(repsIdx, sweepsIdx)/1000.0));
                }

            } else if ((repType == ProtocolCursor::RepetitionAll) && (sweepType != ProtocolCursor::SweepAll)) {
                analysisCursor = new AnalysisEpisodicAllRepsCursor();
                delays.resize(1);
                delays[0] = protocolCursor->getOffset()/1000.0; /*! \todo FCON conversione da ms a s, brutto */

                for (repsIdx = 0; repsIdx < repsNum; repsIdx++) {
                    if (trigType != ProtocolCursor::TriggerNone) {
                        triggerCursors.append(new TriggerCursor(trigId, trigPolarity, protocolCursor->getAbsoluteOffset(repsIdx, sweepsIdx)/1000.0));
                    }
                }

            } else if ((repType != ProtocolCursor::RepetitionAll) && (sweepType == ProtocolCursor::SweepAll)) {
                analysisCursor = new AnalysisEpisodicAllSweepsCursor();
                delays.resize(sweepsNum);
                for (sweepsIdx = 0; sweepsIdx < sweepsNum; sweepsIdx++) {
                    delays[sweepsIdx] = protocolCursor->getOffset(0, sweepsIdx)/1000.0; /*! \todo FCON conversione da ms a s, brutto */

                    if (trigType != ProtocolCursor::TriggerNone) {
                        triggerCursors.append(new TriggerCursor(trigId, trigPolarity, protocolCursor->getAbsoluteOffset(repsIdx, sweepsIdx)/1000.0));
                    }
                }

            } else {
                analysisCursor = new AnalysisEpisodicAllRepsAllSweepsCursor();
                delays.resize(sweepsNum);
                for (sweepsIdx = 0; sweepsIdx < sweepsNum; sweepsIdx++) {
                    delays[sweepsIdx] = protocolCursor->getOffset(0, sweepsIdx)/1000.0; /*! \todo FCON conversione da ms a s, brutto */

                    for (repsIdx = 0; repsIdx < repsNum; repsIdx++) {
                        if (trigType != ProtocolCursor::TriggerNone) {
                            triggerCursors.append(new TriggerCursor(trigId, trigPolarity, protocolCursor->getAbsoluteOffset(repsIdx, sweepsIdx)/1000.0));
                        }
                    }
                }
            }
        }

        analysisCursor->itemIdx = protocolCursor->getItemIdx();
        analysisCursor->repsIdx = protocolCursor->getRepetitionIdx();
        analysisCursor->sweepIdx = protocolCursor->getSweepIdx();
        analysisCursor->setDelays(delays);

        analysisCursors.append(analysisCursor);
    }
    std::sort(triggerCursors.begin(), triggerCursors.end(), TriggerCursorsCompare());
    triggerCursors.append(new TriggerTerminator());
}
