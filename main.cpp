#include "mainwindowtester.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindowTester w;
    w.show();
    return a.exec();
}
