#pragma once
#include "CustomMdSpi.h"
#include "Message.h"
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <string>
#include <qstringlist.h>
#include <atomic>
#include <vector>
#include <mutex>
#include <thread>


class MarketData {
public:
	friend class CThostFtdcMdApi;
	friend class CustomMdSpi;
	MarketData();
	bool IsFrontConnected() const { return _isFrontConnected; }
	bool IsAccountLogin() const { return _isLogin; }
	QStringList GetBrokerIDList() const;
	void SetBroker(const QString& broker) { _curBroker = broker.toStdString(); }
	void Init();
	bool ReqUserLogin();
	void Join();
	Sender GetSender();
	void RegisterSender(const Sender&);
	

	//���ġ�ȡ����������
	void SubScribeMarketData(const QStringList& instrumentlist) const;
	void SubScribeMarketData(const QString& instrument) const;
	void SubScribeMarketData(const std::vector<std::string>& instrumentvec) const;
	void SubScribeMarketData(const std::string& instrument) const;
	void UnSubScribeMarketData(const QStringList& instrumentlist) const;
	void UnSubScribeMarketData(const QString& instrument) const;
	void UnSubScribeMarketData(const std::vector<std::string>& instrumentvec) const;
	void UnSubScribeMarketData(const std::string& instrument) const;

	//���¶����б�
	void AddCurrentSubscription(const std::string& instrument);
	void RemoveCurrentSubscription(const std::string& instrument);
	void AddCurrentSubscription(const QString& instrument) { AddCurrentSubscription(instrument.toStdString()); }
	void RemoveCurrentSubscription(const QString& instrument) { RemoveCurrentSubscription(instrument.toStdString()); }
private:
	//����ǰ���Ƿ�����
	std::atomic_bool _isFrontConnected = false;
	
	//�����Ƿ��½
	std::atomic_bool _isLogin = false;

	CustomMdSpi*     _MdSpi;
	CThostFtdcMdApi* _MdApi;

	//brokerID-frontIP
	std::unordered_map<std::string, std::pair<std::string,std::string>> _brokerFrontAddr;
	std::string _curBroker;

	
	mutable std::mutex _subMut;
	mutable std::unordered_map<std::string, int> _subscription;
	//std::unordered_set<std::string> _curSubscription;
	//�ֲֺ�Լ���붩��
	//std::unordered_set<std::string> _forceSubscription;

	void ReadBrokerFront();

	std::mutex _senderMut;
	Receiver _receiver;
	Receiver _dataReceiver;
	std::set<Sender> _marketDataSenders;

	std::thread _instructionThread, _marketDataThread;

	void InstructionRun();
	void MarketDataRun();
};