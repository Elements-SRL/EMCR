#include <QApplication>

#include "controllermain.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    qRegisterMetaType <vector <string>> ("vector <string>");
    qRegisterMetaType <ErrorCodes_t> ("ErrorCodes_t");

    ControllerMain c;
    MainWindow w;
    c.setMainWindow(&w);
    w.show();

    return a.exec();
}
