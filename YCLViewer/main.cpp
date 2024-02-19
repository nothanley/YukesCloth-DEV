#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    QStringList args = QCoreApplication::arguments();
    for (auto &arg : args)
        if (arg.endsWith(".ycl") )
            w.OpenYukesClothFile(arg);


    return a.exec();
}
