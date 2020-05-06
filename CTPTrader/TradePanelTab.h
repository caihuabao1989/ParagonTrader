#pragma once
#include <qtabwidget.h>
#include "HandTradePage.h"
#include "SpecialTradePage.h"
#include "StrategyTradePage.h"
#include <array>

class AccountPage;
class TradePanelTab :public QTabWidget
{
	//Q_OBJECT
public:
	TradePanelTab(QWidget* parent);
	void SetCompleter();
	void ShowStrategy(int id);
	void SetStrategyStatus(int id, bool flag);
private:
	HandTradePage* _handTrade;
	SpecialTradePage* _specialTrade;
	StrategyTradePage* _strategyTrade;
	AccountPage* _page;
};