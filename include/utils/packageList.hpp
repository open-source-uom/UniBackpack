/**
 * @file packageList.hpp
 * @brief Loads a department's package list from a JSON resource.
 */
#ifndef PACKAGE_LIST_HPP
#define PACKAGE_LIST_HPP

#include <QHash>
#include <QList>
#include <QString>

namespace Utils {
    /**
     * @brief One piece of software and its package name in each package manager.
     */
    struct PackageEntry {
        /// Human-readable name, e.g. "Java JDK".
        QString name;
        /// Package names keyed by PackageManager::name(), e.g. "apt" -> "default-jdk".
        QHash<QString, QString> packages;
    };

    /**
     * @brief Resource path of a department's package list.
     *
     * @return <tt>:/lists/\<university\>/\<department\>/packages.json</tt>
     */
    QString packageListPath(const QString &university, const QString &department);

    /**
     * @brief Parses a package list.
     *
     * The file must contain a JSON array of objects of the form
     * @code{.json}
     * [
     *   { "name": "Java JDK", "apt": "default-jdk", "pacman": "jdk-openjdk" },
     *   { "name": "FreeCAD", "pacman": "freecad" }
     * ]
     * @endcode
     * Every key other than @c name is a package manager. Leave a manager out when
     * the software is not packaged for it. Entries without a @c name are skipped.
     *
     * @param path Path to the JSON file, usually from packageListPath().
     * @return The entries in file order. Empty, with a logged warning, if the file
     *         cannot be opened or is not a valid JSON array.
     */
    QList<PackageEntry> loadPackageList(const QString &path);

    /**
     * @brief Whether any entry of @p entries has a package for @p package_manager.
     */
    bool hasPackagesFor(const QList<PackageEntry> &entries, const QString &package_manager);
}

#endif // PACKAGE_LIST_HPP
