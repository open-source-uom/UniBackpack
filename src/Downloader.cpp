// Authors: Apostolos Chalis 2026 <achalis@csd.auth.gr>, Ioannis Michadasis 2026 <cs05415@uowm.gr>
#include "Downloader.hpp"

#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QTextStream>
#include <QMessageBox>
#include <QStandardPaths>
#include <QProcessEnvironment>
#include <QRegularExpression>

QString Downloader::check_package_manager() {
    if (!QStandardPaths::findExecutable("pacman").isEmpty()) {
        qDebug() << "Pacman found.";
        return "pacman";
    } else if (!QStandardPaths::findExecutable("apt").isEmpty()) {
        qDebug() << "Apt found.";
        return "apt";
    } else {
        qDebug() << "No supported package manager found.";
        return "Unsupported";
    }
}

bool Downloader::is_in_pacman_repo(const QString &package_name) {
    QProcess process;
    process.start("pacman", QStringList() << "-Si" << package_name);
    process.waitForFinished();
    if (process.exitCode() == 0) {
        emit status_message("Found: " + package_name);
        return true;
    } else {
        emit status_message("Not found: " + package_name);
        return false;
    }
}

bool Downloader::is_in_apt_repo(const QString &package_name) {
    QProcess process;
    process.start("apt-cache", QStringList() << "show" << package_name);
    process.waitForFinished();
    if (process.exitCode() == 0) {
        emit status_message("Found: " + package_name);
        return true;
    } else {
        emit status_message("Not found: " + package_name);
        return false;
    }
}

QStringList Downloader::read_package_list(bool standard_package_manager, QString package_manager) {
    QStringList installable_with_standard_package_manager;
    QStringList installable_with_non_standard_package_manager;

    if (standard_package_manager && package_manager == "pacman") {
        QString filepath_of_list = ":/lists/" + name_of_university + "/" + name_of_department + "/pacman_list.txt";
        emit status_message("Checking packages for " + name_of_department + "...");
        QFile file(filepath_of_list);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Critical Error: Could not open the file!" << file.errorString();
            return installable_with_standard_package_manager;
        }
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString package = in.readLine().trimmed();
            if (!package.isEmpty() && is_in_pacman_repo(package)) {
                emit status_message("Adding: " + package);
                installable_with_standard_package_manager.append(package);
            }
        }
        file.close();
        return installable_with_standard_package_manager;
    }

    if (!standard_package_manager && package_manager == "pacman") {
        QString filepath_of_list = ":/lists/" + name_of_university + "/" + name_of_department + "/pacman_list.txt";
        emit status_message("Checking packages for " + name_of_department + "...");
        QFile file(filepath_of_list);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Critical Error: Could not open the file!" << file.errorString();
            return installable_with_non_standard_package_manager;
        }
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString package = in.readLine().trimmed();
            if (!package.isEmpty() && !is_in_pacman_repo(package)) {
                emit status_message("Adding (non-standard): " + package);
                installable_with_non_standard_package_manager.append(package);
            }
        }
        file.close();
        return installable_with_non_standard_package_manager;
    }

    if (standard_package_manager && package_manager == "apt") {
        QString filepath_of_list = ":/lists/" + name_of_university + "/" + name_of_department + "/apt_list.txt";
        emit status_message("Checking packages for " + name_of_department + "...");
        QFile file(filepath_of_list);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Critical Error: Could not open the file!" << file.errorString();
            return installable_with_standard_package_manager;
        }
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString package = in.readLine().trimmed();
            if (!package.isEmpty() && is_in_apt_repo(package)) {
                emit status_message("Adding: " + package);
                installable_with_standard_package_manager.append(package);
            }
        }
        file.close();
        return installable_with_standard_package_manager;
    }

    if (!standard_package_manager && package_manager == "apt") {
        QString filepath_of_list = ":/lists/" + name_of_university + "/" + name_of_department + "/apt_list.txt";
        emit status_message("Checking packages for " + name_of_department + "...");        
        QFile file(filepath_of_list);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Critical Error: Could not open the file!" << file.errorString();
            return installable_with_non_standard_package_manager;
        }
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString package = in.readLine().trimmed();
            if (!package.isEmpty() && !is_in_apt_repo(package)) {
                emit status_message("Adding (non-standard): " + package);
                installable_with_non_standard_package_manager.append(package);
            }
        }
        file.close();
        return installable_with_non_standard_package_manager;
    }

    qDebug() << "Critical Error: Unsupported package manager:" << package_manager;
    return {};
}

