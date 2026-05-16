// Author: Apostolos Chalis 2026 <achalis@csd.auth.gr>
#include <QApplication>
#include "MainWindow.hpp"

int main(int argc, char *argv[]){
	QApplication uni_backpack_app(argc, argv);
	uni_backpack_app.setWindowIcon(QIcon(":/icons/unibackpack.png"));
	uni_backpack_app.setDesktopFileName("UniBackpack");

	MainWindow main_window;
	main_window.show();

	return uni_backpack_app.exec();
}
