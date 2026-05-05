// Author: Apostolos Chalis 2026 <achalis@csd.auth.gr>
// Co-Author: Ioannis Michadasis

#include "ui_MainWindow.h"
#include "MainWindow.hpp"
#include "Downloader.hpp"

#include <QDebug>
#include <QProcess>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QIcon>

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

	connect(ui->listView, &QListView::clicked, this, &MainWindow::on_university_selection);
}

MainWindow::~MainWindow() {
	delete ui;
}

void MainWindow::on_university_selection(const QModelIndex &index) {
	if (showing_universities) {
		current_university = university_model->data(index, Qt::DisplayRole).toString();
		qDebug() << "Selected: " << current_university;

		QStringList departments;

		if (current_university == "Aristotle University of Thessaloniki") {
			departments << "Back to Universities" 
						<< "Informatics" 
						<< "Physics";
		} else if (current_university == "University of Western Macedonia") {
			departments << "Back to Universities" 
						<< "Informatics" 
						<< "Mechanical Engineering";
		} else if (current_university == "University of Macedonia") {
			departments << "Back to Universities" 
						<< "Applied Informatics" 
						<< "Economics";
		}

		department_model->clear();
		for (const QString &dept : departments) {
    		department_model->appendRow(new QStandardItem(dept));
		}
		ui->listView->setModel(department_model);
		showing_universities = false; 
	} else {
		if (index.row() == 0) {
			ui->listView->setModel(university_model);
			showing_universities = true;
		} else {
			QString selectedDept = department_model->data(index, Qt::DisplayRole).toString();			

			qDebug() << "Installing for department: " << selectedDept;
			Downloader *downloader = new Downloader(current_university, selectedDept, this);
			QString package_manager = downloader->check_package_manager();

			if(package_manager != "Unsupported") {
				QStringList packages_to_download = downloader->read_package_list(true, package_manager);

				if (package_manager == "pacman") {
					downloader->download_via_pacman(packages_to_download);
				} else if (package_manager == "apt") {
					downloader->download_via_apt(packages_to_download);
				}
			} else {
				qDebug() << "No supported package manager found. Cannot proceed with installation.";
			}

		}
	}
}
