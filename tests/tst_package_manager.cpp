#include "PackageManager.hpp"
#include "utils/loadUniversities.hpp"
#include "utils/packageList.hpp"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QTemporaryFile>
#include <QtTest>

#include <memory>

namespace {
    QStringList sorted(const QSet<QString> &set) {
        QStringList list(set.begin(), set.end());
        list.sort();
        return list;
    }

    // -1 for "no progress", so results print readably when a comparison fails
    int progress(PackageManager &manager, const QString &line) {
        return manager.parse_progress(line).value_or(-1);
    }

    QString write_temporary(QTemporaryFile &file, const QByteArray &contents) {
        if (!file.open())
            return {};
        file.write(contents);
        file.close();
        return file.fileName();
    }
}

class TestPackageManager : public QObject {
    Q_OBJECT

private slots:
    void pacmanParsesAvailablePackages();
    void aptParsesAvailablePackages();
    void pacmanReportsProgress();
    void aptReportsProgress();
    void aptUpdatesBeforeInstalling();
    void displayNameFallsBackToEnglish();
    void loadsPackageList();
    void bundledDataIsConsistent();
};

void TestPackageManager::pacmanParsesAvailablePackages() {
    // pacman -Si bash nodejs not-a-package, run with LC_ALL=C (the error goes to stderr)
    const QString output =
        "Repository      : core\n"
        "Name            : bash\n"
        "Version         : 5.2.037-1\n"
        "Description     : The GNU Bourne Again shell\n"
        "\n"
        "Repository      : extra\n"
        "Name            : nodejs\n"
        "Version         : 23.5.0-1\n";

    PacmanBackend pacman;
    QCOMPARE(sorted(pacman.parse_available(output)), (QStringList{"bash", "nodejs"}));
}

void TestPackageManager::aptParsesAvailablePackages() {
    // apt-cache show --no-all-versions bash gcc, run with LC_ALL=C
    const QString output =
        "Package: bash\n"
        "Version: 5.2.21-2ubuntu4\n"
        "Pre-Depends: libc6 (>= 2.38)\n"
        "Description: GNU Bourne Again SHell\n"
        "\n"
        "Package: gcc\n"
        "Version: 4:13.2.0-7ubuntu1\n"
        "Package-Type: deb\n";

    AptBackend apt;
    QCOMPARE(sorted(apt.parse_available(output)), (QStringList{"bash", "gcc"}));
}

void TestPackageManager::pacmanReportsProgress() {
    PacmanBackend pacman;
    pacman.begin_install(2);

    // The real total, with dependencies, replaces the requested count
    QCOMPARE(progress(pacman, "Packages (4) a-1-1  b-1-1  c-1-1  d-1-1"), -1);

    // Downloads fill 0-49
    QCOMPARE(progress(pacman, " a-1-1-x86_64 downloading..."), 12);
    QCOMPARE(progress(pacman, " b-1-1-x86_64 downloading..."), 25);

    // Installs fill 50-99, counted or taken from "(n/total)"
    QCOMPARE(progress(pacman, "installing a..."), 62);
    QCOMPARE(progress(pacman, "(4/4) installing d..."), 99);
    QCOMPARE(progress(pacman, "(1/4) checking keys in keyring"), -1);
    QCOMPARE(progress(pacman, ":: Processing package changes..."), -1);
}

void TestPackageManager::aptReportsProgress() {
    AptBackend apt;
    apt.begin_install(3);

    // Downloads fill 0-49 and dpkg 50-99, so the bar never moves backwards
    QCOMPARE(progress(apt, "dlstatus:1:20.0:Retrieving file 1 of 5"), 10);
    QCOMPARE(progress(apt, "pmstatus:bash:50.0:Installing bash"), 75);
    QCOMPARE(progress(apt, "pmstatus:bash:100:Installed bash"), 99);
    QCOMPARE(progress(apt, "Reading package lists..."), -1);
    QCOMPARE(progress(apt, "pmstatus:broken"), -1);
}

