#include "StrategyTradePage.h"
#include "qboxlayout.h"
#include "AccountPage.h"
#include "TradeDataMessage.h"


StrategyTradePage::StrategyTradePage(QWidget* parent) :
	QWidget(parent),
	combStrategies(new QComboBox(this)),
	btnAddStrategy(new QPushButton(QStringLiteral("将该策略加入账户"),this)),
	strategyTable(new QTableWidget(this))
{
	auto hlay = new QHBoxLayout;
	auto vlay = new QVBoxLayout(this);
	QStringList headers;
	headers << QStringLiteral("状态") << QStringLiteral("策略ID") << QStringLiteral("策略名称");
	strategyTable->setColumnCount(3);
	strategyTable->setHorizontalHeaderLabels(headers);
	strategyTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
	strategyTable->setSelectionBehavior(QAbstractItemView::SelectRows);
	strategyTable->setSelectionMode(QAbstractItemView::SingleSelection);
	combStrategies->addItems(GetStrategyList());
	hlay->addWidget(combStrategies);
	hlay->addWidget(btnAddStrategy);
	vlay->addLayout(hlay);
	vlay->addWidget(strategyTable);
	connect(btnAddStrategy, SIGNAL(clicked()), this, SLOT(onBtnAddStrategyClicked()));
	connect(strategyTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(onStrategyDoubleClicked(int, int)));
}

QStringList StrategyTradePage::GetStrategyList()
{
	QStringList ret;
	_strategies = StrategyBase::GetStrategies();
	for (const auto& stg : _strategies)
	{
		ret << QString::number(stg->GetID()).append("-").append(QString::fromStdString(stg->GetName()));
	}
	return ret;
}

void StrategyTradePage::SetAccountPage(AccountPage* page)
{
	_page = page;
}

void StrategyTradePage::ShowStrategy(int id)
{
	for (const auto& stg : _strategies)
	{
		if (stg->GetID() == id)
		{
			int nrow = strategyTable->rowCount();
			strategyTable->insertRow(nrow);
			strategyTable->setItem(nrow, 0, new QTableWidgetItem(QStringLiteral("未启动")));
			strategyTable->setItem(nrow, 1, new QTableWidgetItem(QString::number(id)));
			strategyTable->setItem(nrow, 2, new QTableWidgetItem(QString::fromStdString(stg->GetName())));
		}
	}
}

void StrategyTradePage::SetStatus(int id, bool flag)
{
	int rows = strategyTable->rowCount();
	for (int i = 0; i < rows; ++i)
	{
		if (strategyTable->item(i, 1)->text().toInt() == id)
		{
			strategyTable->item(i, 0)->setText(flag ? QStringLiteral("运行中") : QStringLiteral("未启动"));
		}
	}
}

void StrategyTradePage::onStrategyDoubleClicked(int nrow, int ncol)
{
	int id = strategyTable->item(nrow, 1)->text().toInt();
	_page->GetTraderSender().send(StrategyChangeStatus(id));
}

void StrategyTradePage::onBtnAddStrategyClicked()
{
	auto choice = combStrategies->currentIndex();
	int id = _strategies[choice]->GetID();
	int rows = strategyTable->rowCount();
	for (int i = 0; i < rows; ++i)
	{
		if (strategyTable->item(i, 1)->text().toInt() == id)
		{
			_page->Log(QStringLiteral("[策略插入错误][不能重复插入策略.]"));
			return;
		}
	}
	_page->GetTraderSender().send(StrategyChoice(_strategies[choice]));
}
