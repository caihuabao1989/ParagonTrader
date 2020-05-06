#include "StrategyBase.h"

void StrategyMgr::Init()
{
	t1 = std::thread(&StrategyMgr::MarketDataThread, this);
	t2 = std::thread(&StrategyMgr::TradeDataThread, this);
	t3 = std::thread(&StrategyMgr::QryThread, this);
	t4 = std::thread(&StrategyMgr::TimerThread, this);
}

void StrategyMgr::Join()
{
	t1.join();
	t2.join();
	t3.join();
	t4.join();
}

void StrategyMgr::Log(const std::string& str)
{
	_instructionSender.send(str);
}

bool StrategyMgr::InsertStrategy(StrategyBase* ptr)
{
	if (_strategies.find(ptr->GetID()) != _strategies.end())
		return false;
	_strategies.emplace(ptr->GetID(), ptr);
	ptr->SetManager(this);
	auto sub = ptr->GetSubscription();
	_instructionSender.send(sub);
	return true;
}

bool StrategyMgr::SetRunning(int id)
{
	auto iter = _strategies.find(id);
	if (iter == _strategies.end())
		return false;
	iter->second->SetRunning(true);
	return true;
}



int StrategyMgr::BuyOpen(const std::string& instrument, double price, int volume, int id)
{
	OrderInsertParam param(instrument, price, volume,
		TradeDirectionType::BUY,
		CombOffsetType::OPEN);
	param.UserID = id;
	param.RequestID = requestID;
	_instructionSender.send(param);
	return requestID++;
}

int StrategyMgr::BuyClose(const std::string& instrument, double price, int volume, int id)
{
	OrderInsertParam param(instrument, price, volume,
		TradeDirectionType::BUY,
		CombOffsetType::CLOSE);
	param.UserID = id;
	param.RequestID = requestID;
	_instructionSender.send(param);
	return requestID++;
}

int StrategyMgr::SellOpen(const std::string& instrument, double price, int volume, int id)
{
	OrderInsertParam param(instrument, price, volume,
		TradeDirectionType::SELL,
		CombOffsetType::OPEN);
	param.UserID = id;
	param.RequestID = requestID;
	_instructionSender.send(param);
	return requestID++;
}

int StrategyMgr::SellClose(const std::string& instrument, double price, int volume, int id)
{
	OrderInsertParam param(instrument, price, volume,
		TradeDirectionType::SELL,
		CombOffsetType::CLOSE);
	param.UserID = id;
	param.RequestID = requestID;
	_instructionSender.send(param);
	return requestID++;
}

int StrategyMgr::TimerBuyOpen(const std::string& instrument, double price, int volume, int id,
	int seconds, 
	ActionAfterDelete action)
{
	OrderInsertParam param(instrument, price, volume,
		TradeDirectionType::BUY,
		CombOffsetType::OPEN);
	param.UserID = id;
	param.RequestID = requestID;
	TimerOrderParam timerparam(param);
	timerparam.deleteCondition = OrderDeleteCondition::TIME_OUT;
	timerparam.timeOutSeconds = seconds;
	timerparam.action = action;
	TimerOrder order;
	order._param = timerparam;
	_instructionSender.send(order);
	return requestID++;
}

int StrategyMgr::TimerBuyOpen(const std::string& instrument, double price, int volume, int id,
	double priceSpread, 
	PriceType priceType, 
	ActionAfterDelete action)
{
	OrderInsertParam param(instrument, price, volume,
		TradeDirectionType::BUY,
		CombOffsetType::OPEN);
	param.UserID = id;
	param.RequestID = requestID;
	TimerOrderParam timerparam(param);
	timerparam.deleteCondition = OrderDeleteCondition::ADVERSE_PRICE_MOVEMENT;
	timerparam.priceMovement = priceSpread;
	timerparam.adversePriceType = priceType;
	timerparam.action = action;
	TimerOrder order;
	order._param = timerparam;
	_instructionSender.send(order);
	return requestID++;
}

