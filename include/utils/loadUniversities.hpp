/**
 * @file loadUniversities.hpp
 * @brief Loads the university/department catalogue from a JSON resource.
 */
#ifndef LOAD_UNIVERSITIES_HPP
#define LOAD_UNIVERSITIES_HPP

#include <QHash>
#include <QList>
#include <QString>

/// @brief Free helper functions shared across the UI.
namespace Utils {
    /**
     * @brief A named catalogue entry with optional translated display names.
     */
    struct CatalogEntry {
        /// English name. Also the directory name under @c :/lists/, so it never changes with the language.
        QString name;
        /// Display names keyed by language code, e.g. "el".
        QHash<QString, QString> translations;

        /**
         * @brief Name to show for @p locale, falling back to the English name.
         *
         * @param locale Locale code such as "el_GR". Only the language part is used.
         */
        QString display_name(const QString &locale) const;
    };

    /**
     * @brief A university, its logo and its departments.
     */
    struct University : CatalogEntry {
        /// Resource path of the university logo.
        QString icon;
        /// Departments in display order.
        QList<CatalogEntry> departments;
    };

    /**
     * @brief Parses the universities JSON.
     *
     * The file must contain a JSON array of objects of the form
     * @code{.json}
     * [
     *   {
     *     "name": "Aristotle University of Thessaloniki",
     *     "translations": { "el": "Αριστοτέλειο Πανεπιστήμιο Θεσσαλονίκης" },
     *     "icon": ":/icons/auth_logo.png",
     *     "departments": [
     *       { "name": "Informatics", "translations": { "el": "Τμήμα Πληροφορικής" } }
     *     ]
     *   }
     * ]
     * @endcode
     * @c translations is optional everywhere. Universities and departments with an
     * empty or missing @c name are skipped.
     *
     * @param path Path to the JSON file, usually a Qt resource such as ":/universities.json".
     * @return The universities in file order. Empty, with a logged warning, if the file
     *         cannot be opened or is not a valid JSON array.
     */
    QList<University> loadUniversities(const QString &path);
}

#endif // LOAD_UNIVERSITIES_HPP
