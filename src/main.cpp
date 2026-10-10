// Author: Apostolos Chalis 2026 <achalis@csd.auth.gr>
#include "MainWindow.hpp"
#include "utils/language.hpp"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[]) {
    qputenv("QT_QPA_PLATFORMTHEME", "");
    QApplication uni_backpack_app(argc, argv);
    uni_backpack_app.setOrganizationName("UniBackpack");
    uni_backpack_app.setApplicationName("UniBackpack");
    uni_backpack_app.setWindowIcon(QIcon(":/icons/unibackpack.png"));
    uni_backpack_app.setDesktopFileName("unibackpack");

    Utils::applyTranslator(Utils::savedLanguage());

    MainWindow main_window;
    main_window.show();

    return uni_backpack_app.exec();
}
