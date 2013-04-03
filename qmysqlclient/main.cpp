#include "qmysqlclient.h"
#include <QtWidgets/QApplication>
#include <QtCore/QTranslator>
#include <QtCore/QDebug>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	QApplication::setWindowIcon(QIcon(":QMysqlClient/qt.png"));
	QTranslator translator;
	QString transFileName = QLocale::system().name() + ".qm";
	QString transPath;
#ifdef NDEBUG
	transPath = "locales";
#else
	transPath = ".";
#endif
	translator.load(transFileName, transPath);
	a.installTranslator(&translator);
	QMysqlClient w;
	w.show();
	return a.exec();
}
