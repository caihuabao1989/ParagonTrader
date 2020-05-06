#include "MarketData.h"
#include "MarketDataMessage.h"
#include "InstrumentInfo.h"
#include "LogSystem.h"
#include "LoginDialog.h"
#include <qdebug.h>

std::shared_ptr<MarketData> marketData = std::make_shared<MarketData>();
extern std::unordered_map<std::string, InstrumentInfo> instrumentInfo;
extern LogSystem* globalLogger;

MarketData::MarketData():
	_MdSpi(new CustomMdSpi),
	_MdApi(CThostFtdcMdApi::CreateFtdcMdApi())
{
	_instructionThread = std::thread(&MarketData::InstructionRun, this);
	_marketDataThread = std::thread(&MarketData::MarketDataRun, this);
}

QStringList MarketData::GetBrokerIDList() const
{
	QStringList ret;
	for (const auto& brokerid : _brokerFrontAddr)
	{
		ret << QString::fromStdString(brokerid.first);
	}
	return ret;
}

void MarketData::Init()
{
	_MdApi->RegisterSpi(_MdSpi);
	for (const auto& item : LoginDialog::brokerInfo)
	{
		if (item["brokerName"] == _curBroker)
		{
			for (const auto& ip : item["MarketFront"])
			{
				auto rip = "tcp://" + ip.get<std::string>();
				_MdApi->RegisterFront(const_cast<char*>(rip.c_str()));
			}
		}
	}
	_MdApi->Init();	
}

bool MarketData::ReqUserLogin()
{
	CThostFtdcReqUserLoginField login;
	memset(&login, 0, sizeof(login));
	for (const auto& item : LoginDialog::brokerInfo)
	{
		if (item["brokerName"] == _curBroker)
		{
			strcpy_s(login.BrokerID, item["brokerID"].get<std::string>().c_str());
			break;
		}
	}
	static int requestId = 0;
	return !_MdApi->ReqUserLogin(&login, requestId++);
}

void MarketData::Join()
{
	_instructionThread.join();
	_marketDataThread.join();
}

Sender MarketData::GetSender()
{
	return _receiver;
}

void MarketData::RegisterSender(const Sender& sender)
{
	_marketDataSenders.emplace(sender);
}

void MarketData::SubScribeMarketData(const QStringList& instrumentlist) const
{
	if (instrumentlist.empty())
		return;
	else if (instrumentlist.size() == 1)
	{
		SubScribeMarketData(instrumentlist[0]);
		return;
	}
	auto tmp = new char* [instrumentlist.size()];
	size_t cnt = 0;
	std::unique_lock<std::mutex> lk(_subMut);
	for (const auto& instrument : instrumentlist)
	{
		auto instr = instrument.toStdString();
		auto iter = _subscription.find(instr);
		if (iter != _subscription.end())
		{
			iter->second += 1;
			continue;
		}
		if (instrumentInfo.find(instr) != instrumentInfo.end())
		{
			auto temp = new char[31];
			strcpy(temp, instr.c_str());
			tmp[cnt++] = temp;
		}
	}
	lk.unlock();
	_MdApi->SubscribeMarketData(tmp, cnt);
	for (size_t i = 0; i < cnt; ++i)
		delete[] tmp[i];
	delete[] tmp;
}


void MarketData::ReadBrokerFront()
{
	_brokerFrontAddr.emplace("SIMNOW-tradetime", std::make_pair( "9999","tcp://180.168.146.187:10110" ));
	_brokerFrontAddr.emplace("SIMNOW-nontradetime", std::make_pair( "9999","tcp://180.168.146.187:10131" ));
}

void MarketData::SubScribeMarketData(const QString& instrument) const
{
	SubScribeMarketData(instrument.toStdString());
}

void MarketData::SubScribeMarketData(const std::vector<std::string>& instrumentvec) const
{
	if (instrumentvec.empty())
		return;
	else if (instrumentvec.size() == 1)
	{
		SubScribeMarketData(instrumentvec[0]);
		return;
	}
	auto tmp = new char* [instrumentvec.size()];
	size_t cnt = 0;
	std::unique_lock<std::mutex> lk(_subMut);
	for (const auto& instrument : instrumentvec)
	{
		auto iter = _subscription.find(instrument);
		if (iter != _subscription.end())
		{
			iter->second += 1;
			continue;
		}
		if (instrumentInfo.find(instrument) != instrumentInfo.end())
		{
			auto temp = new char[31];
			strcpy(temp, instrument.c_str());
			tmp[cnt++] = temp;
		}
	}
	lk.unlock();
	_MdApi->SubscribeMarketData(tmp, cnt);
	for (size_t i = 0; i < cnt; ++i)
		delete[] tmp[i];
	delete[] tmp;
}

void MarketData::SubScribeMarketData(const std::string& instrument) const
{
	if (instrumentInfo.find(instrument) == instrumentInfo.end())
		return;
	std::unique_lock<std::mutex> lk(_subMut);
	auto iter = _subscription.find(instrument);
	if (iter != _subscription.end())
	{
		iter->second += 1;
		return;
	}
	lk.unlock();
	auto temp = new char[31];
	strcpy(temp, instrument.c_str());
	char** tmp = &temp;
	_MdApi->SubscribeMarketData(tmp, 1);
	delete[] temp;
}