int StrategyMgr::TimerBuyOpen(const std::string& instrument, double price, int volume, int id,
	int seconds, 
	double priceSpread, 
	PriceType priceType, 
	ActionAfterDelete action)
{
	OrderInsertParam param(instrument, price, volume,
		TradeDirectionType::BUY,
		CombOffsetType::OPEN);
	param.UserID = id;
	param.RequestID = requestID;
	TimerOrderParam timerparam(param);
	timerparam.deleteCondition = OrderDeleteCondition::TIME_OUT_AND_ADVERSE_PRICE_MOVEMENT;
	timerparam.timeOutSeconds = seconds;
	timerparam.action = action;
	timerparam.priceMovement = priceSpread;
	timerparam.adversePriceType = priceType;
	timerparam.action = action;
	TimerOrder order;
	order._param = timerparam;
	_instructionSender.send(order);
	return requestID++;
}

int StrategyMgr::TimerBuyClose(const std::string& instrument, double price, int volume, int id,
	int seconds,
	ActionAfterDelete action)
{
	OrderInsertParam param(instrument, price, volume,
		TradeDirectionType::BUY,
		CombOffsetType::CLOSE);
	param.UserID = id;
	param.RequestID = requestID;
	TimerOrderParam timerparam(param);
	timerparam.deleteCondition = OrderDeleteCondition::TIME_OUT;
	timerparam.timeOutSeconds = seconds;
	timerparam.action = action;
	TimerOrder order;
	order._param = timerparam;
	_instructionSender.send(order);
	return requestID++;
}

int StrategyMgr::TimerBuyClose(const std::string& instrument, double price, int volume, int id,
	double priceSpread,
	PriceType priceType,
	ActionAfterDelete action)
{
	OrderInsertParam param(instrument, price, volume,
		TradeDirectionType::BUY,
		CombOffsetType::CLOSE);
	param.UserID = id;
	param.RequestID = requestID;
	TimerOrderParam timerparam(param);
	timerparam.deleteCondition = OrderDeleteCondition::ADVERSE_PRICE_MOVEMENT;
	timerparam.priceMovement = priceSpread;
	timerparam.adversePriceType = priceType;
	timerparam.action = action;
	TimerOrder order;
	order._param = timerparam;
	_instructionSender.send(order);
	return requestID++;
}

int StrategyMgr::TimerBuyClose(const std::string& instrument, double price, int volume, int id,
	int seconds,
	double priceSpread,
	PriceType priceType,
	ActionAfterDelete action)
{
	OrderInsertParam param(instrument, price, volume,
		TradeDirectionType::BUY,
		CombOffsetType::CLOSE);
	param.UserID = id;
	param.RequestID = requestID;
	TimerOrderParam timerparam(param);
	timerparam.deleteCondition = OrderDeleteCondition::TIME_OUT_AND_ADVERSE_PRICE_MOVEMENT;
	timerparam.timeOutSeconds = seconds;
	timerparam.action = action;
	timerparam.priceMovement = priceSpread;
	timerparam.adversePriceType = priceType;
	timerparam.action = action;
	TimerOrder order;
	order._param = timerparam;
	_instructionSender.send(order);
	return requestID++;
}

int StrategyMgr::TimerSellOpen(const std::string& instrument, double price, int volume, int id,
	int seconds,
	ActionAfterDelete action)
{
	OrderInsertParam param(instrument, price, volume,
		TradeDirectionType::SELL,
		CombOffsetType::OPEN);
	param.UserID = id;
	param.RequestID = requestID;
	TimerOrderParam timerparam(param);
	timerparam.deleteCondition = OrderDeleteCondition::TIME_OUT;
	timerparam.timeOutSeconds = seconds;
	timerparam.action = action;
	TimerOrder order;
	order._param = timerparam;
	_instructionSender.send(order);
	return requestID++;
}

