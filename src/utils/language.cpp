#include "utils/language.hpp"

#include <QCoreApplication>
#include <QLibraryInfo>
#include <QLocale>
#include <QSettings>
#include <QTranslator>

namespace Utils {
    namespace {
        const QString language_key = QStringLiteral("language");

        // Replaces whatever @p translator held with the best match for @p locale
        void reinstall(QTranslator &translator, bool &installed, const QLocale &locale,
                       const QString &file_name, const QString &directory) {
            if (installed) {
                QCoreApplication::removeTranslator(&translator);
                installed = false;
            }
            if (translator.load(locale, file_name, "_", directory) && !translator.isEmpty()) {
                QCoreApplication::installTranslator(&translator);
                installed = true;
            }
        }
    }

    QList<Language> supportedLanguages() {
        return {
            {"en_US", "English"},
            {"el_GR", "Ελληνικά"},
        };
    }

    void applyTranslator(const QString &locale) {
        static QTranslator app_translator;
        static bool app_installed = false;
        static QTranslator qt_translator;
        static bool qt_installed = false;

        const QLocale qlocale(locale);
        // Tries unibackpack_el_GR.qm, then unibackpack_el.qm, and so on
        reinstall(app_translator, app_installed, qlocale, "unibackpack", ":/i18n");
        reinstall(qt_translator, qt_installed, qlocale, "qtbase",
                  QLibraryInfo::path(QLibraryInfo::TranslationsPath));
    }

    QString savedLanguage() {
        return QSettings().value(language_key, QLocale::system().name()).toString();
    }

    void saveLanguage(const QString &locale) {
        QSettings().setValue(language_key, locale);
    }
}
