#pragma once
#include <qwidget.h>
#include <qtabwidget.h>
#include "TradePanelTab.h"
#include "LogBrowser.h"
#include "PositionTable.h"
#include "OrderRecTab.h"
#include "TradeDataMessage.h"

class CTPTrader;

class AccountPage :public QWidget
{
	Q_OBJECT
public:
	AccountPage(CTPTrader* trader=nullptr,QWidget* parent = nullptr);
	QString GetAccountID() const;
	void SetCompleter();
	void SetThisAsAccountLogger() { _trader->SetCurrentLogger(logBrowser); }
	Sender GetTraderSender() const { return _trader->GetSender(); }
	void Log(const QString& str) { _trader->Log(str); }
private:
	TradePanelTab* tradePanel;
	LogBrowser* logBrowser;
	PositionTable* positionTable;
	OrderRecTab* orderRecTab;
	QLabel* lbAvailabel, *lbMargin, *lbDynamicEquity, *lbRiskDegree;
	CTPTrader* _trader;
private slots:
	void _setcompleter();
	void updatefundlabel(double, double);
public:
	void onUpdateOrderRec(const OrderSubmitRecord& rec);
	void onUpdateTradeRec(const OrderTradeRecord& rec);
	void onPositionInit(const std::unordered_map<std::string, AccountPosition>& pos);
	void onUpdateFrozenData(const std::string& instrument, TradeDirectionType direction, int change);
	void onStrategyInsertSuccess(int id);
	void onStrategyStatusChanged(int id, bool flag);
signals:
	void setCompleter();
	void tickArrived(const Tick& tick);
	void orderRecArrived(const OrderSubmitRecord& rec);
	void orderTradeArrived(const OrderTradeRecord& rec);
	void fundUpdateArrived(double, double);
};

