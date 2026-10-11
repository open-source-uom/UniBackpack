/**
 * @file language.hpp
 * @brief The supported UI languages and runtime language switching.
 */
#ifndef UTILS_LANGUAGE_HPP
#define UTILS_LANGUAGE_HPP

#include <QList>
#include <QString>

namespace Utils {
    /**
     * @brief A language the UI can be shown in.
     */
    struct Language {
        /// Locale code stored in QSettings, e.g. "el_GR".
        QString code;
        /// The language's name in that language, e.g. "Ελληνικά".
        QString native_name;
    };

    /**
     * @brief The languages offered in the language selector, English first.
     *
     * Each one except English needs a <tt>translations/unibackpack_\<language\>.ts</tt> file.
     */
    QList<Language> supportedLanguages();

    /**
     * @brief Switches the application's UI language to @p locale.
     *
     * Loads <tt>:/i18n/unibackpack_\<language\>.qm</tt> for the best match of @p locale
     * and installs it, replacing the translator installed by a previous call.
     * If no matching or non-empty @c .qm file exists, no translator is installed,
     * so the original English strings are shown. Qt's own translations, used for
     * example by standard dialog buttons, are installed the same way when the
     * system has them.
     *
     * Installing or removing a translator makes Qt send QEvent::LanguageChange
     * to the application, and widgets then refresh their texts in @c changeEvent().
     *
     * @param locale Locale code such as "en_US" or "el_GR".
     *
     * @pre A QCoreApplication instance must exist.
     * @note Not thread-safe; call it from the GUI thread only.
     */
    void applyTranslator(const QString &locale);

    /**
     * @brief The language the user picked, or the system locale if they never picked one.
     *
     * @return Locale code such as "el_GR", read from QSettings under "language".
     */
    QString savedLanguage();

    /**
     * @brief Stores @p locale as the user's language for savedLanguage().
     */
    void saveLanguage(const QString &locale);
}

#endif // UTILS_LANGUAGE_HPP
