#include "utils/packageList.hpp"

#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include <algorithm>

namespace Utils {
    QString packageListPath(const QString &university, const QString &department) {
        return ":/lists/" + university + "/" + department + "/packages.json";
    }

    QList<PackageEntry> loadPackageList(const QString &path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) {
            qWarning() << "Could not open package list" << path << file.errorString();
            return {};
        }

        QJsonParseError error;
        const QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
        if (error.error != QJsonParseError::NoError || !doc.isArray()) {
            qWarning() << "Invalid package list" << path << error.errorString();
            return {};
        }

        QList<PackageEntry> entries;
        for (const QJsonValue &value : doc.array()) {
            const QJsonObject object = value.toObject();

            PackageEntry entry;
            entry.name = object["name"].toString();
            if (entry.name.isEmpty())
                continue;
            for (auto it = object.begin(); it != object.end(); ++it) {
                const QString package = it.value().toString().trimmed();
                if (it.key() != "name" && !package.isEmpty())
                    entry.packages.insert(it.key(), package);
            }
            entries.append(entry);
        }
        return entries;
    }

    bool hasPackagesFor(const QList<PackageEntry> &entries, const QString &package_manager) {
        return std::any_of(entries.begin(), entries.end(), [&package_manager](const PackageEntry &entry) {
            return entry.packages.contains(package_manager);
        });
    }
}