int StrategyMgr::TimerSellOpen(const std::string& instrument, double price, int volume, int id,
	double priceSpread,
	PriceType priceType,
	ActionAfterDelete action)
{
	OrderInsertParam param(instrument, price, volume,
		TradeDirectionType::SELL,
		CombOffsetType::OPEN);
	param.UserID = id;
	param.RequestID = requestID;
	TimerOrderParam timerparam(param);
	timerparam.deleteCondition = OrderDeleteCondition::ADVERSE_PRICE_MOVEMENT;
	timerparam.priceMovement = priceSpread;
	timerparam.adversePriceType = priceType;
	timerparam.action = action;
	TimerOrder order;
	order._param = timerparam;
	_instructionSender.send(order);
	return requestID++;
}

int StrategyMgr::TimerSellOpen(const std::string& instrument, double price, int volume, int id,
	int seconds,
	double priceSpread,
	PriceType priceType,
	ActionAfterDelete action)
{
	OrderInsertParam param(instrument, price, volume,
		TradeDirectionType::SELL,
		CombOffsetType::OPEN);
	param.UserID = id;
	param.RequestID = requestID;
	TimerOrderParam timerparam(param);
	timerparam.deleteCondition = OrderDeleteCondition::TIME_OUT_AND_ADVERSE_PRICE_MOVEMENT;
	timerparam.timeOutSeconds = seconds;
	timerparam.action = action;
	timerparam.priceMovement = priceSpread;
	timerparam.adversePriceType = priceType;
	timerparam.action = action;
	TimerOrder order;
	order._param = timerparam;
	_instructionSender.send(order);
	return requestID++;
}

int StrategyMgr::TimerSellClose(const std::string& instrument, double price, int volume, int id,
	int seconds,
	ActionAfterDelete action)
{
	OrderInsertParam param(instrument, price, volume,
		TradeDirectionType::SELL,
		CombOffsetType::CLOSE);
	param.UserID = id;
	param.RequestID = requestID;
	TimerOrderParam timerparam(param);
	timerparam.deleteCondition = OrderDeleteCondition::TIME_OUT;
	timerparam.timeOutSeconds = seconds;
	timerparam.action = action;
	TimerOrder order;
	order._param = timerparam;
	_instructionSender.send(order);
	return requestID++;
}

int StrategyMgr::TimerSellClose(const std::string& instrument, double price, int volume, int id,
	double priceSpread,
	PriceType priceType,
	ActionAfterDelete action)
{
	OrderInsertParam param(instrument, price, volume,
		TradeDirectionType::SELL,
		CombOffsetType::CLOSE);
	param.UserID = id;
	param.RequestID = requestID;
	TimerOrderParam timerparam(param);
	timerparam.deleteCondition = OrderDeleteCondition::ADVERSE_PRICE_MOVEMENT;
	timerparam.priceMovement = priceSpread;
	timerparam.adversePriceType = priceType;
	timerparam.action = action;
	TimerOrder order;
	order._param = timerparam;
	_instructionSender.send(order);
	return requestID++;
}

int StrategyMgr::TimerSellClose(const std::string& instrument, double price, int volume, int id,
	int seconds,
	double priceSpread,
	PriceType priceType,
	ActionAfterDelete action)
{
	OrderInsertParam param(instrument, price, volume,
		TradeDirectionType::SELL,
		CombOffsetType::CLOSE);
	param.UserID = id;
	param.RequestID = requestID;
	TimerOrderParam timerparam(param);
	timerparam.deleteCondition = OrderDeleteCondition::TIME_OUT_AND_ADVERSE_PRICE_MOVEMENT;
	timerparam.timeOutSeconds = seconds;
	timerparam.action = action;
	timerparam.priceMovement = priceSpread;
	timerparam.adversePriceType = priceType;
	timerparam.action = action;
	TimerOrder order;
	order._param = timerparam;
	_instructionSender.send(order);
	return requestID++;
}

void StrategyMgr::DeleteOrder(int orderRef)
{
	OrderDeleteParam param(orderRef);
	_instructionSender.send(param);
}

