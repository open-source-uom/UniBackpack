// Author: Apostolos Chalis 2026 <achalis@csd.auth.gr>
// Co-Author: Ioannis Michadasis
/**
 * @file MainWindow.hpp
 * @brief Main application window.
 */
#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include "utils/loadUniversities.hpp"

#include <QList>
#include <QMainWindow>
#include <QPointer>
#include <QString>

#include <functional>
#include <memory>

class Downloader;
class PackageManager;
class QModelIndex;
class QStandardItemModel;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief The application window.
 *
 * Lists the universities, then the selected university's departments, and
 * installs the chosen department's packages with a Downloader while showing
 * progress and the package manager's output.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief Builds the UI, loads the universities from @c :/universities.json
     *        and detects the system's package manager.
     */
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    /**
     * @brief Re-applies translated texts that are not part of the .ui form.
     *
     * Covers the university and department names, the status line, the
     * progress bar text and the details button.
     */
    void retranslate();

protected:
    /**
     * @brief Retranslates the whole window on QEvent::LanguageChange.
     */
    void changeEvent(QEvent *event) override;

    /**
     * @brief Keeps the window open once the package manager is installing.
     */
    void closeEvent(QCloseEvent *event) override;

private slots:
    void handle_item_clicked(const QModelIndex &index);
    void toggle_output();

private:
    enum class InstallState { Idle, Running, Succeeded, Failed };

    void setup_language_selector();
    void show_universities();
    void show_departments(const QString &university_name);
    void start_install(const QString &department);
    bool confirm_install();
    void finish_install(bool success, const QString &error);
    void set_status(std::function<QString()> text);
    const Utils::University *find_university(const QString &name) const;

    std::unique_ptr<Ui::MainWindow> ui;

    QList<Utils::University> universities;
    QStandardItemModel *university_model = nullptr;
    QStandardItemModel *department_model = nullptr;

    /// Detected once at startup; null if the system has no supported package manager.
    std::shared_ptr<PackageManager> package_manager;
    QPointer<Downloader> downloader;
    InstallState install_state = InstallState::Idle;

    /// Produces the status line in the current language; empty while no status is shown.
    std::function<QString()> status_text;

    bool showing_universities = true;
    bool output_visible = false;
    QString current_university;
};

#endif // MAINWINDOW_HPP
