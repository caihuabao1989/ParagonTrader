#include "AccountPage.h"
#include "CTPTrader.h"
#include "MarketTime.h"

extern std::shared_ptr<MarketTime> marketTime;

AccountPage::AccountPage(CTPTrader* trader, QWidget* parent):
	QWidget(parent),
	lbAvailabel(new QLabel(this)),
	lbMargin(new  QLabel(this)),
	lbDynamicEquity(new QLabel(this)),
	lbRiskDegree(new QLabel(this)),
	tradePanel(new TradePanelTab(this)),
	logBrowser(new LogBrowser(this)),
	positionTable(new PositionTable(this)),
	orderRecTab(new OrderRecTab(this)),
	_trader(trader)
{
	auto hlay1 = new QHBoxLayout;
	auto hlay2 = new QHBoxLayout;
	auto glay = new QGridLayout;
	auto vlay = new QVBoxLayout(this);
	auto vlay1 = new QVBoxLayout;
	_trader->SetAccountPage(this);
	orderRecTab->SetAccountPage(this);
	glay->addWidget(lbAvailabel, 0, 0);
	glay->addWidget(lbMargin, 0, 1);
	glay->addWidget(lbDynamicEquity, 1, 0);
	glay->addWidget(lbRiskDegree, 1, 1);
	vlay1->addLayout(glay);
	vlay1->addWidget(logBrowser);
	hlay1->addWidget(tradePanel);
	hlay1->addLayout(vlay1);
	logBrowser->setFixedHeight(380);
	hlay2->addWidget(positionTable);
	hlay2->addWidget(orderRecTab);
	hlay2->setStretch(0, 3);
	hlay2->setStretch(1, 7);
	vlay->addLayout(hlay1);
	vlay->addLayout(hlay2);
	connect(this, SIGNAL(setCompleter()), this, SLOT(_setcompleter()));
	connect(this, SIGNAL(fundUpdateArrived(double, double)), this, SLOT(updatefundlabel(double, double)));
}

void AccountPage::_setcompleter()
{
	tradePanel->SetCompleter();
}

void AccountPage::updatefundlabel(double available, double margin)
{
	lbAvailabel->setText(QStringLiteral("可用:").append(QString::number(available,'f',2)));
	lbMargin->setText(QStringLiteral("占用保证金:").append(QString::number(margin, 'f', 2)));
	lbDynamicEquity->setText(QStringLiteral("动态权益:").append(QString::number(available + margin, 'f', 2)));
	lbRiskDegree->setText(QStringLiteral("风险度:").append(QString::number(margin*100 / (available + margin), 'f', 2)).append("%"));
}

void AccountPage::onUpdateOrderRec(const OrderSubmitRecord& rec)
{
	orderRecTab->OrderRecUpdate(rec);
}

QString AccountPage::GetAccountID() const
{
	return QString::fromStdString(_trader->GetAccountID());
}

void AccountPage::SetCompleter()
{
	//tradePanel->SetCompleter();
	emit setCompleter();
}

void AccountPage::onUpdateTradeRec(const OrderTradeRecord& rec)
{
	if(rec.tradedTime > marketTime->GetLoginMarketTime(GetExchangeTypeFromStr(rec.exchangeID)))
		positionTable->Update(rec);
    orderRecTab->TradeRecUpdate(rec);
}

void AccountPage::onPositionInit(const std::unordered_map<std::string, AccountPosition>& pos)
{
	positionTable->Init(pos);
}

void AccountPage::onUpdateFrozenData(const std::string& instrument, TradeDirectionType direction, int change)
{
	positionTable->UpdateFrozenData(instrument, direction, change);
}

void AccountPage::onStrategyInsertSuccess(int id)
{
	tradePanel->ShowStrategy(id);
}

void AccountPage::onStrategyStatusChanged(int id, bool flag)
{
	tradePanel->SetStrategyStatus(id, flag);
}

