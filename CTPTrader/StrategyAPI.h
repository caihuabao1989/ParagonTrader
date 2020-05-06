#pragma once
#include "Message.h"
#include "TimerOrder.h"
#include "Account.h"


#ifdef _STRATEGY_
#define STRATEGY_API __declspec(dllexport)
#else
#define STRATEGY_API __declspec(dllimport)
#endif

class StrategyBase;

class STRATEGY_API StrategyAPI
{
public:
	static std::shared_ptr<StrategyAPI> CreateStrategyApi();

	//主程序调用函数，策略不应使用
	virtual Sender GetMarketDataSender() = 0;
	virtual Sender GetTradeDataSender() = 0;
	virtual Sender GetQryDataSender() = 0;
	virtual Sender GetTimerSender() = 0;
	virtual void SetReceiver(const Sender& sender) = 0;
	virtual void Init() = 0;
	virtual void Join() = 0;
	virtual void Detach() = 0;
	virtual void Release() = 0;
	virtual bool InsertStrategy(const std::shared_ptr<StrategyBase>& ptr) = 0;
	virtual bool SetRunning(int id, bool flag) = 0;
	virtual bool IsRunning(int id) = 0;

	//输出日志信息
	virtual void Log(const std::string& str) = 0;
	
	/*
	限价下单函数
	@instrument 合约代码
	@price      价格
	@volume     数量
	@id         策略ID（策略ID由策略自身维护，为系统识别策略的标识，用于正确向策略返回对应的成交回报等数据）
	@return     请求ID（请求ID为每笔报单在账户层面为策略报单生成的标识，与系统生成的orderRef形成对应，以便识别对应的报单成交回报）
	*/
	virtual int BuyOpen(const std::string& instrument, double price, int volume, int id) = 0;
	virtual int BuyClose(const std::string& instrument, double price, int volume, int id) = 0;
	virtual int SellOpen(const std::string& instrument, double price, int volume, int id) = 0;
	virtual int SellClose(const std::string& instrument, double price, int volume, int id) = 0;

	/*
	定时器下单函数：报单在未成交前提下，满足一定条件会被撤单，并补单（可选）
	条件1：指定时间后未成交;
	条件2：指定价格偏离报单价达到指定值。
	两条件可择一或全选
	撤单后：不补单/市价补单。
	*/
	virtual int TimerBuyOpen(const std::string& instrument, double price, int volume, int id,
		int seconds,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;
	virtual int TimerBuyOpen(const std::string& instrument, double price, int volume, int id,
		double priceSpread,
		PriceType priceType = PriceType::LAST,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;
	virtual int TimerBuyOpen(const std::string& instrument, double price, int volume, int id,
		int seconds,
		double priceSpread,
		PriceType priceType = PriceType::LAST,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;

	virtual int TimerBuyClose(const std::string& instrument, double price, int volume, int id,
		int seconds,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;
	virtual int TimerBuyClose(const std::string& instrument, double price, int volume, int id,
		double priceSpread,
		PriceType priceType = PriceType::LAST,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;
	virtual int TimerBuyClose(const std::string& instrument, double price, int volume, int id,
		int seconds,
		double priceSpread,
		PriceType priceType = PriceType::LAST,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;

	virtual int TimerSellOpen(const std::string& instrument, double price, int volume, int id,
		int seconds,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;
	virtual int TimerSellOpen(const std::string& instrument, double price, int volume, int id,
		double priceSpread,
		PriceType priceType = PriceType::LAST,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;
	virtual int TimerSellOpen(const std::string& instrument, double price, int volume, int id,
		int seconds,
		double priceSpread,
		PriceType priceType = PriceType::LAST,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;

	virtual int TimerSellClose(const std::string& instrument, double price, int volume, int id,
		int seconds,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;
	virtual int TimerSellClose(const std::string& instrument, double price, int volume, int id,
		double priceSpread,
		PriceType priceType = PriceType::LAST,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;
	virtual int TimerSellClose(const std::string& instrument, double price, int volume, int id,
		int seconds,
		double priceSpread,
		PriceType priceType = PriceType::LAST,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;

	/*
	撤单函数
	1、orderRef在报单回报中获得，与报单的请求ID对应(此orderRef为主程序生成,只要报单发出即存在)
	2、orderSysID为交易所生成，报单被拒等情况下，此字段为空,也无法用来撤单
	*/
	virtual void DeleteOrder(int orderRef) = 0;
	virtual void DeleteOrder(int orderSysID, const std::string& exchange) = 0;

	/*
	查询类接口
	仓位查询返回的是整个账户的仓位，且为主程序自身实时计算维持的仓位，但仍推荐策略自身计算各自仓位
	资金查询返回的是柜台返回的资金，每秒更新一次，不是实时
	*/
	virtual int LongPosition(const std::string& instrument) const = 0;
	virtual int ShortPosition(const std::string& instrument) const = 0;
	virtual AccountPosition GetPosition(const std::string& instrument) const = 0;
	virtual double AvailableCash() const = 0;
	virtual double CurrentMarginUsed() const = 0;
};