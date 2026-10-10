#include "utils/applyTranslator.hpp"
#include <QTranslator>
#include "QCoreApplication"

namespace Utils {
    void applyTranslator(const QString &locale) {
        static QTranslator elTranslator;
        static bool installed = false;

        if (installed) {
            QCoreApplication::removeTranslator(&elTranslator);
            installed = false;
        }

        if (locale.startsWith("el")) {
            if (elTranslator.isEmpty()) // if no translation is found, the original English text is shown
                elTranslator.load(":/i18n/unibackpack_el.qm");
            if (!elTranslator.isEmpty()) {
                QCoreApplication::installTranslator(&elTranslator);
                installed = true;
            }
        }
    }
}
