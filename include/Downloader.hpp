// Author: Apostolos Chalis 2026 <achalis@csd.auth.gr>
/**
 * @file Downloader.hpp
 * @brief Resolves and installs the software list of a university department.
 */
#ifndef DOWNLOADER_HPP
#define DOWNLOADER_HPP

#include <QObject>
#include <QString>
#include <QDebug>
#include <QStringList>

/**
 * @brief Installs the packages a university department needs, using the system package manager.
 *
 * Each department has a package list per supported package manager, bundled as a Qt resource at
 * @c :/lists/\<university\>/\<department\>/pacman_list.txt or @c apt_list.txt, with one package name per line.
 *
 * Typical use:
 * -# call check_package_manager() to find out which package manager is available,
 * -# call read_package_list() to get the packages that exist in its repositories,
 * -# pass the result to download_via_pacman() or download_via_apt().
 *
 * Progress and results are reported through signals, so connect to them before starting.
 */
class Downloader : public QObject {
	Q_OBJECT

	private:
		QString name_of_university;
		QString name_of_department;

	public:
		/**
		 * @brief Creates a downloader for one department of one university.
		 *
		 * The two names select the package list to read, so they must match
		 * the directory names under @c resources/lists exactly.
		 *
		 * @param university Name of the university, as stored in @c universities.json.
		 * @param department Name of the department within that university.
		 * @param parent Owning QObject, or @c nullptr.
		 */
		explicit Downloader(const QString &university, const QString &department, QObject *parent = nullptr)
			: QObject(parent), name_of_university(university), name_of_department(department) {}

		/**
		 * @brief Detects the package manager installed on the system.
		 *
		 * Looks for the @c pacman executable first and then for @c apt.
		 *
		 * @return @c "pacman", @c "apt", or @c "Unsupported" if neither is found.
		 */
		QString check_package_manager();

		/**
		 * @brief Reads the department's package list and filters it by repository availability.
		 *
		 * Every package in the list is looked up with is_in_pacman_repo() or is_in_apt_repo(),
		 * so this call blocks until all lookups have finished.
		 * status_message() and status_update() are emitted along the way.
		 *
		 * @param standard_package_manager If @c true, return the packages that are available
		 *        in the repositories of @p package_manager. If @c false, return the ones that are not.
		 * @param package_manager Either @c "pacman" or @c "apt", as returned by check_package_manager().
		 *
		 * @return The matching package names, in the order they appear in the list.
		 *         The result is empty if the list cannot be opened or @p package_manager is not supported.
		 */
		QStringList read_package_list(bool standard_package_manager, QString package_manager); 

		/**
		 * @brief Checks whether a package exists in the configured pacman repositories.
		 *
		 * Runs <tt>pacman -Si</tt> and waits for it to finish.
		 * Emits status_message() and status_update() with the outcome.
		 *
		 * @param package_name Name of the package to look up.
		 * @return @c true if pacman exits successfully, @c false otherwise.
		 */
		bool is_in_pacman_repo(const QString &package_name);

		/**
		 * @brief Checks whether a package exists in the configured apt repositories.
		 *
		 * Runs <tt>apt-cache show</tt> and waits for it to finish.
		 * Emits status_message() and status_update() with the outcome.
		 *
		 * @param package_name Name of the package to look up.
		 * @return @c true if the command exits successfully and its output
		 *         contains a @c "Package:" field, @c false otherwise.
		 */
		bool is_in_apt_repo(const QString &package_name);

		/**
		 * @brief Installs packages with pacman.
		 *
		 * First shows a modal warning advising the user to update the system.
		 * Then starts <tt>pkexec pacman -S --noconfirm --needed</tt> and returns without waiting for it.
		 * Output is forwarded through status_message(), progress through progress_updated(),
		 * and download_completed() is emitted when the process exits.
		 *
		 * If the list is empty, nothing is started and download_completed() is emitted with @c false.
		 *
		 * @param list_to_be_downloaded Names of the packages to install.
		 */
		void download_via_pacman(const QStringList &list_to_be_downloaded);

		/**
		 * @brief Installs packages with apt.
		 *
		 * Starts <tt>pkexec apt install -y</tt> with @c DEBIAN_FRONTEND=noninteractive
		 * and returns without waiting for it.
		 * Output is forwarded through status_message(), including apt's
		 * @c dlstatus and @c pmstatus progress lines (@c APT::Status-Fd=1).
		 * download_completed() is emitted when the process exits.
		 * progress_updated() is not emitted on this path.
		 *
		 * If the list is empty, nothing is started and download_completed() is emitted with @c false.
		 *
		 * @param list_to_be_downloaded Names of the packages to install.
		 */
		void download_via_apt(const QStringList &list_to_be_downloaded);

	signals:
		/**
		 * @brief Reports installation progress during download_via_pacman().
		 *
		 * @param percentage Overall progress from 0 to 99. Values below 50 cover
		 *        the download phase and values from 50 the install phase.
		 */
		void progress_updated(int percentage);

		/**
		 * @brief Carries detailed output for the log view.
		 *
		 * @param message A status line, or raw output of the package manager,
		 *        which may span several lines.
		 */
		void status_message(const QString &message);

		/**
		 * @brief Carries a short description of the current step for the status label.
		 *
		 * @param message A single translated line, e.g. "Found: gcc".
		 */
		void status_update(const QString &message);

		/**
		 * @brief Emitted once when an installation ends.
		 *
		 * @param success @c true if the package manager exited with code 0.
		 *        @c false if it failed or there was nothing to install.
		 */
		void download_completed(bool success);
};

#endif // DOWNLOADER_HPP
