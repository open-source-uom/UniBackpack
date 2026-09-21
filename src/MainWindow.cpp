// Author: Apostolos Chalis 2026 <achalis@csd.auth.gr>
// Co-Author: Ioannis Michadasis
#include "ui_MainWindow.h"
#include "MainWindow.hpp"
#include "Downloader.hpp"
#include "Utilities/ConfigChecker.hpp"

#include <QCoreApplication>
#include <QSettings>
#include <QComboBox>
#include <QDebug>
#include <QStandardItemModel>
#include <QIcon>
#include <QTextEdit>

void applyTranslator(const QString &locale) {
    static QTranslator elTranslator;
    static bool installed = false;

    if (installed) {
        QCoreApplication::removeTranslator(&elTranslator);
        installed = false;
    }

    if (locale.startsWith("el")) {
        if (elTranslator.isEmpty()) // if no translation is found, the original English text is shown
            elTranslator.load(":/i18n/unibackpack_el.qm");
        if (!elTranslator.isEmpty()) {
            QCoreApplication::installTranslator(&elTranslator);
            installed = true;
        }
    }
}

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

    for (const auto &[key, iconPath] : universities) {
        QStandardItem *item = new QStandardItem(QIcon(iconPath), key);
        item->setData(key, Qt::UserRole);
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

    // Language selector in status bar
    QComboBox *langCombo = new QComboBox(this);
    langCombo->addItem("English", "en_US");
    langCombo->addItem("Ελληνικά", "el_GR");

    QSettings settings;
    QString currentLang = settings.value("language", "en_US").toString();
    int idx = langCombo->findData(currentLang);
    if (idx >= 0) {
        langCombo->blockSignals(true);
        langCombo->setCurrentIndex(idx);
        langCombo->blockSignals(false);
    }

    statusBar()->addPermanentWidget(langCombo);

    connect(langCombo, &QComboBox::currentIndexChanged, this, [=](int index) {
        QString lang = langCombo->itemData(index).toString();
        QSettings s;
        s.setValue("language", lang);
        applyTranslator(lang);
        QEvent ev(QEvent::LanguageChange);
        QCoreApplication::sendEvent(this, &ev);
    });

    retranslate();

    QMenu *toolsMenu = menuBar()->addMenu(tr("Tools"));
    QAction *checkConfigAction = toolsMenu->addAction(tr("Check for AcademicConfig.json"));
    connect(checkConfigAction, &QAction::triggered, this, [=]() {
        ConfigChecker *checker = new ConfigChecker(this);
        checker->scanAndNotify();
        checker->deleteLater();
    });
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
        current_university = university_model->data(index, Qt::UserRole).toString();

        QStringList departments;

        if (current_university == "Aristotle University of Thessaloniki") {
            departments << "Informatics" << "Physics";
        } else if (current_university == "University of Western Macedonia") {
            departments << "Informatics" << "Mechanical Engineering";
        } else if (current_university == "University of Macedonia") {
            departments << "Applied Informatics" << "Economics";
        }

        department_model->clear();
        
        QStandardItem *backItem = new QStandardItem(tr("Back to Universities"));
        backItem->setData("__back__", Qt::UserRole);
        department_model->appendRow(backItem);

        for (const QString &dept : departments) {
            QStandardItem *item = new QStandardItem(tr(dept.toUtf8().constData()));
            item->setData(dept, Qt::UserRole);
            department_model->appendRow(item);
        }

        ui->listView->setModel(department_model);
        showing_universities = false;

    } else {
        QString selectedDept = department_model->data(index, Qt::UserRole).toString();

        if (selectedDept == "__back__") {
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

            connect(downloader, &Downloader::status_message, this, [=](const QString &msg) {
                ui->outputView->append(msg);
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

            connect(downloader, &Downloader::status_update, this, [=](const QString &msg) {
                ui->statusLabel->setText(msg.trimmed());
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

            QStringList packages_to_download = downloader->read_package_list(true, package_manager);

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

void MainWindow::retranslate() {
    for (int i = 0; i < university_model->rowCount(); ++i) {
        QStandardItem *item = university_model->item(i);
        QString key = item->data(Qt::UserRole).toString();
        QString translated = tr(key.toUtf8().constData());
        item->setText(translated.isEmpty() ? key : translated);
    }

    if (!showing_universities) {
        for (int i = 0; i < department_model->rowCount(); ++i) {
            QStandardItem *item = department_model->item(i);
            QString key = item->data(Qt::UserRole).toString();
            if (key == "__back__") {
                item->setText(tr("Back to Universities"));
            } else {
                QString translated = tr(key.toUtf8().constData());
                item->setText(translated.isEmpty() ? key : translated);
            }
        }
    }

    ui->showMoreButton->setText(output_visible ? tr("Hide details ▲") : tr("Show details ▼"));
}

void MainWindow::changeEvent(QEvent *event) {
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        retranslate();
    }
    QMainWindow::changeEvent(event);
}