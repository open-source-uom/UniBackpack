// Author: Vagellis Sotiropoulos 2026 <vagellis.sotiropoulos@unios.com>
#ifndef CONFIGCHECKER_HPP
#define CONFIGCHECKER_HPP

#include <QObject>
#include <QString>
#include <QStringList>

//edw na kses psaxnei kai an ta vrei to leei ksa mou
class ConfigChecker : public QObject
{
	Q_OBJECT

	public:
		explicit ConfigChecker(QObject *parent = nullptr);

		// Absolute path to the expected config file: ~/.unios/academicConfig.json
		static QString expectedConfigPath();

		// Checks for the file and shows a system notification reporting
		// whether it was found. Emits scanFinished() afterwards.
		void scanAndNotify();

		// Performs the check only, with no UI side effects. Returns true
		// if the file exists at expectedConfigPath().
		bool findConfigFile() const;

	signals:
		void scanFinished(bool found, const QString &path);
};

#endif // CONFIGCHECKER_HPP
