#include "utils/loadUniversities.hpp"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QLocale>

namespace Utils {
    namespace {
        CatalogEntry read_entry(const QJsonObject &object) {
            CatalogEntry entry;
            entry.name = object["name"].toString();
            const QJsonObject translations = object["translations"].toObject();
            for (auto it = translations.begin(); it != translations.end(); ++it)
                entry.translations.insert(it.key(), it.value().toString());
            return entry;
        }
    }

    QString CatalogEntry::display_name(const QString &locale) const {
        const QString language = QLocale(locale).name().section('_', 0, 0);
        return translations.value(language, name);
    }

    QList<University> loadUniversities(const QString &path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Could not open" << path;
            return {};
        }

        QJsonParseError error;
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
        if (error.error != QJsonParseError::NoError || !doc.isArray()) {
            qWarning() << "Invalid universities JSON:" << error.errorString();
            return {};
        }

        QList<University> universities;
        for (const QJsonValue &value : doc.array()) {
            const QJsonObject object = value.toObject();

            University university;
            static_cast<CatalogEntry &>(university) = read_entry(object);
            if (university.name.isEmpty())
                continue;
            university.icon = object["icon"].toString();

            for (const QJsonValue &department : object["departments"].toArray()) {
                CatalogEntry entry = read_entry(department.toObject());
                if (!entry.name.isEmpty())
                    university.departments.append(entry);
            }
            universities.append(university);
        }
        return universities;
    }
}