void TestPackageManager::aptUpdatesBeforeInstalling() {
    AptBackend apt;
    const QStringList command = apt.install_command({"gcc", "$(reboot)"});

    // One shell runs update, then install; package names stay separate arguments ("$@")
    QCOMPARE(command.size(), 6);
    QCOMPARE(command.mid(0, 2), (QStringList{"sh", "-c"}));
    QVERIFY(command[2].startsWith("apt-get update && exec apt-get install "));
    QVERIFY(command[2].contains("--force-confold"));
    QVERIFY(!command[2].contains("gcc"));
    QCOMPARE(command.mid(3), (QStringList{"sh", "gcc", "$(reboot)"}));
}

void TestPackageManager::displayNameFallsBackToEnglish() {
    Utils::CatalogEntry entry;
    entry.name = "Informatics";
    entry.translations.insert("el", "Τμήμα Πληροφορικής");

    QCOMPARE(entry.display_name("el_GR"), QString("Τμήμα Πληροφορικής"));
    QCOMPARE(entry.display_name("en_US"), QString("Informatics"));
    QCOMPARE(entry.display_name("de_DE"), QString("Informatics"));
}

void TestPackageManager::loadsPackageList() {
    QTemporaryFile file;
    const QString path = write_temporary(file, R"([
        { "name": "Java JDK", "apt": "default-jdk", "pacman": "jdk-openjdk" },
        { "name": "FreeCAD", "pacman": "freecad", "apt": "  " },
        { "apt": "nameless" }
    ])");
    QVERIFY(!path.isEmpty());

    const QList<Utils::PackageEntry> entries = Utils::loadPackageList(path);
    QCOMPARE(entries.size(), 2);
    QCOMPARE(entries[0].name, QString("Java JDK"));
    QCOMPARE(entries[0].packages.value("apt"), QString("default-jdk"));
    QCOMPARE(entries[0].packages.value("pacman"), QString("jdk-openjdk"));
    QVERIFY(!entries[1].packages.contains("apt"));

    QVERIFY(Utils::hasPackagesFor(entries, "pacman"));
    QVERIFY(!Utils::hasPackagesFor(entries, "dnf"));
    QTest::ignoreMessage(QtWarningMsg, QRegularExpression("^Could not open package list"));
    QVERIFY(Utils::loadPackageList("/does/not/exist.json").isEmpty());
}

void TestPackageManager::bundledDataIsConsistent() {
    const QDir resources(QStringLiteral(UNIBACKPACK_SOURCE_DIR "/resources"));

    const QList<Utils::University> universities =
        Utils::loadUniversities(resources.filePath("universities.json"));
    QVERIFY2(!universities.isEmpty(), "universities.json is missing or invalid");

    QSet<QString> catalogued_lists;
    for (const Utils::University &university : universities) {
        QVERIFY2(QFile::exists(resources.filePath(university.icon.mid(2))),
                 qPrintable("Missing icon " + university.icon));

        for (const Utils::CatalogEntry &department : university.departments) {
            const QString path = resources.filePath("lists/" + university.name + "/" + department.name + "/packages.json");
            catalogued_lists.insert(QDir::cleanPath(path));
            if (!QFile::exists(path))
                continue; // Shown greyed out in the app

            const QList<Utils::PackageEntry> entries = Utils::loadPackageList(path);
            QVERIFY2(!entries.isEmpty(), qPrintable(path + " is empty or invalid"));
            for (const Utils::PackageEntry &entry : entries)
                QVERIFY2(!entry.packages.isEmpty(), qPrintable(entry.name + " in " + path + " has no packages"));
        }
    }

    // A list whose directory names don't match the catalogue is never shown
    QDirIterator it(resources.filePath("lists"), {"packages.json"}, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        const QString path = QDir::cleanPath(it.next());
        QVERIFY2(catalogued_lists.contains(path),
                 qPrintable(path + " does not match a university and department in universities.json"));
    }
}

QTEST_GUILESS_MAIN(TestPackageManager)
#include "tst_package_manager.moc"