void StrategyMgr::DeleteOrder(int orderSysID, const std::string& exchange)
{
	OrderDeleteParamBySysID param(orderSysID, exchange);
	_instructionSender.send(param);
}

int StrategyMgr::LongPosition(const std::string& instrument) const
{
	std::lock_guard<std::mutex> lk(_PosMut);
	auto iter = _position.find(instrument);
	if (iter == _position.end())
		return 0;
	return iter->second.TotalLong;
}

int StrategyMgr::ShortPosition(const std::string& instrument) const
{
	std::lock_guard<std::mutex> lk(_PosMut);
	auto iter = _position.find(instrument);
	if (iter == _position.end())
		return 0;
	return iter->second.TotalShort;
}

AccountPosition StrategyMgr::GetPosition(const std::string& instrument) const
{
	std::lock_guard<std::mutex> lk(_PosMut);
	auto iter = _position.find(instrument);
	if (iter == _position.end())
		return AccountPosition();
	return iter->second;
}

double StrategyMgr::AvailableCash() const
{
	return _available.load();
}

double StrategyMgr::CurrentMarginUsed() const
{
	return _curMargin.load();
}

void StrategyMgr::MarketDataThread()
{
	while (true)
	{
		_marketDataReceiver.wait()
			.handle<Tick>([&](const Tick& tick)
				{
					std::lock_guard<std::mutex> lk(_strategyMut);
					for (const auto& stg : _strategies)
					{
						if (stg.second->IsRunning() &&
							_subscription[stg.first].find(tick.instrumentID) != _subscription[stg.first].end())
						{
							stg.second->onTick(tick);
						}
					}
				});
	}
}

void StrategyMgr::TradeDataThread()
{
	while (true)
	{
		_dataReceiver.wait()
			.handle<OrderSubmitRecord>([&](const OrderSubmitRecord& rec)
				{
					if (rec.UserID == -1)
						return;
					std::lock_guard<std::mutex> lk(_strategyMut);
					auto iter = _strategies.find(rec.UserID);
					if (iter != _strategies.end() && iter->second->IsRunning())
					{
						iter->second->onReturnOrder(rec);
					}
				})
			.handle<OrderTradeRecord>([&](const OrderTradeRecord& rec)
				{
					if (rec.UserID == -1)
						return;
					std::lock_guard<std::mutex> lk(_strategyMut);
					auto iter = _strategies.find(rec.UserID);
					if (iter != _strategies.end() && iter->second->IsRunning())
					{
						iter->second->onReturnTrade(rec);
					}
				});
	}
}

void StrategyMgr::QryThread()
{
	while (true)
	{
		_qryReceiver.wait()
			.handle<std::pair<double, double>>([&](const std::pair<double, double>& cash)
				{
					_available.store(cash.first);
					_curMargin.store(cash.second);
				})
			.handle<AccountPosition>([&](const AccountPosition& pos)
				{
					std::lock_guard<std::mutex> lk(_PosMut);
					_position[pos.InstrumentID] = pos;
				});
	}
}

void StrategyMgr::TimerThread()
{
	while (true)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
		for (const auto& stg : _strategies)
		{
			if (stg.second->IsRunning())
			{
				stg.second->onTimer();
			}
		}
	}
}

int StrategyBase::BuyOpen(const std::string& instrument, double price, int volume)
{
	return _mgr->BuyOpen(instrument, price, volume, GetID());
}

int StrategyBase::BuyClose(const std::string& instrument, double price, int volume)
{
	return _mgr->BuyClose(instrument, price, volume, GetID());
}

int StrategyBase::SellOpen(const std::string& instrument, double price, int volume)
{
	return _mgr->SellOpen(instrument, price, volume, GetID());
}

int StrategyBase::SellClose(const std::string& instrument, double price, int volume)
{
	return _mgr->SellClose(instrument, price, volume, GetID());
}

int StrategyBase::TimerBuyOpen(const std::string& instrument, double price, int volume, 
	int seconds, 
	ActionAfterDelete action)
{
	return _mgr->TimerBuyOpen(instrument, price, volume, GetID(), seconds, action);
}

