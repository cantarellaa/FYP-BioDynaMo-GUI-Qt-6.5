#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  QTranslator translator;
  const QStringList uiLanguages = QLocale::system().uiLanguages();
  for (const QString &locale : uiLanguages) {
      const QString baseName = "BioDyNaMo-Qt-6.5.0-GUI_" + QLocale(locale).name();
      if (translator.load(":/i18n/" + baseName)) {
          a.installTranslator(&translator);
          break;
      }
  }

  QFile stylesheet(":/stylesheets/main.qss");
  stylesheet.open(QFile::ReadOnly);
  QString stylesheetContent = QString::fromUtf8(stylesheet.readAll());
  a.setStyleSheet(stylesheetContent);

  QCoreApplication::setOrganizationName("cantarellaa");
  QCoreApplication::setApplicationName("biodynamo-gui");

  MainWindow w;

  w.showMaximized();

  return a.exec();
}
