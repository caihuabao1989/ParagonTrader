#pragma once
#include "TradeDataMessage.h"

enum class OrderDeleteCondition
{
	TIME_OUT = 0,
	ADVERSE_PRICE_MOVEMENT = 1,
	TIME_OUT_AND_ADVERSE_PRICE_MOVEMENT = 2
};

enum class PriceType { ASK = 0, LAST = 1, BID = 2 };
enum class ActionAfterDelete { NONE = 0, MARKET_ORDER = 1 };

struct TimerOrderParam
{
	OrderInsertParam baseParam;
	OrderDeleteCondition deleteCondition = OrderDeleteCondition::TIME_OUT;
	int timeOutSeconds = -1;
	PriceType adversePriceType = PriceType::LAST;
	double priceMovement = 2.0;
	ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER;

	TimerOrderParam() = default;
	TimerOrderParam(const OrderInsertParam& baseparam):
		baseParam(baseparam){}
	TimerOrderParam(const OrderInsertParam& baseparam,
		OrderDeleteCondition deletecondition,
		int seconds,
		PriceType adversepricetype,
		double pricemovement,
		ActionAfterDelete action):
		baseParam(baseparam),
		deleteCondition(deletecondition),
		timeOutSeconds(seconds),
		adversePriceType(adversepricetype),
		priceMovement(pricemovement),
		action(action){}

	TimerOrderParam(OrderDeleteCondition deletecondition,
		int seconds,
		PriceType adversepricetype,
		double pricemovement,
		ActionAfterDelete action) :
		deleteCondition(deletecondition),
		timeOutSeconds(seconds),
		adversePriceType(adversepricetype),
		priceMovement(pricemovement),
		action(action) {}
};

struct TimerOrder
{
	int _frontID, _sessionID, _orderRef;
	int _orderSysID;
	TimerOrderParam _param;
};