void Downloader::download_via_pacman(const QStringList &list_to_be_downloaded) {
    // there is a problem with pacman -Syu though, so the user should be alerted about that
    QMessageBox::warning(
        nullptr,
        "Warning",
        "It is advised to update your system before proceeding."
        "\nRun sudo pacman -Syu"
    );

    if (list_to_be_downloaded.isEmpty()) {
        qDebug() << "Download list is empty\nNothing to do.";
        emit download_completed(false);
        return;
    }

    qDebug() << "Starting to download package list via pacman";

    int total = list_to_be_downloaded.size();
    int *downloaded = new int(0);

    QProcess *download_process = new QProcess(this);

    connect(download_process, &QProcess::readyReadStandardOutput, this, [=]() {
        QString output = download_process->readAllStandardOutput();
        emit status_message(output);

        // Download phase: "downloading pkgname..."
        static QRegularExpression download_re(R"(downloading (.+)\.\.\.)");
        QRegularExpressionMatchIterator it = download_re.globalMatch(output);
        int download_count = 0;
        while (it.hasNext()) {
            it.next();
            download_count++;
        }
        if (download_count > 0 && total > 0) {
            *downloaded += download_count;
            int percent = static_cast<int>((*downloaded * 50.0) / total);
            emit progress_updated(qMin(percent, 49));
        }

        // Install phase: "(1/23) installing pkgname"
        static QRegularExpression pacman_re(R"(\((\d+)/(\d+)\))");
        QRegularExpressionMatch match = pacman_re.match(output);
        if (match.hasMatch()) {
            int current = match.captured(1).toInt();
            int pkg_total = match.captured(2).toInt();
            if (pkg_total > 0) {
                int percent = 50 + static_cast<int>((current * 50.0) / pkg_total);
                emit progress_updated(qMin(percent, 99));
            }
        }
    });

    connect(download_process, &QProcess::readyReadStandardError, this, [=]() {
        emit status_message(download_process->readAllStandardError());
    });

    connect(download_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus status) {
        delete downloaded;
        qDebug() << "Process finished with exit code:" << exitCode << "status:" << status;
        if (exitCode == 0) {
            qDebug() << "Package list downloaded via pacman";
        } else {
            qDebug() << "Error downloading packages via pacman. Exit code:" << exitCode;
        }
        emit download_completed(exitCode == 0);
        download_process->deleteLater();
    });

    QStringList command_structure;
    command_structure << "pacman" << "-S" << "--noconfirm" << "--needed" << list_to_be_downloaded;
    qDebug() << "Executing: pkexec" << command_structure.join(" ");
    download_process->start("pkexec", command_structure);
}

void Downloader::download_via_apt(const QStringList &list_to_be_downloaded) {
    if (list_to_be_downloaded.isEmpty()) {
        qDebug() << "Download list is empty\nNothing to do.";
        emit download_completed(false);
        return;
    }

    qDebug() << "Starting to download package list via apt";

    QProcess *download_process = new QProcess(this);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("DEBIAN_FRONTEND", "noninteractive");
    download_process->setProcessEnvironment(env);

    connect(download_process, &QProcess::readyReadStandardOutput, this, [=]() {
        emit status_message(download_process->readAllStandardOutput());
    });
    connect(download_process, &QProcess::readyReadStandardError, this, [=]() {
        emit status_message(download_process->readAllStandardError());
    });
    connect(download_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [=](int exitCode, QProcess::ExitStatus status) {
        qDebug() << "Process finished with exit code:" << exitCode << "status:" << status;
        if (exitCode == 0) {
            qDebug() << "Package list downloaded via apt";
        } else {
            qDebug() << "Error downloading packages via apt. Exit code:" << exitCode;
        }
        emit download_completed(exitCode == 0);
        download_process->deleteLater();
    });

    QStringList command_structure;
    command_structure << "apt" << "install" << "-y"
                      << "-o" << "APT::Status-Fd=1"
                      << list_to_be_downloaded;
    qDebug() << "Executing: pkexec" << command_structure.join(" ");
    download_process->start("pkexec", command_structure);
}
