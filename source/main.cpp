#include "mainwindow.h"
#include "audiomanager.h"
#include "singleapplication.h"

#include <QApplication>
#include <QComboBox>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    // make sure only 1 instance of the app is running (https://github.com/itay-grudev/SingleApplication)
    SingleApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "VirtualSnS_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;

    w.keyboardListener->Start();

    return a.exec();
}

void Test(){

}
