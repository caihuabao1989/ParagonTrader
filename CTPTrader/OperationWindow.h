#pragma once
#include <qmainwindow.h>
#include <qmenubar.h>
#include <qstatusbar.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include "LogSystem.h"
#include "AccountPage.h"
#include <qapplication.h>
#include <qevent.h>
#include <set>

class OperationWindow :public QMainWindow,public LogSystem
{
	Q_OBJECT
public:
	OperationWindow();
	void UpdateTime(const QStringList& timelist);
	void AddAccount(AccountPage* page);
	void SetApplication(QApplication* app);
	void OpenOperationWindow();
	void Log(const QString& str) override;
	void Log(const std::string& str) override;
private:
	std::mutex _logMut;
	QApplication* _app;
	QMenuBar* menuBar;
	QWidget* mainWidget;
	QLabel* lbLogger;
	QStatusBar* statusBar;
	QTabWidget* accountTab;
	QLabel* lbSHFE, * lbDCE, * lbCZCE, * lbFFEX, * lbINE, * lbSHFETime, * lbDCETime, * lbCZCETime, * lbFFEXTime, * lbINETime;
	std::set<CTPTrader*> _traders;
	void SetUpUi();
	void closeEvent(QCloseEvent* event) override;
signals:
	//void TimeUpdate(const QStringList& timelist);
	void openOperationWindow();
	void addAccount(void* page);
private slots:
	void onAddAccount(void* page);
	void onActionAddAccount();
};