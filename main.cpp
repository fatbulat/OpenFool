#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator qtTranslator;
    qtTranslator.load("qt_" + QLocale::system().name().left(2),
                      QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&qtTranslator);

    QTranslator openFoolTranslator;
    openFoolTranslator.load(
        "OpenFool_" + QLocale::system().name().left(2),
        QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    a.installTranslator(&openFoolTranslator);

    MainWindow w;
    w.show();

    return a.exec();
}
