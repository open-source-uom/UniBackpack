// Author: Apostolos Chalis 2026 <achalis@csd.auth.gr>
// Co-Author: Ioannis Michadasis
#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QStandardItemModel>
#include <QModelIndex>
#include <QTranslator>
#include <QHash>
#include <QStringList>

void applyTranslator(const QString &locale);

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

/**
 * @brief The application's main window.
 *
 * Shows the list of universities first. Clicking a university replaces the list
 * with its departments, and clicking a department installs that department's
 * software through a Downloader.
 *
 * While an installation runs, the window shows a progress bar, a one-line status
 * and an optional detailed output view. A language selector in the status bar
 * switches the UI between English and Greek at runtime.
 */
class MainWindow : public QMainWindow
{
    	Q_OBJECT
	
	public:
		/**
		 * @brief Builds the window and fills the university list.
		 *
		 * Universities are loaded from @c :/universities.json with Utils::loadUniversities().
		 * The language selector is added to the status bar and preset to the value
		 * stored in QSettings under "language", or English if nothing is stored.
		 *
		 * @param parent Parent widget, or @c nullptr for a top-level window.
		 */
    		MainWindow(QWidget *parent = nullptr);

		/// @brief Destroys the window and the UI generated from @c MainWindow.ui.
		~MainWindow(); 

		/**
		 * @brief Re-applies translations to the texts that @c retranslateUi() does not cover.
		 *
		 * These are the university names, the department names and "Back to Universities"
		 * entry when departments are shown, and the label of the show/hide details button.
		 * A name without a translation keeps its original text.
		 */
		void retranslate();

	protected:
		/**
		 * @brief Refreshes all visible texts when the application language changes.
		 *
		 * On QEvent::LanguageChange it calls the generated @c retranslateUi() and then retranslate().
		 * Every event is passed on to QMainWindow::changeEvent().
		 *
		 * @param event The state change event.
		 */
		void changeEvent(QEvent *event) override;
	
	private slots: 
		/**
		 * @brief Handles a click on the list, for both universities and departments.
		 *
		 * In the university list, it switches the view to the departments of the clicked university.
		 * In the department list, "Back to Universities" returns to the university list.
		 * Any other entry starts installing that department's packages with the detected
		 * package manager, or shows an error if none is supported.
		 *
		 * @param index Index of the clicked row in the model currently shown.
		 */
		void on_university_selection(const QModelIndex &index);

		/// @brief Shows or hides the detailed output view.
    	void toggle_output();

	private:
    		Ui::MainWindow *ui;

		QHash<QString, QStringList> departments_by_university;

		QStandardItemModel *university_model;
		QStandardItemModel *department_model;

		bool showing_universities;
		bool output_visible = false;
		QString current_university;
};

#endif // MAINWINDOW_HPP
