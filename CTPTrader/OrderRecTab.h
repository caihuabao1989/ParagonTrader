#pragma once
#include <qtabwidget.h>
#include <qtablewidget.h>
#include "TradeDataMessage.h"

class AccountPage;
class OrderRecTab :public QTabWidget
{
	Q_OBJECT
public:
	OrderRecTab(QWidget* parent = nullptr);
	void OrderRecUpdate(const OrderSubmitRecord& rec);
	void TradeRecUpdate(const OrderTradeRecord& rec);
	void SetAccountPage(AccountPage* page);
private:
	QTableWidget* orderRec,* tradeRec;
	AccountPage* _page = nullptr;
	void RowsMoveDown(QTableWidget*);
private slots:
	void onOrderRecRowDoubleClicked(int row, int col);
};