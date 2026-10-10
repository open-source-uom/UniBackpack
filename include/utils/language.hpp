/**
 * @file applyTranslator.hpp
 * @brief Runtime language switching for the application.
 */
#ifndef UTILS_TRANSLATOR_HPP
#define UTILS_TRANSLATOR_HPP

#include <QString>

namespace Utils {
    /**
     * @brief Switches the application's UI language to @p locale.
     *
     * Only Greek is currently translated. If @p locale starts with @c "el",
     * the Greek translator is loaded from @c :/i18n/unibackpack_el.qm and installed.
     * The file is loaded once and cached for later calls.
     * Any other locale removes the Greek translator, so the original English strings are shown.
     *
     * If the @c .qm file is missing or empty, nothing is installed and the UI stays in English.
     *
     * Installing or removing a translator makes Qt send QEvent::LanguageChange
     * to the application, and widgets then refresh their texts in @c changeEvent().
     *
     * @param locale Locale code as stored in QSettings under "language",
     *        e.g. "en_US" or "el_GR".
     *
     * @pre A QCoreApplication instance must exist.
     * @note Not thread-safe; call it from the GUI thread only.
     */
    void applyTranslator(const QString &locale);
}

#endif // UTILS_TRANSLATOR_HPP
