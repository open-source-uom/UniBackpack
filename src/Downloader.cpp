// Author: Apostolos Chalis 2026 <achalis@csd.auth.gr>
#include "Downloader.hpp"

#include <QDebug> 
#include <QFile>
#include <QString> 
#include <QProcess> 
#include <QTextStream>
#include <QStringList>
#include <QStandardPaths>

QString Downloader::check_package_manager() {
	if (!QStandardPaths::findExecutable("pacman").isEmpty()){
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
	    qDebug() << "Package" << package_name << "is available in pacman repositories.";
        return true;
    } else {
	    qDebug() << "Package" << package_name << "is NOT available in pacman repositories.";
        return false;
    }
}

bool Downloader::is_in_apt_repo(const QString &package_name) {
    QProcess process;
    
    process.start("apt-cache", QStringList() << "show" << package_name);
    process.waitForFinished();

    if (process.exitCode() == 0) {
	    qDebug() << "Package" << package_name << "is available in apt repositories.";
        return true;
    } else {
	    qDebug() << "Package" << package_name << "is NOT available in apt repositories.";
        return false;
    }
}


QStringList Downloader::read_package_list(bool standard_package_manager, QString package_manager) {
	QStringList installable_with_standard_package_manager; 
	QStringList installable_with_non_standard_package_manager;

	// filtering the packages that can be installed with the standard package manager (etc: apt)
	if (standard_package_manager && package_manager == "pacman") {
		QString filepath_of_list = ":/lists/" + name_of_university + "/" + name_of_department + "/pacman_list.txt";

		qDebug() << "Reading package list from: " << filepath_of_list;
		
		QFile file(filepath_of_list);

		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            		qDebug() << "Critical Error: Could not open the file!" << file.errorString();
			return installable_with_standard_package_manager;
        	}

		QTextStream in(&file);

    		while (!in.atEnd()) {
        		QString package = in.readLine().trimmed();
        
        		if (!package.isEmpty() && is_in_pacman_repo(package)) {
				qDebug() << "Adding package to installable list: " << package;
            			installable_with_standard_package_manager.append(package);
        		}
    		}
		file.close(); 
		return installable_with_standard_package_manager;
	}

	if (!standard_package_manager && package_manager == "pacman") {
		QString filepath_of_list = ":/lists/" + name_of_university + "/" + name_of_department + "/pacman_list.txt";

		qDebug() << "Reading package list from: " << filepath_of_list;
		
		QFile file(filepath_of_list);

		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            		qDebug() << "Critical Error: Could not open the file!" << file.errorString();
			return installable_with_non_standard_package_manager;
        	}

		QTextStream in(&file);

    		while (!in.atEnd()) {
        		QString package = in.readLine().trimmed();
        
        		if (!package.isEmpty() && is_in_pacman_repo(package)) {
				qDebug() << "Adding package to installable list: " << package;
            			installable_with_non_standard_package_manager.append(package);
        		}
    		}
		file.close(); 
		return installable_with_non_standard_package_manager; 
	}

	if (standard_package_manager && package_manager == "apt") {
		QString filepath_of_list = ":/lists/" + name_of_university + "/" + name_of_department + "/apt_list.txt";

		qDebug() << "Reading package list from: " << filepath_of_list;
		
		QFile file(filepath_of_list);

		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            		qDebug() << "Critical Error: Could not open the file!" << file.errorString();
			return installable_with_standard_package_manager;
        	}

		QTextStream in(&file);

    		while (!in.atEnd()) {
        		QString package = in.readLine().trimmed();
        
        		if (!package.isEmpty() && is_in_apt_repo(package)) {
				qDebug() << "Adding package to installable list: " << package;
            			installable_with_standard_package_manager.append(package);
        		}
    		}
		file.close(); 
		return installable_with_standard_package_manager;
	}

	if (!standard_package_manager && package_manager == "apt") {
		QString filepath_of_list = ":/lists/" + name_of_university + "/" + name_of_department + "/apt_list.txt";

		qDebug() << "Reading package list from: " << filepath_of_list;
		
		QFile file(filepath_of_list);

		if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            		qDebug() << "Critical Error: Could not open the file!" << file.errorString();
			return installable_with_non_standard_package_manager;
        	}

		QTextStream in(&file);

    		while (!in.atEnd()) {
        		QString package = in.readLine().trimmed();
        
        		if (!package.isEmpty() && is_in_apt_repo(package)) {
				qDebug() << "Adding package to installable list: " << package;
            			installable_with_non_standard_package_manager.append(package);
        		}
    		}
		file.close(); 
		return installable_with_non_standard_package_manager; 
	}

	return installable_with_standard_package_manager;
}

void Downloader::download_via_pacman(const QStringList &list_to_be_downloaded) {
    // there is a problem with pacman -Syu though, so the user should be alerted about that
    QMessageBox::warning(
        nullptr,
        "Warning",
        "It is advised to update your system before proceeding."
        "\nRun pacman -Syu"
    );

    if (list_to_be_downloaded.isEmpty()) {
        qDebug() << "Download list is empty\nNothing to do.";
        return;
    }

	qDebug() << "Starting to download package list via pacman"; 

	QProcess download_process;
	QStringList command_structure;

	command_structure << "pacman" << "-S" << "--noconfirm" << "--needed"; 
	command_structure.append(list_to_be_downloaded);

	qDebug() << "Executing: pkexec" << command_structure.join(" ");

	download_process.start("pkexec", command_structure);
	download_process.waitForFinished(-1);

	qDebug() << download_process.readAllStandardOutput();

	if (download_process.exitCode() == 0) {
		qDebug() << "Package list downloaded via pacman" ;
	} else {
		qDebug() << "Error downloading packages via pacman. Exit code:" << download_process.exitCode();
		qDebug() << "Error output:" << download_process.readAllStandardError();
	}
}

void Downloader::download_via_apt(const QStringList &list_to_be_downloaded) {
	if (list_to_be_downloaded.isEmpty()) {
		qDebug() << "Download list is empty\nNothing to do."; 
		return; 
	}

	qDebug() << "Starting to download package list via apt";
}
