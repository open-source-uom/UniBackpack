// Author: Apostolos Chalis 2026 <achalis@csd.auth.gr>
/**
 * @file Downloader.hpp
 * @brief Checks and installs a department's package list.
 */
#ifndef DOWNLOADER_HPP
#define DOWNLOADER_HPP

#include "PackageManager.hpp"

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QString>
#include <QStringList>

#include <memory>

class QProcess;

/**
 * @brief Installs a department's package list with the given package manager.
 *
 * start() checks which listed packages are in the repositories and then installs
 * them through pkexec. Everything after start() runs asynchronously, and
 * finished() is emitted exactly once at the end, including on failure.
 */
class Downloader : public QObject {
    Q_OBJECT

public:
    /**
     * @brief Creates a downloader for one department.
     *
     * @param university University name, as used in the @c :/lists/ resource path.
     * @param department Department name, as used in the @c :/lists/ resource path.
     * @param manager Backend to check and install with. Must not be null. It keeps
     *        progress state, so only one Downloader may use it at a time.
     * @param parent Optional QObject parent.
     */
    Downloader(const QString &university, const QString &department,
               std::shared_ptr<PackageManager> manager, QObject *parent = nullptr);

    /**
     * @brief Starts the check, then the install. Returns immediately.
     *
     * If the department has no packages for this package manager, finished()
     * is emitted before this function returns.
     */
    void start();

    /**
     * @brief Stops the run if it is still safe to, and finishes it as cancelled.
     *
     * Possible while checking the packages and while pkexec waits for
     * authorization. Once the package manager runs as root it can no longer
     * be stopped, and stopping it could leave the package database broken,
     * so the install then continues and cancellable_changed(false) is emitted.
     */
    void cancel();

signals:
    /**
     * @brief Whether cancel() can still stop the run. It starts out true.
     */
    void cancellable_changed(bool cancellable);

    /**
     * @brief Overall progress of the install, from 0 to 99.
     */
    void progress_updated(int percentage);

    /**
     * @brief Text for the detailed log: complete lines of package manager output or own messages.
     */
    void log_message(const QString &message);

    /**
     * @brief A short, user-facing description of the current step.
     */
    void status_changed(const QString &message);

    /**
     * @brief Emitted once when the run ends.
     *
     * @param success true if the package manager finished with exit code 0.
     * @param error Translated reason for the failure; empty on success.
     */
    void finished(bool success, const QString &error);

private:
    void check_packages(const QHash<QString, QString> &software_by_package);
    void install(const QStringList &packages);

    void mark_package_manager_running();
    void handle_install_output(const QByteArray &data);
    void flush_install_output();
    void connect_start_failure(QProcess *process);

    void report(const QString &message);
    void fail(const QString &message);

    QString name_of_university;
    QString name_of_department;
    std::shared_ptr<PackageManager> package_manager;
    QString pending_output; ///< Install output after the last complete line
    QPointer<QProcess> current_process; ///< The query or install process, while it runs
    bool cancel_requested = false;
    bool package_manager_running = false; ///< The install printed output, so it runs as root
};

#endif // DOWNLOADER_HPP