int StrategyBase::TimerBuyOpen(const std::string& instrument, double price, int volume, 
	double priceSpread, 
	PriceType priceType, 
	ActionAfterDelete action)
{
	return _mgr->TimerBuyOpen(instrument, price, volume, GetID(), priceSpread, priceType, action);
}

int StrategyBase::TimerBuyOpen(const std::string& instrument, double price, int volume, 
	int seconds, 
	double priceSpread, 
	PriceType priceType, 
	ActionAfterDelete action)
{
	return _mgr->TimerBuyOpen(instrument, price, volume, GetID(), seconds, priceSpread, priceType, action);
}

int StrategyBase::TimerBuyClose(const std::string& instrument, double price, int volume,
	int seconds,
	ActionAfterDelete action)
{
	return _mgr->TimerBuyClose(instrument, price, volume, GetID(), seconds, action);
}

int StrategyBase::TimerBuyClose(const std::string& instrument, double price, int volume,
	double priceSpread,
	PriceType priceType,
	ActionAfterDelete action)
{
	return _mgr->TimerBuyClose(instrument, price, volume, GetID(), priceSpread, priceType, action);
}

int StrategyBase::TimerBuyClose(const std::string& instrument, double price, int volume,
	int seconds,
	double priceSpread,
	PriceType priceType,
	ActionAfterDelete action)
{
	return _mgr->TimerBuyClose(instrument, price, volume, GetID(), seconds, priceSpread, priceType, action);
}

int StrategyBase::TimerSellOpen(const std::string& instrument, double price, int volume,
	int seconds,
	ActionAfterDelete action)
{
	return _mgr->TimerSellOpen(instrument, price, volume, GetID(), seconds, action);
}

int StrategyBase::TimerSellOpen(const std::string& instrument, double price, int volume,
	double priceSpread,
	PriceType priceType,
	ActionAfterDelete action)
{
	return _mgr->TimerSellOpen(instrument, price, volume, GetID(), priceSpread, priceType, action);
}

int StrategyBase::TimerSellOpen(const std::string& instrument, double price, int volume,
	int seconds,
	double priceSpread,
	PriceType priceType,
	ActionAfterDelete action)
{
	return _mgr->TimerSellOpen(instrument, price, volume, GetID(), seconds, priceSpread, priceType, action);
}

int StrategyBase::TimerSellClose(const std::string& instrument, double price, int volume,
	int seconds,
	ActionAfterDelete action)
{
	return _mgr->TimerSellClose(instrument, price, volume, GetID(), seconds, action);
}

int StrategyBase::TimerSellClose(const std::string& instrument, double price, int volume,
	double priceSpread,
	PriceType priceType,
	ActionAfterDelete action)
{
	return _mgr->TimerSellClose(instrument, price, volume, GetID(), priceSpread, priceType, action);
}

int StrategyBase::TimerSellClose(const std::string& instrument, double price, int volume,
	int seconds,
	double priceSpread,
	PriceType priceType,
	ActionAfterDelete action)
{
	return _mgr->TimerSellClose(instrument, price, volume, GetID(), seconds, priceSpread, priceType, action);
}

void StrategyBase::DeleteOrder(int orderRef)
{
	_mgr->DeleteOrder(orderRef);
}

void StrategyBase::DeleteOrder(int orderSysID, const std::string& exchange)
{
	_mgr->DeleteOrder(orderSysID, exchange);
}

int StrategyBase::LongPosition(const std::string& instrument) const
{
	return _mgr->LongPosition(instrument);
}

int StrategyBase::ShortPosition(const std::string& instrument) const
{
	return _mgr->ShortPosition(instrument);
}

AccountPosition StrategyBase::GetPosition(const std::string& instrument) const
{
	return _mgr->GetPosition(instrument);
}

double StrategyBase::AvailableCash() const
{
	return _mgr->AvailableCash();
}

double StrategyBase::CurrentMarginUsed() const
{
	return _mgr->CurrentMarginUsed();
}
