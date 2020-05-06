#include "TradePanelTab.h"
#include "AccountPage.h"

TradePanelTab::TradePanelTab(QWidget* parent) :
	QTabWidget(parent),
	_handTrade(new HandTradePage(this)),
	_specialTrade(new SpecialTradePage(this)),
	_strategyTrade(new StrategyTradePage(this)),
	_page(dynamic_cast<AccountPage*>(parent))
{
	insertTab(0, _handTrade, QStringLiteral("手工交易"));
	insertTab(1, _specialTrade, QStringLiteral("半自动交易"));
	insertTab(2, _strategyTrade, QStringLiteral("策略交易"));
	_handTrade->SetAccountPage(_page);
	_strategyTrade->SetAccountPage(_page);
}

void TradePanelTab::SetCompleter()
{
	_handTrade->SetCompleter();
}

void TradePanelTab::ShowStrategy(int id)
{
	_strategyTrade->ShowStrategy(id);
}

void TradePanelTab::SetStrategyStatus(int id, bool flag)
{
	_strategyTrade->SetStatus(id, flag);
}
