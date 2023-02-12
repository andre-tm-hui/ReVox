#define _ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH

#include <Windows.h>

#include <QApplication>
#include <QComboBox>
#include <QLocale>
#include <QTranslator>

#include "singleapplication.h"
#include "ui/mainwindow.h"

int main(int argc, char *argv[]) {
  // make sure only 1 instance of the app is running
  // (https://github.com/itay-grudev/SingleApplication)
  SingleApplication a(argc, argv);

  spdlog::flush_on(spdlog::level::warn);
  spdlog::flush_every(std::chrono::seconds(5));

  QTranslator translator;
  const QStringList uiLanguages = QLocale::system().uiLanguages();
  for (const QString &locale : uiLanguages) {
    const QString baseName = "ReVox_" + QLocale(locale).name();
    if (translator.load(":/i18n/" + baseName)) {
      a.installTranslator(&translator);
      break;
    }
  }
  MainWindow w;

  w.keyboardListener->Start();

  return a.exec();
}
