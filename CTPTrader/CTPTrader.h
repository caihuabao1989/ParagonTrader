#pragma once
#include "CustomTdSpi.h"
#include "Message.h"
#include "TradeDataMessage.h"
#include "LogSystem.h"
#include "Account.h"
#include "LoginDialog.h"
#include "StrategyBase.h"
#include "TimerOrder.h"
#include "StrategyAPI.h"
#include <atomic>
#include <unordered_map>
#include <map>
#include <unordered_set>

class AccountPage;
class CTPTrader
{
public:
	friend class CustomTdSpi;
	CTPTrader();
	bool IsFrontConnected() const { return _isFrontConnected; }
	bool IsAccountLogin() const { return _isLogin; }
	bool IsResumeFinished() const { return _isResumeFinished; }
	Sender GetSender() { return _receiver; }
	std::string GetBroker() const;
	std::string GetAccountID() const;
	QStringList GetBrokerIDList() const;
	void SetLoginDialog(LoginDialog* dialog) { _loginDialog = dialog; }
	void SetCurrentLogger(LogSystem* logger) { _curLogger = logger; }
	void SetAccountPage(AccountPage* page);
	void Log(const QString& str) const { _curLogger->Log(str); }
	void Log(const std::string& str) const { _curLogger->Log(str); }
	void SetBroker(const QString& broker);
	void SetAccount(const QString& account);
	void SetPassWord(const QString& password);
	void Init();
	void Join();
	void Detach();
	
	int OrderInsert(const std::string& instrument, double price, int volume, TradeDirectionType direction, CombOffsetType comboffset);
	int MarketOrderInsert(const std::string& instrument, int volume, TradeDirectionType direction, CombOffsetType comboffset);
	int OrderDelete(int orderref);
	int OrderDelete(int orderSysID, const std::string& exchangeID);
	void QryInstrumentInfo(const std::string& instrument);
	void QryPositionDetail(const std::string& instrument);
	void QryPosition(const std::string& instrument);
	void QryTradingAccount();
	

	bool ReqUserLogin();
	bool ReqAuthenticate();
	void ReqConfirmSettlementInfo();
	void ReqSubscription(const std::vector<std::string>& instruments);
private:
	LoginDialog* _loginDialog;
	LogSystem* _curLogger;
	AccountPage* _page;
	//交易前置是否连接
	std::atomic_bool _isFrontConnected = false;

	//交易账号是否登陆
	std::atomic_bool _isLogin = false;
	std::atomic_bool _isResumeFinished = false;

	CustomTdSpi*         _TdSpi;
	CThostFtdcTraderApi* _TdApi;

    int	_frontID;	
	int	_sessionID;	//会话编号
	int	_orderRef;	//报单引用

	//brokerID-frontIP
	static std::unordered_map<std::string, std::pair<std::string,std::string>> _brokerFrontAddr;
	std::string _curBroker;
	CThostFtdcReqUserLoginField _loginInfo;

	std::mutex _orderMut, _tradeMut, _timerMut, _strategyMut;
	std::unordered_map<int, OrderSubmitRecord> _orderSubmitRec;
	std::unordered_map<int, OrderTradeRecord> _orderTradeRec;
	std::unordered_map<int, TimerOrder> _timerOrderBook;
	std::unordered_set<int> _timerWaitingBook;
	std::unordered_map<std::string, AtomicTick> _curTick;

	Account _account;
	Receiver _receiver;
	Receiver _dataReceiver;
	Receiver _marketDataReceiver;

	int _nReqID_OrderInsert = 0;
	int _nReqID_OrderDelete = 0;
	int _nReqID_QryInstrumentInfo = 0;
	int _nReqID_QryPositionDetail = 0;
	int _nReqID_QryPosition = 0;
	int _nReqID_QryTradingAccount = 0;

	void ReadBrokerFront();
	bool ValidateOrderInsert(const std::string& instrument, double price, int volume, TradeDirectionType direction, CombOffsetType comboffset);
	bool ValidateOrderDelete(int orderref);
	

	std::thread _instructionThread, _tradeDataThread, _marketDataThread, _fundUpdateThread, _timerOrderThread;

	void InstructionRun();
	void TradeDataRun();
	void MarketDataRun();
	void UpdateFundRun();
	void TimerOrderRun();
	std::atomic_bool _updateFundRunning = true, _timerOrderRunning = true;
	std::shared_ptr<StrategyAPI> _strategy;
	std::unordered_map<int, int> _strategyRefs;
};