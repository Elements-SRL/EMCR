#include <QApplication>

#include "controllermain.h"
#include "mainwindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    qRegisterMetaType <vector <string>> ("vector <string>");
    qRegisterMetaType <e384cl::ErrorCodes_t> ("e384cl::ErrorCodes_t");

    ControllerMain c;
    MainWindow w;
    c.setMainWindow(&w);
    w.show();

    return a.exec();
}
