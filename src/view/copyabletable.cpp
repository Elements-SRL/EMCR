#include "copyabletable.h"

#include <QGuiApplication>
#include <QClipboard>
#include <QKeyEvent>
#include <QTextStream>
#include <QHeaderView>

CopyableTable::CopyableTable(QWidget *parent) : QTableWidget(parent) {

}

QSize CopyableTable::sizeHint() const {
    QSize size = QTableWidget::sizeHint();
    int tableWidth = 2; // margin

    if (this->verticalHeader()->isVisible()) {
        tableWidth += this->verticalHeader()->width();
    }

    for (int colIdx = 0; colIdx < this->columnCount(); colIdx++) {
        tableWidth += this->columnWidth(colIdx);
    }

    size.setWidth(tableWidth);
    return size;
}

bool CopyableTable::eventFilter(QObject * obj, QEvent * event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent * keyEvent = static_cast <QKeyEvent *> (event);
        if ((keyEvent->key() == Qt::Key_C) && (keyEvent->modifiers() == Qt::CTRL)) {
            QClipboard * clipboard = QGuiApplication::clipboard();
            QList <QTableWidgetSelectionRange> ranges = this->selectedRanges();
            if (ranges.size() > 0) {
                QTableWidgetSelectionRange range = ranges.at(0);
                QString clipboardText;
                QTextStream stream(&clipboardText);
                stream.setLocale(QLocale::system());
                int colIdx;

                for (int rowIdx = range.topRow(); rowIdx <= range.bottomRow(); rowIdx++) {
                    for (colIdx = range.leftColumn(); colIdx < range.rightColumn(); colIdx++) {
                        this->addCell(this->item(rowIdx, colIdx), stream);
                    }
                    this->addCell(this->item(rowIdx, colIdx), stream, "\n");
                }
                clipboard->setText(clipboardText);
            }
        }
    }
    return QObject::eventFilter(obj, event);
}

void CopyableTable::addCell(QTableWidgetItem * item, QTextStream &stream, QString terminator) {
    /*! Cells may be empty, in which case just set the terminator */
    if (item != nullptr) {
        bool conversionOk;
        double value;

        /*! Cells may contain both numeric values and strings, so we need to discriminate the 2 in order to use local format for numbers */
        QString cellString;
        QStringList cellStrings;
        cellStrings = item->text().split(" ", QString::SkipEmptyParts);
        if (cellStrings.size() > 0) {
            int stringIdx;
            for (stringIdx = 0; stringIdx < cellStrings.size()-1; stringIdx++) {
                cellString = cellStrings[stringIdx];
                value = cellString.toDouble(&conversionOk);
                if (conversionOk) {
                    stream << value << " ";

                } else {
                    stream << cellString << " ";
                }
            }

            cellString = cellStrings[stringIdx];
            value = cellString.toDouble(&conversionOk);
            if (conversionOk) {
                stream << value << terminator;

            } else {
                stream << cellString << terminator;
            }

        } else {
            stream << terminator;
        }

    } else {
        stream << terminator;
    }
}
