#include "TradePanelTab.h"
#include "AccountPage.h"

TradePanelTab::TradePanelTab(QWidget* parent) :
	QTabWidget(parent),
	_handTrade(new HandTradePage(this)),
	_specialTrade(new SpecialTradePage(this)),
	_strategyTrade(new StrategyTradePage(this)),
	_page(dynamic_cast<AccountPage*>(parent))
{
	insertTab(0, _handTrade, QStringLiteral("�ֹ�����"));
	insertTab(1, _specialTrade, QStringLiteral("���Զ�����"));
	insertTab(2, _strategyTrade, QStringLiteral("���Խ���"));
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
