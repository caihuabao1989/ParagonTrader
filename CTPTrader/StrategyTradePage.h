#pragma once
#include <qwidget.h>
#include <qcombobox.h>
#include <qtablewidget.h>
#include <qpushbutton.h>


class AccountPage;
class StrategyBase;
class StrategyTradePage :public QWidget
{
	Q_OBJECT
public:
	StrategyTradePage(QWidget* parent = nullptr);
	QStringList GetStrategyList();
	void SetAccountPage(AccountPage* page);
	void ShowStrategy(int id);
	void SetStatus(int id, bool flag);
private:
	std::vector<std::shared_ptr<StrategyBase>> _strategies;
	AccountPage* _page;
	QComboBox* combStrategies;
	QPushButton* btnAddStrategy;
	QTableWidget* strategyTable;
private slots:
	void onBtnAddStrategyClicked();
	void onStrategyDoubleClicked(int nrow,int ncol);
};