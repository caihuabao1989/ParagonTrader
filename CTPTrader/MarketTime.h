#pragma once
#include "date.h"
#include "ThostFtdcUserApiStruct.h"
#include <array>
#include <thread>
#include <atomic>
#include <set>
#include "Message.h"

enum class ExchangeType { SHFE = 0, DCE, CZCE, CFFEX, INE, UNKNOWN };
struct ExchangeTime
{
	chbTime SHFEtime, DCEtime, CZCEtime, CFFEXtime, INEtime;
};

std::string GetExchangeTypeStr(ExchangeType exchange);
ExchangeType GetExchangeTypeFromStr(const std::string& str);

class MarketTime
{
public:
	void SetStartLocalTime();
	void CalculateLags(CThostFtdcRspUserLoginField* pRspUserLogin);
	void StartUpdateTime();
	void Join();
	void Stop();
	void RegisterTimerSender(const Sender& sender);
	chbTime GetLocalTime() const;
	chbTime GetMarketTime(ExchangeType exchange = ExchangeType::CFFEX) const;
	chbTime GetLoginMarketTime(ExchangeType exchange = ExchangeType::CFFEX) const;
private:
	chbTime _startLocal;
	std::array<int, 5> _lags = { 0 };
	int SHFElag, DCElag, CZCElag, FFEXlag, INElag;
	std::thread t;
	std::atomic_bool _running = false;
	std::set<Sender> _timerSenders;

	void UpdateTimeFunc();
};