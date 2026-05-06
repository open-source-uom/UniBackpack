// Author: Apostolos Chalis 2026 <achalis@csd.auth.gr>
#include "Downloader.hpp"

#include <QDebug> 
#include <QFile>
#include <QString> 
#include <QProcess> 
#include <QMessageBox>
#include <QTextStream>
#include <QStringList>
#include <QStandardPaths>

QString Downloader::check_package_manager() {
    if (!QStandardPaths::findExecutable("pacman").isEmpty()){
        return "pacman";
    } else if (!QStandardPaths::findExecutable("apt").isEmpty()) {
        return "apt";
    }
    return "Unsupported";
}


bool Downloader::is_in_pacman_repo(const QString &package_name) {
    QProcess process;

    process.start("pacman", QStringList() << "-Si" << package_name);
    process.waitForFinished();
    return (process.exitCode() == 0);
}

bool Downloader::is_in_apt_repo(const QString &package_name) {
    QProcess process;

    process.start("apt-cache", QStringList() << "show" << package_name);
    process.waitForFinished();
    return (process.exitCode() == 0);
}


QStringList Downloader::read_package_list(bool standard_package_manager, QString package_manager) {
    QStringList result_list;
    QString type = (package_manager == "pacman") ? "pacman_list.txt" : "apt_list.txt";
    QString filepath_of_list = ":/lists/" + name_of_university + "/" + name_of_department + "/" + type;

    QFile file(filepath_of_list);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Critical Error: Could not open " << filepath_of_list;
        return result_list;
    }

    QTextStream in(&file);
	
    while (!in.atEnd()) {
        QString package = in.readLine().trimmed();
        if (!package.isEmpty()) {
            // Only check repo if necessary to save time
            bool exists = (package_manager == "pacman") ? is_in_pacman_repo(package) : is_in_apt_repo(package);
            if (exists) result_list.append(package);
        }
    }
    file.close();
    return result_list;
}

void Downloader::download_via_pacman(const QStringList &list_to_be_downloaded) {
    if (list_to_be_downloaded.isEmpty()) return;

    QMessageBox::warning(nullptr, "Update Advised", "Please run 'sudo pacman -Syu' first.");

    QProcess *proc = new QProcess(this);
    proc->setProcessChannelMode(QProcess::MergedChannels);

    QStringList args;
    args << "pacman" << "-S" << "--noconfirm" << "--needed" << list_to_be_downloaded;

    connect(proc, &QProcess::readyReadStandardOutput, [=]() {
        emit status_message(QString::fromUtf8(proc->readAllStandardOutput()));
    });

    connect(proc, &QProcess::finished, [=](int exitCode) {
        emit download_completed(exitCode == 0);
        proc->deleteLater();
    });

    proc->start("pkexec", args);
}

void Downloader::download_via_apt(const QStringList &list_to_be_downloaded) {
    if (list_to_be_downloaded.isEmpty()) {
        emit download_completed(false);
        return;
    }

    QProcess *proc = new QProcess(this);
    proc->setProcessChannelMode(QProcess::MergedChannels);

    QStringList args;
    args << "apt" << "install" << "-y" << list_to_be_downloaded;

    connect(proc, &QProcess::readyReadStandardOutput, [=]() {
        QString output = QString::fromUtf8(proc->readAllStandardOutput());
        emit status_message(output);
    });

    connect(proc, &QProcess::finished, [=](int exitCode) {
        if (exitCode == 0) {
            emit status_message("\n✓ All packages installed successfully.");
            emit download_completed(true);
        } else {
            emit status_message("\n✗ Error during installation.");
            emit download_completed(false);
        }
        proc->deleteLater();
    });

    proc->start("pkexec", args);
}
