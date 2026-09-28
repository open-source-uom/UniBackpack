// Author: Apostolos Chalis 2026 <achalis@csd.auth.gr>
#include <QApplication>
#include "MainWindow.hpp"
#include "Utilities/ConfigChecker.hpp"
#include <QTranslator>
#include <QSettings>
#include <QLocale>

int main(int argc, char *argv[]){
	qputenv("QT_QPA_PLATFORMTHEME", "");
	QApplication uni_backpack_app(argc, argv);
	uni_backpack_app.setOrganizationName("UniBackpack");
    uni_backpack_app.setApplicationName("UniBackpack");
	uni_backpack_app.setWindowIcon(QIcon(":/icons/unibackpack.png"));
	uni_backpack_app.setDesktopFileName("UniBackpack");

	// Load saved language, default to system locale, fall back to English
	QSettings settings;
    QString lang = settings.value("language", QLocale::system().name()).toString();
	applyTranslator(lang);

	ConfigChecker *startupConfigChecker = new ConfigChecker(&uni_backpack_app);
	startupConfigChecker->scanAndNotify();
	
	MainWindow main_window;
	main_window.show();

	return uni_backpack_app.exec();
}
