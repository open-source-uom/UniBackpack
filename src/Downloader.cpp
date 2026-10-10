// Authors: Apostolos Chalis 2026 <achalis@csd.auth.gr>, Ioannis Michadasis 2026 <cs05415@uowm.gr>
#include "Downloader.hpp"
#include "utils/packageList.hpp"

#include <QDebug>
#include <QProcess>
#include <QProcessEnvironment>

namespace {
    QString describe_exit(int exit_code) {
        // Exit codes reserved by pkexec, see pkexec(1)
        switch (exit_code) {
        case 126:
            return Downloader::tr("Authorization was cancelled.");
        case 127:
            return Downloader::tr("You are not authorized to install packages.");
        default:
            return Downloader::tr("The package manager failed with exit code %1.").arg(exit_code);
        }
    }
}

Downloader::Downloader(const QString &university, const QString &department,
                       std::shared_ptr<PackageManager> manager, QObject *parent)
    : QObject(parent),
      name_of_university(university),
      name_of_department(department),
      package_manager(std::move(manager)) {}

void Downloader::start() {
    const QList<Utils::PackageEntry> entries =
        Utils::loadPackageList(Utils::packageListPath(name_of_university, name_of_department));

    // Package name -> software name, for the messages
    QHash<QString, QString> software_by_package;
    for (const Utils::PackageEntry &entry : entries) {
        const QString package = entry.packages.value(package_manager->name());
        if (package.isEmpty())
            report(tr("Not packaged for %1: %2").arg(package_manager->name(), entry.name));
        else
            software_by_package.insert(package, entry.name);
    }

    if (software_by_package.isEmpty()) {
        fail(tr("No %1 package list found for %2.").arg(package_manager->name(), name_of_department));
        return;
    }
    check_packages(software_by_package);
}

void Downloader::check_packages(const QHash<QString, QString> &software_by_package) {
    report(tr("Checking packages for %1...").arg(name_of_department));

    QStringList packages = software_by_package.keys();
    packages.sort();

    QProcess *query = new QProcess(this);

    // parse_available() matches untranslated field names
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("LC_ALL", "C");
    query->setProcessEnvironment(env);

    connect_start_failure(query);
    connect(query, &QProcess::finished, this, [this, query, packages, software_by_package]() {
        // The exit code is non-zero as soon as one package is missing, so only the output is used
        const QSet<QString> available =
            package_manager->parse_available(QString::fromUtf8(query->readAllStandardOutput()));
        query->deleteLater();

        QStringList installable;
        for (const QString &package : packages) {
            const QString label = QString("%1 (%2)").arg(software_by_package.value(package), package);
            if (available.contains(package)) {
                report(tr("Found: %1").arg(label));
                installable.append(package);
            } else {
                report(tr("Not in repos: %1").arg(label));
            }
        }
        install(installable);
    });

    query->start(package_manager->query_program(), package_manager->query_arguments(packages));
}

void Downloader::install(const QStringList &packages) {
    if (packages.isEmpty()) {
        fail(tr("None of the packages are available in your repositories."));
        return;
    }

    report(tr("Installing %n package(s)...", nullptr, static_cast<int>(packages.size())));
    package_manager->begin_install(static_cast<int>(packages.size()));
    pending_output.clear();

    QProcess *process = new QProcess(this);

    connect(process, &QProcess::readyReadStandardOutput, this, [this, process]() {
        handle_install_output(process->readAllStandardOutput());
    });
    connect(process, &QProcess::readyReadStandardError, this, [this, process]() {
        const QString errors = QString::fromUtf8(process->readAllStandardError()).trimmed();
        if (!errors.isEmpty())
            emit log_message(errors);
    });

    connect_start_failure(process);
    connect(process, &QProcess::finished, this, [this, process](int exit_code, QProcess::ExitStatus status) {
        qDebug() << "Process finished with exit code:" << exit_code << "status:" << status;
        flush_install_output();
        process->deleteLater();

        if (status != QProcess::NormalExit)
            fail(tr("The package manager stopped unexpectedly."));
        else if (exit_code != 0)
            fail(describe_exit(exit_code));
        else
            emit finished(true, QString());
    });

    // pkexec clears the environment, so variables have to go through env.
    // LC_ALL=C keeps the output untranslated for parse_progress().
    const QStringList arguments = QStringList{"env", "LC_ALL=C"}
                                  + package_manager->install_environment()
                                  + package_manager->install_command(packages);
    qDebug() << "Executing: pkexec" << arguments.join(' ');
    process->start("pkexec", arguments);
}

void Downloader::handle_install_output(const QByteArray &data) {
    // Chunks can end mid-line; keep the partial line until the rest arrives
    pending_output += QString::fromUtf8(data);
    const qsizetype end = pending_output.lastIndexOf('\n');
    if (end < 0)
        return;

    const QString complete = pending_output.left(end);
    pending_output.remove(0, end + 1);

    emit log_message(complete);
    for (const QString &line : complete.split('\n')) {
        if (const std::optional<int> percent = package_manager->parse_progress(line))
            emit progress_updated(*percent);
    }
}

void Downloader::flush_install_output() {
    if (!pending_output.isEmpty())
        handle_install_output(QByteArrayLiteral("\n"));
}

void Downloader::connect_start_failure(QProcess *process) {
    connect(process, &QProcess::errorOccurred, this, [this, process](QProcess::ProcessError error) {
        // finished() is not emitted when the process never started, e.g. pkexec is missing
        if (error != QProcess::FailedToStart)
            return;
        const QString message = tr("Could not run %1: %2").arg(process->program(), process->errorString());
        process->deleteLater();
        fail(message);
    });
}

void Downloader::report(const QString &message) {
    emit log_message(message);
    emit status_changed(message);
}

void Downloader::fail(const QString &message) {
    qWarning() << message;
    emit log_message(message);
    emit finished(false, message);
}
