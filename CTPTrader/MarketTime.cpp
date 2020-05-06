#include "MarketTime.h"
#include "OperationWindow.h"
#include <sstream>
#include <iostream>

std::shared_ptr<MarketTime> marketTime = std::make_shared<MarketTime>();
extern OperationWindow* mainWindow;

void MarketTime::SetStartLocalTime()
{
	_startLocal = chbDateTime::get_system_time().time();
}

void MarketTime::CalculateLags(CThostFtdcRspUserLoginField* pRspUserLogin)
{
    _lags[0] = (pRspUserLogin->SHFETime[0]=='-')?
        INT_MAX:
        (_startLocal - chbTime(pRspUserLogin->SHFETime));
    _lags[1] = (pRspUserLogin->DCETime[0] == '-') ? 
        INT_MAX : 
        (_startLocal - chbTime(pRspUserLogin->DCETime));
    _lags[2] = (pRspUserLogin->CZCETime[0] == '-') ? 
        INT_MAX : 
        (_startLocal - chbTime(pRspUserLogin->CZCETime));
    _lags[3] = (pRspUserLogin->FFEXTime[0] == '-') ? 
        INT_MAX : 
        (_startLocal - chbTime(pRspUserLogin->FFEXTime));
    _lags[4] = (pRspUserLogin->INETime[0] == '-') ? 
        INT_MAX : 
        (_startLocal - chbTime(pRspUserLogin->INETime));
}

void MarketTime::StartUpdateTime()
{
    t = std::thread(&MarketTime::UpdateTimeFunc, this);
}

void MarketTime::Join()
{
    t.join();
}

void MarketTime::Stop()
{
    _running = false;
}

void MarketTime::RegisterTimerSender(const Sender& sender)
{
    _timerSenders.emplace(sender);
}

chbTime MarketTime::GetLocalTime() const
{
    return chbDateTime::get_system_time().time();
}

chbTime MarketTime::GetMarketTime(ExchangeType exchange) const
{
    int lag = _lags[static_cast<int>(exchange)];
    if (lag == INT_MAX)
        return chbTime();
    else
        return GetLocalTime() - lag;
}

chbTime MarketTime::GetLoginMarketTime(ExchangeType exchange) const
{
    int lag = _lags[static_cast<int>(exchange)];
    if (lag == INT_MAX)
        return chbTime();
    else
        return chbTime(_startLocal) - lag;
}

void MarketTime::UpdateTimeFunc()
{
    _running = true;
    while (_running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        ExchangeTime tt;
        tt.SHFEtime = GetMarketTime(ExchangeType::SHFE);
        tt.DCEtime = GetMarketTime(ExchangeType::DCE);
        tt.CZCEtime = GetMarketTime(ExchangeType::CZCE);
        tt.CFFEXtime = GetMarketTime(ExchangeType::CFFEX);
        tt.INEtime = GetMarketTime(ExchangeType::INE);
        QStringList list;
        list << QString::fromStdString(tt.SHFEtime.time_str())
            << QString::fromStdString(tt.DCEtime.time_str())
            << QString::fromStdString(tt.CZCEtime.time_str())
            << QString::fromStdString(tt.CFFEXtime.time_str())
            << QString::fromStdString(tt.INEtime.time_str());
        mainWindow->UpdateTime(list);
        for (const auto& sender : _timerSenders)
            sender.send(tt);
    }
}

std::string GetExchangeTypeStr(ExchangeType exchange)
{
    //CFFEX、CZCE、DCE、INE、SHFE
    switch (exchange)
    {
    case ExchangeType::SHFE:
        return "SHFE";
    case ExchangeType::DCE:
        return "DCE";
    case ExchangeType::CZCE:
        return "CZCE";
    case ExchangeType::CFFEX:
        return "CFFEX";
    case ExchangeType::INE:
        return "INE";
    default:
        return "";
    }
}

ExchangeType GetExchangeTypeFromStr(const std::string& str)
{
    if (str == "SHFE")
        return ExchangeType::SHFE;
    else if (str == "DCE")
        return ExchangeType::DCE;
    else if (str == "CZCE")
        return ExchangeType::CZCE;
    else if (str == "CFFEX")
        return ExchangeType::CFFEX;
    else if (str == "INE")
        return ExchangeType::INE;
    else
        return ExchangeType::UNKNOWN;
}
