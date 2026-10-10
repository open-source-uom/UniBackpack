// Authors: Apostolos Chalis 2026 <achalis@csd.auth.gr>, Ioannis Michadasis 2026 <cs05415@uowm.gr>
#include "PackageManager.hpp"

#include <QDebug>
#include <QRegularExpression>
#include <QStandardPaths>

namespace {
    QSet<QString> captured_names(const QRegularExpression &re, const QString &text) {
        QSet<QString> names;
        QRegularExpressionMatchIterator it = re.globalMatch(text);
        while (it.hasNext())
            names.insert(it.next().captured(1));
        return names;
    }

    /// Maps @p done of @p total onto [first, first + 49], so each phase gets half the bar.
    int half_bar(int first, int done, int total) {
        return first + qMin(static_cast<int>((done * 50.0) / total), 49);
    }
}

std::unique_ptr<PackageManager> PackageManager::detect() {
    if (!QStandardPaths::findExecutable("pacman").isEmpty()) {
        qDebug() << "Pacman found.";
        return std::make_unique<PacmanBackend>();
    }
    if (!QStandardPaths::findExecutable("apt-get").isEmpty()) {
        qDebug() << "Apt found.";
        return std::make_unique<AptBackend>();
    }
    qDebug() << "No supported package manager found.";
    return nullptr;
}

QStringList PackageManager::install_environment() const {
    return {};
}

void PackageManager::begin_install(int package_count) {
    Q_UNUSED(package_count);
}

QString PackageManager::pre_install_warning() const {
    return {};
}

// ---- pacman ----

QString PacmanBackend::name() const {
    return "pacman";
}

QString PacmanBackend::query_program() const {
    return "pacman";
}

QStringList PacmanBackend::query_arguments(const QStringList &packages) const {
    return QStringList{"-Si"} + packages;
}

QSet<QString> PacmanBackend::parse_available(const QString &query_output) const {
    // One "Name            : pkgname" line per package found in the sync databases
    static const QRegularExpression name_re(R"(^Name\s*:\s*(\S+))", QRegularExpression::MultilineOption);
    return captured_names(name_re, query_output);
}

QStringList PacmanBackend::install_command(const QStringList &packages) const {
    return QStringList{"pacman", "-S", "--noconfirm", "--needed"} + packages;
}

void PacmanBackend::begin_install(int package_count) {
    // Replaced by the real total, dependencies included, once pacman prints it
    total = package_count;
    downloaded = 0;
    installed = 0;
}

std::optional<int> PacmanBackend::parse_progress(const QString &line) {
    // Without a terminal pacman prints plain lines instead of progress bars:
    //   "Packages (23) foo-1.0-1  bar-2.0-1 ..."
    //   " foo-1.0-1-x86_64 downloading..."   (pacman >= 6; older: "downloading foo...")
    //   "installing foo..."                  (may be prefixed with "(3/23) ")
    static const QRegularExpression total_re(R"(^Packages \((\d+)\))");
    static const QRegularExpression download_re(R"(\bdownloading\b)");
    static const QRegularExpression install_re(R"(^(?:\((\d+)/(\d+)\) )?(?:installing|upgrading|reinstalling) )");

    if (const QRegularExpressionMatch match = total_re.match(line); match.hasMatch()) {
        total = match.captured(1).toInt();
        return std::nullopt;
    }
    if (total <= 0)
        return std::nullopt;

    if (download_re.match(line).hasMatch())
        return half_bar(0, ++downloaded, total);

    if (const QRegularExpressionMatch match = install_re.match(line); match.hasMatch()) {
        if (!match.captured(1).isEmpty()) {
            installed = match.captured(1).toInt();
            total = qMax(total, match.captured(2).toInt());
        } else {
            ++installed;
        }
        return half_bar(50, installed, total);
    }
    return std::nullopt;
}

QString PacmanBackend::pre_install_warning() const {
    // pacman -S without a prior -Syu can leave the system partially upgraded
    // Qualified so lupdate files it under the same context that tr() uses at runtime
    return PackageManager::tr("It is advised to update your system before proceeding.\nRun sudo pacman -Syu");
}

// ---- apt ----

QString AptBackend::name() const {
    return "apt";
}

QString AptBackend::query_program() const {
    return "apt-cache";
}

QStringList AptBackend::query_arguments(const QStringList &packages) const {
    return QStringList{"show", "--no-all-versions"} + packages;
}

QSet<QString> AptBackend::parse_available(const QString &query_output) const {
    // One "Package: pkgname" stanza per installable package; purely virtual ones print none
    static const QRegularExpression package_re(R"(^Package:\s*(\S+))", QRegularExpression::MultilineOption);
    return captured_names(package_re, query_output);
}

QStringList AptBackend::install_command(const QStringList &packages) const {
    // apt-get rather than apt: apt's output is not meant for scripts and warns about it
    return QStringList{"apt-get", "install", "-y", "-o", "APT::Status-Fd=1"} + packages;
}

QStringList AptBackend::install_environment() const {
    return {"DEBIAN_FRONTEND=noninteractive"};
}

std::optional<int> AptBackend::parse_progress(const QString &line) {
    // "dlstatus:<n>:<percent>:<text>" while downloading, "pmstatus:<pkg>:<percent>:<text>" in dpkg
    const bool downloading = line.startsWith("dlstatus:");
    if (!downloading && !line.startsWith("pmstatus:"))
        return std::nullopt;

    const QStringList parts = line.split(':');
    if (parts.size() < 3)
        return std::nullopt;

    bool ok;
    const double percent = parts[2].toDouble(&ok);
    if (!ok)
        return std::nullopt;
    return half_bar(downloading ? 0 : 50, static_cast<int>(percent), 100);
}
