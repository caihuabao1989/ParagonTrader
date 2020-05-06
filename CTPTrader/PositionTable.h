#pragma once
#include <qtablewidget.h>
#include "Account.h"
#include "TradeDataMessage.h"

class PositionTable :public QTableWidget
{
public:
	PositionTable(QWidget* parent = nullptr);
	void Init(const std::unordered_map<std::string, AccountPosition>& pos);
	void Update(const OrderTradeRecord& rec);
	void UpdateFrozenData(const std::string& instrument, TradeDirectionType direction,int change);
private:
	int Search(const QString& instrumentID, const QString& direction);
};