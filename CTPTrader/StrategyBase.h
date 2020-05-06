#pragma once

#include "StrategyAPI.h"
#include "MarketDataMessage.h"
#include "MarketTime.h"
#include <vector>


#ifdef _STRATEGYBASE_
#define STRATEGYBASE __declspec(dllexport)
#else
#define STRATEGYBASE __declspec(dllimport)
#endif

class STRATEGYBASE StrategyBase
{
public:
	static std::vector<std::shared_ptr<StrategyBase>> GetStrategies();
	void SetManager(StrategyAPI* mgr) { _api = mgr; }

	virtual std::vector<std::string> GetSubscription() const { return std::vector<std::string>(); };
	virtual void Init() {};
	virtual void Release() {};
	virtual void onTick(const Tick& tick) {};
	virtual void onBar(const Bar& bar) {};
	virtual void onTimer(const ExchangeTime& time) {};
	virtual void onReturnOrder(const OrderSubmitRecord& rec) {};
	virtual void onReturnTrade(const OrderTradeRecord& rec) {};
	virtual int GetID() const { return -1; };
	virtual std::string GetName() const { return ""; }
protected:
	StrategyAPI* _api;
};

