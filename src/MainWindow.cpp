// Author: Apostolos Chalis 2026 <achalis@csd.auth.gr>
// Co-Author: Ioannis Michadasis

#include "ui_MainWindow.h"
#include "MainWindow.hpp"
#include "Downloader.hpp"

#include <QDebug>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QIcon>
#include <QTextEdit>
#include <QCoreApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow) {
    
    ui->setupUi(this);

    university_model = new QStandardItemModel(this);
    department_model = new QStandardItemModel(this);

    QList<QPair<QString, QString>> universities = {
        {"Aristotle University of Thessaloniki", ":/icons/auth_logo.png"},
        {"University of Western Macedonia",      ":/icons/uowm_logo.png"},
        {"University of Macedonia",              ":/icons/uom_logo.png"}
    };

    for (const auto &[name, iconPath] : universities) {
        QStandardItem *item = new QStandardItem(QIcon(iconPath), name);
        university_model->appendRow(item);
    }

    ui->listView->setModel(university_model);
    showing_universities = true;

    ui->progressBar->setVisible(false);
    ui->statusLabel->setVisible(false);
    ui->outputView->setVisible(false);
    ui->showMoreButton->setVisible(false);

    connect(ui->listView, &QListView::clicked, this, &MainWindow::on_university_selection);
    connect(ui->showMoreButton, &QPushButton::clicked, this, &MainWindow::toggle_output);
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::toggle_output() {
    output_visible = !output_visible;
    ui->outputView->setVisible(output_visible);
    ui->showMoreButton->setText(output_visible ? "Hide details ▲" : "Show details ▼");
}

void MainWindow::on_university_selection(const QModelIndex &index) {
    if (showing_universities) {
        current_university = university_model->data(index, Qt::DisplayRole).toString();
        
        QStringList departments;
        departments << "Back to Universities";

        if (current_university == "Aristotle University of Thessaloniki") {
            departments << "Informatics" << "Physics";
        } else if (current_university == "University of Western Macedonia") {
            departments << "Informatics" << "Mechanical Engineering";
        } else if (current_university == "University of Macedonia") {
            departments << "Applied Informatics" << "Economics";
        }

        department_model->clear();
        for (const QString &dept : departments) {
            department_model->appendRow(new QStandardItem(dept));
        }
        
        ui->listView->setModel(department_model);
        showing_universities = false;

    } else {
        QString selectedDept = department_model->data(index, Qt::DisplayRole).toString();
        
        if (selectedDept == "Back to Universities") {
            ui->listView->setModel(university_model);
            showing_universities = true;
            return;
        }

        Downloader *downloader = new Downloader(current_university, selectedDept, this);
        QString package_manager = downloader->check_package_manager();

        if (package_manager != "Unsupported") {
            ui->listView->setEnabled(false);
            ui->outputView->clear();
            ui->progressBar->setMaximum(100);
            ui->progressBar->setValue(0);
            ui->progressBar->setFormat("%p%");
            ui->progressBar->setStyleSheet("");
            ui->progressBar->setVisible(true);
            ui->statusLabel->setText("Checking packages...");
            ui->statusLabel->setVisible(true);
            ui->showMoreButton->setVisible(true);

            // Connect signals
            connect(downloader, &Downloader::status_message, this, [=](const QString &msg) {
                ui->outputView->append(msg);

                if (msg.startsWith("Found:") || msg.startsWith("Adding:") ||
                    msg.startsWith("Checking") || msg.startsWith("Not found:") ||
                    msg.startsWith("Adding (non-standard):")) {
                    ui->statusLabel->setText(msg.trimmed());
                }

                for (const QString &line : msg.split('\n')) {
                    if (line.startsWith("dlstatus:") || line.startsWith("pmstatus:")) {
                        QStringList parts = line.split(':');
                        if (parts.size() >= 3) {
                            bool ok;
                            double percent = parts[2].toDouble(&ok);
                            if (ok) {
                                ui->progressBar->setMaximum(100);
                                ui->progressBar->setValue(static_cast<int>(percent));
                            }
                        }
                    }
                }
            });

            connect(downloader, &Downloader::download_completed, this, [=](bool success) {
                ui->listView->setEnabled(true);
                ui->progressBar->setMaximum(100);
                ui->progressBar->setValue(100);

                if (success) {
                    ui->statusLabel->setText("✓ Finished!");
                    ui->progressBar->setFormat("Done!");
                    ui->progressBar->setStyleSheet("QProgressBar::chunk { background-color: #4CAF50; }");
                } else {
                    ui->statusLabel->setText("✗ Installation failed.");
                    ui->progressBar->setFormat("Failed");
                    ui->progressBar->setStyleSheet("QProgressBar::chunk { background-color: #f44336; }");
                }
                downloader->deleteLater();
            });

            connect(downloader, &Downloader::progress_updated, this, [=](int percent) {
                ui->progressBar->setMaximum(100);
                ui->progressBar->setValue(percent);
            });

            // Read packages
            QStringList packages_to_download = downloader->read_package_list(true, package_manager);

            // Start download
            if (package_manager == "pacman") {
                downloader->download_via_pacman(packages_to_download);
            } else if (package_manager == "apt") {
                downloader->download_via_apt(packages_to_download);
            }
        } else {
            ui->statusLabel->setText("Error: Unsupported Package Manager");
            ui->statusLabel->setVisible(true);
        }
    }
}