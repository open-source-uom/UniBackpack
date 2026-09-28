// Author: <your name here>
#include "ConfigChecker.hpp"

#include <QDir>
#include <QFileInfo>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QDebug>

ConfigChecker::ConfigChecker(QObject *parent) : QObject(parent) {}

QString ConfigChecker::expectedConfigPath() {
	return QDir::homePath() + "/.unios/academicConfig.json";
}

bool ConfigChecker::findConfigFile() const {
	QFileInfo candidate(expectedConfigPath());
	return candidate.exists() && candidate.isFile();
}

void ConfigChecker::scanAndNotify() {
	const QString path = expectedConfigPath();
	const bool found = findConfigFile();

	const QString title = tr("UniBackpack");
	const QString message = found
		? tr("academicConfig.json was detected at:\n%1").arg(path)
		: tr("Paixtike malakia na kses \n%1").arg(path);

	if (QSystemTrayIcon::isSystemTrayAvailable()) {
		static QSystemTrayIcon trayIcon;
		trayIcon.setIcon(QIcon(":/icons/unibackpack.png"));
		trayIcon.show();
		trayIcon.showMessage(title, message,
			found ? QSystemTrayIcon::Information : QSystemTrayIcon::Warning, 5000);
	} else {
		// No tray support on this desktop environment - fall back to the console.
		qInfo().noquote() << title << ":" << message;
	}

	emit scanFinished(found, path);
}
