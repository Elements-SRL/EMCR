#ifndef DEBUGDOCKWIDGET_H
#define DEBUGDOCKWIDGET_H

#include <QDockWidget>

class DebugDockWidget : public QDockWidget {
    Q_OBJECT

public:
    DebugDockWidget(QWidget * parent = nullptr);

signals:
    void setDebugBit(int word, int bit, bool flag);
    void setDebugWord(int word, int value);
    void setDebugTrigger(int bit);
    void sigWriteCalibrationEeprom(std::vector <uint32_t> value, std::vector <uint32_t> address, std::vector <uint32_t> size);
};

#endif // DEBUGDOCKWIDGET_H
