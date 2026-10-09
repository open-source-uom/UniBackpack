/**
 * @file loadUniversities.hpp
 * @brief Loads the university/department catalogue from a JSON resource.
 */

#include "utils/loadUniversities.hpp"

#include <QString>
#include <QJsonDocument>
#include <QFile>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QStandardItem>

/// @brief Free helper functions shared across the UI.
namespace Utils {
    /**
     * @brief Parses the universities JSON and fills the lookup table and list model.
     *
     * The file must contain a JSON array of objects of the form
     * @code{.json}
     * [
     *   {
     *     "name": "Aristotle University of Thessaloniki",
     *     "icon": ":/icons/auth_logo.png",
     *     "departments": ["Informatics"]
     *   }
     * ]
     * @endcode
     * Entries with an empty or missing @c name are skipped.
     * For each university, one QStandardItem is appended to @p university_model.
     * Its display text and Qt::UserRole data are both set to the name, and its icon comes from @c icon.
     *
     * @param path Path to the JSON file, usually a Qt resource such as ":/universities.json".
     * @param departments_by_university Output map from university name to its department names.
     *        Existing entries with the same key are overwritten.
     * @param university_model Model to append university rows to. Must not be null.
     *        Existing rows are not cleared.
     *
     * @note If the file cannot be opened or is not a valid JSON array, a warning is logged
     *       and neither output is modified.
     */
    void loadUniversities(const QString &path, QHash<QString, QStringList> &departments_by_university, QStandardItemModel *university_model) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Could not open" << path;
            return;
        }

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
        if (error.error != QJsonParseError::NoError || !doc.isArray()) {
            qWarning() << "Invalid universities JSON:" << error.errorString();
            return;
        }

        for (const QJsonValue &value : doc.array()) {
            QJsonObject uni = value.toObject();
            QString key = uni["name"].toString();
            if (key.isEmpty())
                continue;

            QStringList departments;
            for (const QJsonValue &dept : uni["departments"].toArray())
                departments << dept.toString();
            departments_by_university.insert(key, departments);

            QStandardItem *item = new QStandardItem(QIcon(uni["icon"].toString()), key);
            item->setData(key, Qt::UserRole);
            university_model->appendRow(item);
        }
    }
}