void MarketData::UnSubScribeMarketData(const QStringList& instrumentlist) const
{
	if (instrumentlist.empty())
		return;
	else if (instrumentlist.size() == 1)
	{
		UnSubScribeMarketData(instrumentlist[0]);
		return;
	}
	auto tmp = new char* [instrumentlist.size()];
	size_t cnt = 0;
	std::unique_lock<std::mutex> lk(_subMut);
	for (const auto& instrument : instrumentlist)
	{
		auto instr = instrument.toStdString();
		auto iter = _subscription.find(instr);
		if (iter == _subscription.end())
			continue;
		else
		{
			if (iter->second > 1)
			{
				--iter->second;
				continue;
			}
			else
			{
				auto temp = new char[31];
				strcpy(temp, instr.c_str());
				tmp[cnt++] = temp;
			}
		}
	}
	lk.unlock();
	_MdApi->UnSubscribeMarketData(tmp, cnt);
	for (size_t i = 0; i < cnt; ++i)
		delete[] tmp[i];
	delete[] tmp;
}

void MarketData::UnSubScribeMarketData(const QString& instrument) const
{
	UnSubScribeMarketData(instrument.toStdString());
}

void MarketData::UnSubScribeMarketData(const std::vector<std::string>& instrumentvec) const
{
	if (instrumentvec.empty())
		return;
	else if (instrumentvec.size() == 1)
	{
		UnSubScribeMarketData(instrumentvec[0]);
		return;
	}
	auto tmp = new char* [instrumentvec.size()];
	size_t cnt = 0;
	std::unique_lock<std::mutex> lk(_subMut);
	for (const auto& instrument : instrumentvec)
	{
		auto iter = _subscription.find(instrument);
		if (iter == _subscription.end())
			continue;
		else
		{
			if (iter->second > 1)
			{
				--iter->second;
				continue;
			}
			else
			{
				auto temp = new char[31];
				strcpy(temp, instrument.c_str());
				tmp[cnt++] = temp;
			}
		}
	}
	lk.unlock();
	_MdApi->UnSubscribeMarketData(tmp, cnt);
	for (size_t i = 0; i < cnt; ++i)
		delete[] tmp[i];
	delete[] tmp;
}

void MarketData::UnSubScribeMarketData(const std::string& instrument) const
{
	if (instrumentInfo.find(instrument) == instrumentInfo.end())
		return;
	std::unique_lock<std::mutex> lk(_subMut);
	auto iter = _subscription.find(instrument);
	if (iter == _subscription.end())
		return;
	else
	{
		if (iter->second > 1)
		{
			--iter->second;
			return;
		}
		else
		{
			lk.unlock();
			auto temp = new char[31];
			strcpy(temp, instrument.c_str());
			char** tmp = &temp;
			_MdApi->UnSubscribeMarketData(tmp, 1);
			delete[] temp;
		}
	}
}

void MarketData::AddCurrentSubscription(const std::string& instrument)
{
	std::lock_guard<std::mutex> lk(_subMut);
	auto iter = _subscription.find(instrument);
	if (iter == _subscription.end())
	{
		_subscription.emplace(instrument, 1);
	}
}

void MarketData::RemoveCurrentSubscription(const std::string& instrument)
{
	std::lock_guard<std::mutex> lk(_subMut);
	auto iter = _subscription.find(instrument);
	if (iter != _subscription.end())
	{
		_subscription.erase(iter);
	}
}

void MarketData::InstructionRun()
{
	try
	{
		while (true)
		{
			_receiver.wait()
				.handle<BrokerChoice>([&](const BrokerChoice& choice)
					{
						SetBroker(choice.broker);
						Init();
					})
				.handle<SenderRegistration>([&](const SenderRegistration& sender)
					{
						RegisterSender(sender.sender);
					})
				.handle<Subscription<QStringList>>([&](const Subscription<QStringList>& sub)
					{
						SubScribeMarketData(sub.contents);
					})
				.handle<Subscription<std::vector<std::string>>>([&](const Subscription<std::vector<std::string>>& sub)
					{
						SubScribeMarketData(sub.contents);
					})
				.handle<Subscription<QString>>([&](const Subscription<QString>& sub)
					{
						SubScribeMarketData(sub.contents);
					})
				.handle<Subscription<std::string>>([&](const Subscription<std::string>& sub)
					{
						SubScribeMarketData(sub.contents);
					})
				.handle<UnSubscription<QStringList>>([&](const UnSubscription<QStringList>& unsub)
					{
						UnSubScribeMarketData(unsub.contents);
					})
				.handle<UnSubscription<std::vector<std::string>>>([&](const UnSubscription<std::vector<std::string>>& unsub)
					{
						UnSubScribeMarketData(unsub.contents);
					})
				.handle<UnSubscription<QString>>([&](const UnSubscription<QString>& unsub)
					{
						UnSubScribeMarketData(unsub.contents);
					})
				.handle<UnSubscription<std::string>>([&](const UnSubscription<std::string>& unsub)
					{
						UnSubScribeMarketData(unsub.contents);
					});
		}
	}
	catch (const Done&)
	{
		Sender(_dataReceiver).send(Done());
	}
}

void MarketData::MarketDataRun()
{
	try
	{
		while (true)
		{
			_dataReceiver.wait()
				.handle<Tick>([&](const Tick& tick)
					{
						for (const auto& sender : _marketDataSenders)
						{
							sender.send(tick);
						}
					});
		}
	}
	catch (const Done&)
	{
		for (const auto& sender : _marketDataSenders)
		{
			sender.send(Done());
		}
		if (_MdApi)
		{
			_MdApi->Release();
			_MdApi = nullptr;
		}
		if (_MdSpi)
		{
			delete _MdSpi;
			_MdSpi = nullptr;
		}
	}
}
