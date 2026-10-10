// Authors: Apostolos Chalis 2026 <achalis@csd.auth.gr>, Ioannis Michadasis 2026 <cs05415@uowm.gr>
/**
 * @file PackageManager.hpp
 * @brief Backends for the system package managers UniBackpack can install with.
 */
#ifndef PACKAGE_MANAGER_HPP
#define PACKAGE_MANAGER_HPP

#include <QCoreApplication>
#include <QSet>
#include <QString>
#include <QStringList>

#include <memory>
#include <optional>

/**
 * @brief Everything that differs between the supported system package managers.
 *
 * Downloader drives the check/install flow and asks the backend for the
 * commands to run and how to interpret their output. All output given to a
 * backend comes from processes run with @c LC_ALL=C, so it is untranslated.
 */
class PackageManager {
    Q_DECLARE_TR_FUNCTIONS(PackageManager)

public:
    virtual ~PackageManager() = default;

    /**
     * @brief Returns a backend for the first supported package manager on PATH.
     *
     * pacman is preferred over apt when both are present.
     *
     * @return The backend, or nullptr if no supported package manager is installed.
     */
    static std::unique_ptr<PackageManager> detect();

    /**
     * @brief Command name of the package manager, e.g. "pacman".
     *
     * Also the key for this manager's package names in a department's package list.
     */
    virtual QString name() const = 0;

    /**
     * @brief Program that prints repository info for several packages in a single run.
     */
    virtual QString query_program() const = 0;

    /**
     * @brief Arguments for query_program() that look up all of @p packages.
     */
    virtual QStringList query_arguments(const QStringList &packages) const = 0;

    /**
     * @brief Extracts the available package names from the query output.
     *
     * @param query_output Standard output of query_program().
     * @return Names of the packages found in the repositories.
     */
    virtual QSet<QString> parse_available(const QString &query_output) const = 0;

    /**
     * @brief Command line, program first, that installs @p packages. It is run through pkexec.
     */
    virtual QStringList install_command(const QStringList &packages) const = 0;

    /**
     * @brief Extra @c NAME=value variables for the install command. Empty by default.
     *
     * pkexec clears the environment, so these are passed through @c env.
     */
    virtual QStringList install_environment() const;

    /**
     * @brief Resets progress-parsing state before an install of @p package_count packages.
     */
    virtual void begin_install(int package_count);

    /**
     * @brief Parses overall install progress from one line of install output.
     *
     * @param line A complete line of standard output, without the line break.
     * @return Progress from 0 to 99, or std::nullopt if the line carries no progress
     *         information. 100 is left for the caller once the install has finished.
     */
    virtual std::optional<int> parse_progress(const QString &line) = 0;

    /**
     * @brief Message to show the user before installing. Empty by default.
     */
    virtual QString pre_install_warning() const;
};

/**
 * @brief Backend for Arch-based systems (pacman).
 *
 * Downloads fill the first half of the progress bar and installs the second,
 * both counted against the "Packages (N)" total that pacman prints.
 */
class PacmanBackend : public PackageManager {
public:
    QString name() const override;
    QString query_program() const override;
    QStringList query_arguments(const QStringList &packages) const override;
    QSet<QString> parse_available(const QString &query_output) const override;
    QStringList install_command(const QStringList &packages) const override;
    void begin_install(int package_count) override;
    std::optional<int> parse_progress(const QString &line) override;
    QString pre_install_warning() const override;

private:
    int total = 0;
    int downloaded = 0;
    int installed = 0;
};

/**
 * @brief Backend for Debian-based systems (apt-get).
 *
 * Progress comes from the machine-readable status lines that apt-get writes
 * when run with @c APT::Status-Fd=1: downloads fill the first half of the bar
 * and dpkg the second.
 */
class AptBackend : public PackageManager {
public:
    QString name() const override;
    QString query_program() const override;
    QStringList query_arguments(const QStringList &packages) const override;
    QSet<QString> parse_available(const QString &query_output) const override;
    QStringList install_command(const QStringList &packages) const override;
    QStringList install_environment() const override;
    std::optional<int> parse_progress(const QString &line) override;
};

#endif // PACKAGE_MANAGER_HPP
