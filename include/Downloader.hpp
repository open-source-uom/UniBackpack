// Author: Apostolos Chalis 2026 <achalis@csd.auth.gr>
#ifndef DOWNLOADER_HPP
#define DOWNLOADER_HPP

#include <QObject>
#include <QString>
#include <QDebug>
#include <QStringList>

class Downloader : public QObject {
	Q_OBJECT

	private:
		QString name_of_university;
		QString name_of_department;

	public:
		explicit Downloader(const QString &university, const QString &department, QObject *parent = nullptr)
			: QObject(parent), name_of_university(university), name_of_department(department) {}

		QString check_package_manager();
		QStringList read_package_list(bool standard_package_manager, QString package_manager); 
		bool is_in_pacman_repo(const QString &package_name);
		bool is_in_apt_repo(const QString &package_name);

		void download_via_pacman(const QStringList &list_to_be_downloaded);
		void download_via_apt(const QStringList &list_to_be_downloaded);

	signals:
		void progress_updated(int percentage);
		void status_message(const QString &message);
		void download_completed(bool success);
};

#endif // DOWNLOADER_HPP
