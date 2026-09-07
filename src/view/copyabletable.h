#ifndef COPYABLETABLE_H
#define COPYABLETABLE_H

#include <QTableWidget>

class CopyableTable : public QTableWidget
{
    Q_OBJECT
public:
    explicit CopyableTable(QWidget *parent = nullptr);
    QSize sizeHint() const override;

protected:
    bool eventFilter(QObject * obj, QEvent * event) override;

private:
    bool exportHeader;
    void addCell(QTableWidgetItem * item, QTextStream &stream, QString terminator = "\t");

signals:

};

#endif // COPYABLETABLE_H
