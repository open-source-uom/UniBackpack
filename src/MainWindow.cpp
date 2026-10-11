// Author: Apostolos Chalis 2026 <achalis@csd.auth.gr>
// Co-Author: Ioannis Michadasis
#include "ui_MainWindow.h"
#include "MainWindow.hpp"
#include "Downloader.hpp"
#include "PackageManager.hpp"
#include "utils/language.hpp"
#include "utils/packageList.hpp"

#include <QCloseEvent>
#include <QComboBox>
#include <QIcon>
#include <QLocale>
#include <QMessageBox>
#include <QPushButton>
#include <QStandardItemModel>

#include <algorithm>

namespace {
    const QString success_chunk_style = QStringLiteral("QProgressBar::chunk { background-color: #4CAF50; }");
    const QString failure_chunk_style = QStringLiteral("QProgressBar::chunk { background-color: #f44336; }");
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(std::make_unique<Ui::MainWindow>()) {

    ui->setupUi(this);

    university_model = new QStandardItemModel(this);
    department_model = new QStandardItemModel(this);

    universities = Utils::loadUniversities(":/universities.json");
    for (const Utils::University &university : universities) {
        QStandardItem *item = new QStandardItem(QIcon(university.icon), university.name);
        item->setData(university.name, Qt::UserRole);
        university_model->appendRow(item);
    }

    ui->listView->setModel(university_model);

    ui->progressBar->setVisible(false);
    ui->statusLabel->setVisible(false);
    ui->outputView->setVisible(false);
    ui->showMoreButton->setVisible(false);

    connect(ui->listView, &QListView::clicked, this, &MainWindow::handle_item_clicked);
    connect(ui->backButton, &QPushButton::clicked, this, &MainWindow::show_universities);
    connect(ui->showMoreButton, &QPushButton::clicked, this, &MainWindow::toggle_output);
    connect(ui->cancelButton, &QPushButton::clicked, this, [this]() {
        if (downloader)
            downloader->cancel();
    });

    package_manager = PackageManager::detect();

    if (universities.isEmpty())
        set_status([this]() { return tr("Could not load the list of universities."); });
    else if (!package_manager)
        set_status([this]() { return tr("No supported package manager (pacman or apt) was found."); });

    setup_language_selector();
    retranslate();
}

MainWindow::~MainWindow() {
    // A running Downloader is deleted with this window's children, after ui is gone
    if (downloader)
        downloader->disconnect(this);
}

void MainWindow::setup_language_selector() {
    QComboBox *language_combo = new QComboBox(this);
    for (const Utils::Language &language : Utils::supportedLanguages())
        language_combo->addItem(language.native_name, language.code);

    // Select the language main() applied, which falls back to the system locale
    const QLocale::Language saved = QLocale(Utils::savedLanguage()).language();
    for (int i = 0; i < language_combo->count(); ++i) {
        if (QLocale(language_combo->itemData(i).toString()).language() == saved)
            language_combo->setCurrentIndex(i);
    }

    statusBar()->addPermanentWidget(language_combo);

    connect(language_combo, &QComboBox::currentIndexChanged, this, [this, language_combo](int index) {
        const QString language = language_combo->itemData(index).toString();
        Utils::saveLanguage(language);
        // Swapping the translator sends LanguageChange, which retranslates the .ui texts.
        // University names come from the JSON, so refresh them even if no translator changed.
        Utils::applyTranslator(language);
        retranslate();
    });
}

void MainWindow::toggle_output() {
    output_visible = !output_visible;
    ui->outputView->setVisible(output_visible);
    retranslate();
}

void MainWindow::handle_item_clicked(const QModelIndex &index) {
    const QString key = index.data(Qt::UserRole).toString();

    if (showing_universities)
        show_departments(key);
    else
        start_install(key);
}

void MainWindow::show_universities() {
    ui->listView->setModel(university_model);
    ui->backButton->setVisible(false);
    showing_universities = true;
}

void MainWindow::show_departments(const QString &university_name) {
    const Utils::University *university = find_university(university_name);
    if (!university)
        return;

    current_university = university->name;
    department_model->clear();

    // Departments without packages for this system's package manager cannot be installed
    for (const Utils::CatalogEntry &department : university->departments) {
        QStandardItem *item = new QStandardItem;
        item->setData(department.name, Qt::UserRole);
        item->setEnabled(package_manager
                         && Utils::hasPackagesFor(
                                Utils::loadPackageList(Utils::packageListPath(university->name, department.name)),
                                package_manager->name()));
        department_model->appendRow(item);
    }

    ui->listView->setModel(department_model);
    ui->backButton->setVisible(true);
    showing_universities = false;
    retranslate();
}

void MainWindow::start_install(const QString &department) {
    if (!package_manager || !confirm_install())
        return;

    install_state = InstallState::Running;
    ui->listView->setEnabled(false);
    ui->backButton->setEnabled(false);
    ui->outputView->clear();
    ui->progressBar->setMaximum(100);
    ui->progressBar->setValue(0);
    ui->progressBar->setStyleSheet("");
    ui->progressBar->setVisible(true);
    ui->showMoreButton->setVisible(true);
    ui->cancelButton->setEnabled(true);
    ui->cancelButton->setVisible(true);
    set_status([this]() { return tr("Checking packages..."); });
    retranslate();

    downloader = new Downloader(current_university, department, package_manager, this);

    connect(downloader, &Downloader::log_message, ui->outputView, &QPlainTextEdit::appendPlainText);
    connect(downloader, &Downloader::status_changed, this, [this](const QString &message) {
        // Already translated by the Downloader; stays in the language it was sent in
        set_status([message]() { return message; });
    });
    connect(downloader, &Downloader::progress_updated, ui->progressBar, &QProgressBar::setValue);
    connect(downloader, &Downloader::cancellable_changed, ui->cancelButton, &QPushButton::setEnabled);
    connect(downloader, &Downloader::finished, this, &MainWindow::finish_install);

    downloader->start();
}

bool MainWindow::confirm_install() {
    const QString warning = package_manager->pre_install_warning();
    if (warning.isEmpty())
        return true;

    QMessageBox box(QMessageBox::Warning, tr("Warning"), warning, QMessageBox::NoButton, this);
    QPushButton *continue_button = box.addButton(tr("Continue"), QMessageBox::AcceptRole);
    box.addButton(tr("Cancel"), QMessageBox::RejectRole);
    box.setDefaultButton(continue_button);
    box.exec();
    return box.clickedButton() == continue_button;
}

void MainWindow::finish_install(bool success, const QString &error) {
    install_state = success ? InstallState::Succeeded : InstallState::Failed;
    ui->listView->setEnabled(true);
    ui->backButton->setEnabled(true);
    ui->cancelButton->setVisible(false);
    ui->progressBar->setValue(100);
    ui->progressBar->setStyleSheet(success ? success_chunk_style : failure_chunk_style);

    if (success)
        set_status([this]() { return tr("✓ Finished!"); });
    else
        set_status([this, error]() { return tr("✗ Installation failed: %1").arg(error); });
    retranslate();

    if (downloader)
        downloader->deleteLater();
}

void MainWindow::set_status(std::function<QString()> text) {
    status_text = std::move(text);
    ui->statusLabel->setText(status_text().trimmed());
    ui->statusLabel->setVisible(true);
}

const Utils::University *MainWindow::find_university(const QString &name) const {
    const auto it = std::find_if(universities.cbegin(), universities.cend(),
                                 [&name](const Utils::University &university) { return university.name == name; });
    return it == universities.cend() ? nullptr : &*it;
}

void MainWindow::retranslate() {
    const QString language = Utils::savedLanguage();

    for (int row = 0; row < university_model->rowCount(); ++row) {
        QStandardItem *item = university_model->item(row);
        if (const Utils::University *university = find_university(item->data(Qt::UserRole).toString()))
            item->setText(university->display_name(language));
    }

    if (const Utils::University *university = find_university(current_university)) {
        for (int row = 0; row < department_model->rowCount(); ++row) {
            QStandardItem *item = department_model->item(row);
            const QString key = item->data(Qt::UserRole).toString();
            for (const Utils::CatalogEntry &department : university->departments) {
                if (department.name == key)
                    item->setText(department.display_name(language));
            }
            item->setToolTip(item->isEnabled() ? QString()
                                               : tr("No package list is available for this department yet."));
        }
    }

    if (status_text)
        ui->statusLabel->setText(status_text().trimmed());

    switch (install_state) {
    case InstallState::Succeeded:
        ui->progressBar->setFormat(tr("Done!"));
        break;
    case InstallState::Failed:
        ui->progressBar->setFormat(tr("Failed"));
        break;
    default:
        ui->progressBar->setFormat("%p%");
        break;
    }

    ui->showMoreButton->setText(output_visible ? tr("Hide details ▲") : tr("Show details ▼"));
    ui->cancelButton->setToolTip(tr("Once the package manager has started, stopping it could break your system."));
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        retranslate();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event) {
    // Interrupting apt or pacman can leave the package database broken. Before they
    // start, closing is a cancel: the processes are killed along with the Downloader.
    if (install_state == InstallState::Running && !ui->cancelButton->isEnabled()) {
        QMessageBox::information(this, tr("Installation in progress"),
                                 tr("Please wait until the installation finishes."));
        event->ignore();
        return;
    }
    QMainWindow::closeEvent(event);
}
