#include "CTPTrader.h"
#include "InstrumentInfo.h"
#include "MarketData.h"
#include "MarketDataMessage.h"
#include "AccountPage.h"
#include "date.h"
#include <qdebug.h>
#include "OperationWindow.h"
#include "MarketTime.h"
#include "StrategyBase.h"

std::map<std::string, CTPTrader*> traders;
std::unordered_map<std::string, std::pair<std::string,std::string>> CTPTrader::_brokerFrontAddr;
extern std::unordered_map<std::string, InstrumentInfo> instrumentInfo;
extern std::shared_ptr<MarketData> marketData;
extern OperationWindow* mainWindow;
extern std::shared_ptr<MarketTime> marketTime;

CTPTrader::CTPTrader():
	_TdSpi(new CustomTdSpi(this)),
	_TdApi(CThostFtdcTraderApi::CreateFtdcTraderApi()),
	_account(this),
    _strategy(StrategyAPI::CreateStrategyApi())
{
    qDebug() << _strategy.get();
    memset(&_loginInfo, 0, sizeof(_loginInfo));
    _strategy->SetReceiver(_receiver);
    marketTime->RegisterTimerSender(_strategy->GetTimerSender());
    _instructionThread = std::thread(&CTPTrader::InstructionRun, this);
    _tradeDataThread = std::thread(&CTPTrader::TradeDataRun, this);
    _marketDataThread = std::thread(&CTPTrader::MarketDataRun, this);
    _fundUpdateThread = std::thread(&CTPTrader::UpdateFundRun, this);
    _timerOrderThread = std::thread(&CTPTrader::TimerOrderRun, this);
    _strategy->Init();
}

std::string CTPTrader::GetAccountID() const
{
	return _loginInfo.UserID;
}

QStringList CTPTrader::GetBrokerIDList() const
{
    QStringList ret;
    for (const auto& brokerid : _brokerFrontAddr)
    {
        ret << QString::fromStdString(brokerid.first);
    }
    return ret;
}

void CTPTrader::SetAccountPage(AccountPage* page)
{
   _page = page; 
}

void CTPTrader::SetBroker(const QString& broker)
{
    _curBroker = broker.toStdString();
    for (const auto& item : LoginDialog::brokerInfo)
    {
        if(item["brokerName"]==_curBroker)
            strcpy(_loginInfo.BrokerID, item["brokerID"].get<std::string>().c_str());
    }
}

void CTPTrader::SetAccount(const QString& account)
{
	strcpy(_loginInfo.UserID, account.toStdString().c_str());
}

void CTPTrader::SetPassWord(const QString& password)
{
	strcpy(_loginInfo.Password, password.toStdString().c_str());
}

void CTPTrader::Init()
{
    _TdApi->RegisterSpi(_TdSpi);
    _TdApi->SubscribePublicTopic(THOST_TERT_RESTART);
    _TdApi->SubscribePrivateTopic(THOST_TERT_RESTART);
    for (const auto& item : LoginDialog::brokerInfo)
    {
        if (item["brokerName"] == _curBroker)
        {
            for (const auto& ip : item["TradeFront"])
            {
                auto rip = "tcp://" + ip.get<std::string>();
                _TdApi->RegisterFront(const_cast<char*>(rip.c_str()));
            }
        }
    }
    _TdApi->Init();
}

void CTPTrader::Join()
{
    _instructionThread.join();
    _tradeDataThread.join();
    _marketDataThread.join();
    _fundUpdateThread.join();
    _timerOrderThread.join();
    //_TdApi->Join();
    _strategy->Join();
   // delete _TdSpi;
   // _TdApi->Release();
}

void CTPTrader::Detach()
{
    _instructionThread.detach();
    _tradeDataThread.detach();
    _marketDataThread.detach();
    _fundUpdateThread.detach();
    _timerOrderThread.detach();
    //_TdApi->Join();
    _strategy->Join();
}

int CTPTrader::OrderInsert(const std::string& instrument, double price, int volume, TradeDirectionType direction, CombOffsetType comboffset)
{
    if (!ValidateOrderInsert(instrument, price, volume, direction, comboffset))
        return -1;
    auto exchange = instrumentInfo[instrument].ExchangeId;
    CThostFtdcInputOrderField orderInsertReq;
    memset(&orderInsertReq, 0, sizeof(orderInsertReq));
    strcpy(orderInsertReq.BrokerID, _loginInfo.BrokerID);
    strcpy(orderInsertReq.InvestorID, _loginInfo.UserID);
    strcpy(orderInsertReq.InstrumentID, instrument.c_str());
    int tref = chbDateTime::get_system_time().time_num()*100;
    while (tref <= _orderRef)
        ++tref;
    _orderRef = tref;
    strcpy(orderInsertReq.OrderRef, std::to_string(tref).c_str());
    orderInsertReq.OrderPriceType = THOST_FTDC_OPT_LimitPrice;
    orderInsertReq.Direction = static_cast<char>(direction);
    orderInsertReq.CombOffsetFlag[0] = static_cast<char>(comboffset);
    orderInsertReq.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
    orderInsertReq.LimitPrice = price;
    orderInsertReq.VolumeTotalOriginal = volume;
    orderInsertReq.TimeCondition = THOST_FTDC_TC_GFD;
    orderInsertReq.VolumeCondition = THOST_FTDC_VC_AV;
    orderInsertReq.MinVolume = 1;
    orderInsertReq.ContingentCondition = THOST_FTDC_CC_Immediately;
    orderInsertReq.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    orderInsertReq.IsAutoSuspend = 0;
    orderInsertReq.UserForceClose = 0;
    strcpy(orderInsertReq.ExchangeID, exchange.c_str());
    int rt = _TdApi->ReqOrderInsert(&orderInsertReq, _nReqID_OrderInsert++);
    if (!rt)
    {
        OrderSubmitRecord torder;
        torder.FirstCreate(&orderInsertReq, _nReqID_OrderInsert-1);
        std::lock_guard<std::mutex> lk(_orderMut);
        _orderSubmitRec.emplace(_orderRef, torder);
        return _orderRef;
    }
    else
        return -2;
}

int CTPTrader::MarketOrderInsert(const std::string& instrument, int volume, TradeDirectionType direction, CombOffsetType comboffset)
{
    auto exchange = instrumentInfo[instrument].ExchangeId;
    CThostFtdcInputOrderField orderInsertReq;
    memset(&orderInsertReq, 0, sizeof(orderInsertReq));
    strcpy(orderInsertReq.BrokerID, _loginInfo.BrokerID);
    strcpy(orderInsertReq.InvestorID, _loginInfo.UserID);
    strcpy(orderInsertReq.InstrumentID, instrument.c_str());
    int tref = chbDateTime::get_system_time().time_num() * 100;
    while (tref <= _orderRef)
        ++tref;
    _orderRef = tref;
    strcpy(orderInsertReq.OrderRef, std::to_string(tref).c_str());
    orderInsertReq.OrderPriceType = THOST_FTDC_OPT_AnyPrice;
    orderInsertReq.Direction = static_cast<char>(direction);
    orderInsertReq.CombOffsetFlag[0] = static_cast<char>(comboffset);
    orderInsertReq.CombHedgeFlag[0] = THOST_FTDC_HF_Speculation;
    orderInsertReq.LimitPrice = 0;
    orderInsertReq.VolumeTotalOriginal = volume;
    orderInsertReq.TimeCondition = THOST_FTDC_TC_GFD;
    orderInsertReq.VolumeCondition = THOST_FTDC_VC_AV;
    orderInsertReq.MinVolume = 1;
    orderInsertReq.ContingentCondition = THOST_FTDC_CC_Immediately;
    orderInsertReq.ForceCloseReason = THOST_FTDC_FCC_NotForceClose;
    orderInsertReq.IsAutoSuspend = 0;
    orderInsertReq.UserForceClose = 0;
    strcpy(orderInsertReq.ExchangeID, exchange.c_str());

    int rt = _TdApi->ReqOrderInsert(&orderInsertReq, _nReqID_OrderInsert++);
    if (!rt)
    {
        OrderSubmitRecord torder;
        torder.FirstCreate(&orderInsertReq, _nReqID_OrderInsert - 1);
        std::lock_guard<std::mutex> lk(_orderMut);
        _orderSubmitRec.emplace(_orderRef, torder);
        return _orderRef;
    }
    else
        return -2;
}


int CTPTrader::OrderDelete(int orderref)
{
    if (!ValidateOrderDelete(orderref))
        return -1;
    std::unique_lock<std::mutex> lk(_orderMut);
    auto rec = _orderSubmitRec[orderref];
    lk.unlock();
    CThostFtdcInputOrderActionField orderActionReq;
    memset(&orderActionReq, 0, sizeof(orderActionReq));
    strcpy(orderActionReq.BrokerID, _loginInfo.BrokerID);
    strcpy(orderActionReq.InvestorID, _loginInfo.UserID);
    orderActionReq.OrderActionRef = rec.orderRef;
    strcpy(orderActionReq.OrderRef, std::to_string(rec.orderRef).c_str());
    orderActionReq.FrontID = rec.frontID;
    orderActionReq.SessionID = rec.sessionID;
    orderActionReq.ActionFlag = THOST_FTDC_AF_Delete;
    strcpy(orderActionReq.InstrumentID, rec.instrumentID.c_str());
    int rt = _TdApi->ReqOrderAction(&orderActionReq, _nReqID_OrderDelete++);
    if (!rt)
    {
        return orderActionReq.OrderActionRef;
    }
    else
    {
        return -2;
    }

}

int CTPTrader::OrderDelete(int orderSysID, const std::string& exchangeID)
{
    int tref = -1;
    std::string instr;
    std::unique_lock<std::mutex> lk(_orderMut);
    for(const auto& rec:_orderSubmitRec)
        if(!std::string(rec.second.orderSysID).empty()
            && std::stoi(rec.second.orderSysID) == orderSysID
            && rec.second.exchangeID == exchangeID)
        {
            tref = rec.first;
            instr = rec.second.instrumentID;
            break;
        }
    lk.unlock();
    if (tref == -1)
    {
        Log(QStringLiteral("[撤单错误][报单编号不存在.]").append("->OrderSysID:").append(QString::number(orderSysID))
            .append("->ExchangeID:").append(exchangeID.c_str()));
    }
    if (!ValidateOrderDelete(tref))
        return -1;

    CThostFtdcInputOrderActionField orderActionReq;
    memset(&orderActionReq, 0, sizeof(orderActionReq));
    strcpy(orderActionReq.BrokerID, _loginInfo.BrokerID);
    strcpy(orderActionReq.InvestorID, _loginInfo.UserID);
    orderActionReq.OrderActionRef = tref;
    strcpy(orderActionReq.ExchangeID, exchangeID.c_str());
    sprintf(orderActionReq.OrderSysID, "%12d", orderSysID);
    orderActionReq.ActionFlag = THOST_FTDC_AF_Delete;
    strcpy(orderActionReq.InstrumentID, instr.c_str());
    int rt = _TdApi->ReqOrderAction(&orderActionReq, _nReqID_OrderDelete++);
    if (!rt)
    {
        return orderActionReq.OrderActionRef;
    }
    else
    {
        return -2;
    }
}

void CTPTrader::QryInstrumentInfo(const std::string& instrument)
{
    CThostFtdcQryInstrumentField instrumentqry;
    memset(&instrumentqry, 0, sizeof(instrumentqry));
    strcpy_s(instrumentqry.InstrumentID, instrument.c_str());
    int rt = _TdApi->ReqQryInstrument(&instrumentqry, _nReqID_QryInstrumentInfo++);
    if (rt)
        Log(QStringLiteral("[合约信息查询请求发送失败]").append("->ReturnValue:").append(QString::number(rt)));
}

void CTPTrader::QryPositionDetail(const std::string& instrument)
{
    auto iter = instrumentInfo.find(instrument);
    if (iter == instrumentInfo.end() && !instrument.empty())
        return;
    CThostFtdcQryInvestorPositionDetailField field;
    memset(&field, 0, sizeof(field));
    strcpy(field.BrokerID, _loginInfo.BrokerID);
    strcpy(field.InvestorID, _loginInfo.UserID);
    strcpy(field.InstrumentID, instrument.c_str());
    strcpy(field.ExchangeID, instrument.empty()?"":iter->second.ExchangeId.c_str());
    int rt = _TdApi->ReqQryInvestorPositionDetail(&field, _nReqID_QryPositionDetail++);
    if (rt)
        Log(QStringLiteral("[持仓明细查询请求发送失败]").append("->ReturnValue:").append(QString::number(rt)));
}

void CTPTrader::QryPosition(const std::string& instrument)
{
    auto iter = instrumentInfo.find(instrument);
    if (iter == instrumentInfo.end() && !instrument.empty())
        return;
    CThostFtdcQryInvestorPositionField field;
    memset(&field, 0, sizeof(field));
    strcpy(field.BrokerID, _loginInfo.BrokerID);
    strcpy(field.InvestorID, _loginInfo.UserID);
    strcpy(field.InstrumentID, instrument.c_str());
    strcpy(field.ExchangeID, instrument.empty() ? "" : iter->second.ExchangeId.c_str());
    int rt = _TdApi->ReqQryInvestorPosition(&field, _nReqID_QryPosition++);
    if (rt)
    {
        --_nReqID_QryPosition;
        Log(QStringLiteral("[持仓查询请求发送失败]").append("->ReturnValue:").append(QString::number(rt)));
    }
}

void CTPTrader::QryTradingAccount()
{
    CThostFtdcQryTradingAccountField tradingAccountReq;
    memset(&tradingAccountReq, 0, sizeof(tradingAccountReq));
    strcpy(tradingAccountReq.BrokerID, _loginInfo.BrokerID);
    strcpy(tradingAccountReq.InvestorID, _loginInfo.UserID);
    int rt = _TdApi->ReqQryTradingAccount(&tradingAccountReq, _nReqID_QryTradingAccount++);
    if (rt)
        Log(QStringLiteral("[账户资金查询请求发送失败]").append("->ReturnValue:").append(QString::number(rt)));
}

bool CTPTrader::ReqUserLogin()
{
    static int requestId = 0;
    return !_TdApi->ReqUserLogin(&_loginInfo, requestId++);
}

bool CTPTrader::ReqAuthenticate()
{
    static int requestId = 0;
    CThostFtdcReqAuthenticateField pReqAuthenticateField;
    strcpy(pReqAuthenticateField.BrokerID, _loginInfo.BrokerID);
    strcpy(pReqAuthenticateField.UserID, _loginInfo.UserID);
    for (const auto& item : LoginDialog::brokerInfo)
    {
        if (item["brokerName"] == _curBroker)
        {
            strcpy(pReqAuthenticateField.AppID, item["AppID"].get<std::string>().c_str());
            strcpy(pReqAuthenticateField.AuthCode, item["AuthCode"].get<std::string>().c_str());
            break;
        }
    }
    return !_TdApi->ReqAuthenticate(&pReqAuthenticateField, requestId++);
}

void CTPTrader::ReqConfirmSettlementInfo()
{
    CThostFtdcSettlementInfoConfirmField settlementConfirmReq;
    memset(&settlementConfirmReq, 0, sizeof(settlementConfirmReq));
    strcpy(settlementConfirmReq.BrokerID, _loginInfo.BrokerID);
    strcpy(settlementConfirmReq.InvestorID, _loginInfo.UserID);
    static int requestID = 0;
    int rt = _TdApi->ReqSettlementInfoConfirm(&settlementConfirmReq, requestID++);
    if (rt)
        Log(QStringLiteral("[结算单确认请求发送失败]").append("->ReturnValue:").append(QString::number(rt)));
}

void CTPTrader::ReqSubscription(const std::vector<std::string>& instruments)
{
    marketData->GetSender().send(Subscription<std::vector<std::string>>(instruments));
}

void CTPTrader::ReadBrokerFront()
{
    _brokerFrontAddr.emplace("SIMNOW-tradetime", std::make_pair("9999","tcp://180.168.146.187:10100"));
    _brokerFrontAddr.emplace("SIMNOW-nontradetime", std::make_pair("9999","tcp://180.168.146.187:10130"));
}

bool CTPTrader::ValidateOrderInsert(const std::string& instrument, double price, int volume, TradeDirectionType direction, CombOffsetType comboffset)
{
    if (!IsAccountLogin())
    {
        Log(QStringLiteral("[拒绝报单][交易账户未登陆.]"));
        return false;
    }
    else if (price < 0 || volume <= 0)
    {
        Log(QStringLiteral("[拒绝报单][价格或数量不合法.]").append("->instrument:").append(QString::fromStdString(instrument)).append("->price:").append(QString::number(price))
            .append("->volume:").append(QString::number(volume)));
        return false;
    }
    auto iter = instrumentInfo.find(instrument);
    
    if (volume<iter->second.MinLimitOrderVol || volume>iter->second.MaxLimitOrderVol)
    {
        Log(QStringLiteral("[拒绝报单][数量超限.]").append("->instrument:").append(QString::fromStdString(instrument)).append("->OrderVolume:").append(QString::number(volume)).append("->MinLimitOrderVol:").append(QString::number(iter->second.MinLimitOrderVol))
            .append("->MaxLimitOrderVol:").append(QString::number(iter->second.MaxLimitOrderVol)));
        return false;
    }
    else if (comboffset == CombOffsetType::CLOSE || comboffset == CombOffsetType::CLOSE_YESTERDAY)
    {
        auto tpos = _account.GetCalculatedPosition(instrument);
        if (direction == TradeDirectionType::BUY)
        {
            if (iter->second.ExchangeId == "SHFE" && (tpos.PreviousShort - tpos.FrozenPreviousShort) < volume)
            {
                Log(QStringLiteral("[拒绝报单][可平昨日仓位不足]").append("->instrument:").append(QString::fromStdString(instrument))
                    .append("->PreviousShortPosition:").append(QString::number(tpos.PreviousShort))
                    .append("->FrozenPreviousShortPosition:").append(QString::number(tpos.FrozenPreviousShort))
                    .append("->OrderVolume:").append(QString::number(volume)));
                return false;
            }
            else if((tpos.TotalShort - tpos.FrozenTotalShort) < volume)
            {
                Log(QStringLiteral("[拒绝报单][可平仓位不足]").append("->instrument:").append(QString::fromStdString(instrument))
                    .append("->ShortPosition:").append(QString::number(tpos.TotalShort))
                    .append("->FrozenShortPosition:").append(QString::number(tpos.FrozenTotalShort))
                    .append("->OrderVolume:").append(QString::number(volume)));
                return false;
            }
        }
        else if (direction == TradeDirectionType::SELL)
        {
            if (iter->second.ExchangeId == "SHFE" && (tpos.PreviousLong - tpos.FrozenPreviousLong) < volume)
            {
                Log(QStringLiteral("[拒绝报单][可平昨日仓位不足]").append("->instrument:").append(QString::fromStdString(instrument))
                    .append("->PreviousLongPosition:").append(QString::number(tpos.PreviousLong))
                    .append("->FrozenPreviousLongPosition:").append(QString::number(tpos.FrozenPreviousLong))
                    .append("->OrderVolume:").append(QString::number(volume)));
                return false;
            }
            else if ((tpos.TotalLong - tpos.FrozenTotalLong) < volume)
            {
                Log(QStringLiteral("[拒绝报单][可平仓位不足]").append("->instrument:").append(QString::fromStdString(instrument))
                    .append("->LongPosition:").append(QString::number(tpos.TotalLong))
                    .append("->FrozenLongPosition:").append(QString::number(tpos.FrozenTotalLong))
                    .append("->OrderVolume:").append(QString::number(volume)));
                return false;
            }
        }
        
    }
    else if (comboffset == CombOffsetType::ClOSE_TODAY)
    {
        auto tpos = _account.GetCalculatedPosition(instrument);
        if (direction == TradeDirectionType::BUY && (tpos.TodayShort - tpos.FrozenTodayShort) < volume)
        {
            Log(QStringLiteral("[拒绝报单][可今日仓位不足]").append("->instrument:").append(QString::fromStdString(instrument))
                .append("->TodayShortPosition:").append(QString::number(tpos.TodayShort))
                .append("->FrozenTodayShortPosition:").append(QString::number(tpos.FrozenTodayShort))
                .append("->OrderVolume:").append(QString::number(volume)));
            return false;
        }
        else if (direction == TradeDirectionType::SELL && (tpos.TodayLong - tpos.FrozenTodayLong) < volume)
        {
            Log(QStringLiteral("[拒绝报单][可平今日仓位不足]").append("->instrument:").append(QString::fromStdString(instrument))
                .append("->TodayLongPosition:").append(QString::number(tpos.TodayLong))
                .append("->FrozenTodayLongPosition:").append(QString::number(tpos.FrozenTodayLong))
                .append("->OrderVolume:").append(QString::number(volume)));
            return false;
        }
    }
    return true;
}

bool CTPTrader::ValidateOrderDelete(int orderref)
{
    std::unique_lock<std::mutex> lk(_orderMut);
    auto iter = _orderSubmitRec.find(orderref);
    if (iter == _orderSubmitRec.end())
    {
        Log(QStringLiteral("[撤单错误][报单引用不存在.]").append("->OrderRef:").append(QString::number(orderref)));
        return false;
    }
    else
    {
        if (IsFinalTradeStatus(iter->second.tradeStatus))
        {
            Log(QStringLiteral("[撤单错误][报单状态无法撤单.]").append("->OrderTradeStatus:").append(TranslateOrderTradeStatus(iter->second.tradeStatus)));
            return false;
        }
        else
            return true;
        switch (iter->second.tradeStatus)
        {
        case OrderTradeStatus::AllTraded:
        case OrderTradeStatus::PartTradedNotQueueing:
        case OrderTradeStatus::NoTradeNotQueueing:
        case OrderTradeStatus::Canceled:
        {
            Log(QStringLiteral("[撤单错误][报单状态无法撤单.]").append("->OrderTradeStatus:").append(TranslateOrderTradeStatus(iter->second.tradeStatus)));
            return false;
        }
        default:
            return true;
        }
    }
}

void CTPTrader::InstructionRun()
{
    try
    {
        while (true)
        {
            _receiver.wait()
                .handle<TraderLoginInfo>([&](const TraderLoginInfo& info)
                    {
                        SetBroker(info.brokerID);
                        SetAccount(info.accountID);
                        SetPassWord(info.passWord);
                        if (!IsFrontConnected())
                            Init();
                        else
                            ReqUserLogin();
                    })
                .handle<TraderFrontConnected>([&](const TraderFrontConnected)
                    {
                        //ReqUserLogin();
                        ReqAuthenticate();
                    })
                .handle<AuthenticateSuccess>([&](const AuthenticateSuccess)
                    {
                        ReqUserLogin();
                    })
                .handle<LoginSuccess>([&](const LoginSuccess&)
                    {
                        traders.emplace(GetAccountID(), this);
                        mainWindow->AddAccount(_page);
                        marketData->RegisterSender(_marketDataReceiver);
                        _isResumeFinished = true;
                        ReqConfirmSettlementInfo();
                    })
                .handle<std::vector<std::string>>([&](const std::vector<std::string>& sub)
                    {
                        ReqSubscription(sub);
                    })
                .handle<OrderInsertParam>([&](const OrderInsertParam& param)
                    {
                        auto iter = instrumentInfo.find(param.instrumentID);
                        if (iter == instrumentInfo.end())
                        {
                            Log(QStringLiteral("[拒绝报单][合约ID不合法.]").append("->instrument:").append(QString::fromStdString(param.instrumentID)));
                            return;
                        }
                        if (iter->second.ExchangeId == "SHFE" && param.offset == CombOffsetType::CLOSE)
                        {
                            auto tpos = _account.GetCalculatedPosition(param.instrumentID);
                            int ps = 0, ts = 0;
                            if (param.direction == TradeDirectionType::BUY)
                            {
                                ps = tpos.PreviousShort - tpos.FrozenPreviousShort;
                                ts = tpos.TodayShort - tpos.FrozenTodayShort;
                            }
                            else if (param.direction == TradeDirectionType::SELL)
                            {
                                ps = tpos.PreviousLong - tpos.FrozenPreviousLong;
                                ts = tpos.TodayLong - tpos.FrozenTodayLong;
                            }
                            if (ps+ts < param.volume)
                            {
                                Log(QStringLiteral("[拒绝报单][可平仓位不足]").append("->instrument:").append(QString::fromStdString(param.instrumentID))
                                    .append("->ShortPosition:").append(QString::number(tpos.TotalShort))
                                    .append("->FrozenShortPosition:").append(QString::number(tpos.FrozenTotalShort))
                                    .append("->OrderVolume:").append(QString::number(param.volume)));
                                return;
                            }
                            if (ps >= param.volume)
                            {
                                int rt = OrderInsert(param.instrumentID, param.price, param.volume, param.direction, param.offset);
                                if (param.UserID != -1 && rt >= 0)
                                {
                                    std::lock_guard<std::mutex> lk(_strategyMut);
                                    _strategyRefs.emplace(rt, param.RequestID);
                                }
                            }
                            else if (!ps)
                            {
                                int rt= OrderInsert(param.instrumentID, param.price, param.volume, param.direction, CombOffsetType::ClOSE_TODAY);
                                if (param.UserID != -1 && rt >= 0)
                                {
                                    std::lock_guard<std::mutex> lk(_strategyMut);
                                    _strategyRefs.emplace(rt, param.RequestID);
                                }
                            }
                            else
                            {
                                int rt1 = OrderInsert(param.instrumentID, param.price, ps, param.direction, param.offset);
                                int rt2 = OrderInsert(param.instrumentID, param.price, param.volume-ps, param.direction, CombOffsetType::ClOSE_TODAY);
                                if (param.UserID != -1)
                                {
                                    std::lock_guard<std::mutex> lk(_strategyMut);
                                    if (rt1 >= 0) _strategyRefs.emplace(rt1, param.RequestID);
                                    if (rt2 >= 0) _strategyRefs.emplace(rt2, param.RequestID);
                                }
                            }
                           
                        }
                        int rt = OrderInsert(param.instrumentID, param.price, param.volume, param.direction, param.offset);
                        if (param.UserID != -1 && rt >= 0)
                        {
                            std::lock_guard<std::mutex> lk(_strategyMut);
                            _strategyRefs.emplace(rt, param.RequestID);
                        }
                    })
                .handle<MarketOrderInsertParam>([&](const MarketOrderInsertParam& param)
                    {
                        MarketOrderInsert(param.instrumentID, param.volume, param.direction, param.offset);
                    })
                .handle<OrderDeleteParam>([&](const OrderDeleteParam& param)
                    {
                        OrderDelete(param.orderRef);
                    })
                .handle<OrderDeleteParamBySysID>([&](const OrderDeleteParamBySysID& param)
                    {
                        OrderDelete(param.orderSysID, param.exchangeID);
                    })
                .handle<TimerOrder>([&](TimerOrder& param)
                    {
                        param._orderRef = OrderInsert(param._param.baseParam.instrumentID,
                            param._param.baseParam.price,
                            param._param.baseParam.volume,
                            param._param.baseParam.direction,
                            param._param.baseParam.offset);
                        _timerOrderBook.emplace(param._orderRef, param);
                        if (param._param.baseParam.UserID != -1 && param._orderRef >= 0)
                        {
                            std::lock_guard<std::mutex> lk(_strategyMut);
                            _strategyRefs.emplace(param._orderRef, param._param.baseParam.RequestID);
                        }
                    })
                .handle<std::string>([&](const std::string& str)
                    {
                        Log(QString::fromStdString(str));
                    })
                .handle<StrategyChoice>([&](const StrategyChoice& choice)
                    {
                        _strategy->InsertStrategy(choice.strategy);
                    })
                .handle<StrategyInsertSuccess>([&](const StrategyInsertSuccess& stg)
                    {
                        _page->onStrategyInsertSuccess(stg.id);
                    })
                .handle<StrategyChangeStatus>([&](const StrategyChangeStatus& stg)
                    {
                        _strategy->SetRunning(stg.id, !_strategy->IsRunning(stg.id));
                    })
                .handle<StrategyChangeStatusSuccess>([&](const StrategyChangeStatusSuccess& stg)
                    {
                        _page->onStrategyStatusChanged(stg.id, stg.flag);
                    });
        }
    }
    catch (const Done&)
    {
        Sender(_dataReceiver).send(Done());
    }
}

void CTPTrader::TradeDataRun()
{
    try
    {
        while (true)
        {
            _dataReceiver.wait()
                .handle<OrderSubmitRecord>([&](const OrderSubmitRecord& rec)
                    {
                        if (!(std::string(rec.orderSysID).empty() && rec.tradeStatus == OrderTradeStatus::Unknown))
                        {
                            std::unique_lock<std::mutex> lk(_orderMut);
                            auto iter = _orderSubmitRec.find(rec.orderRef);
                            if (iter == _orderSubmitRec.end())
                            {
                                _orderSubmitRec.emplace(rec.orderRef, rec);
                            }
                            else
                            {
                                iter->second.Update(rec);
                            }
                            lk.unlock();
                            if (IsFinalTradeStatus(rec.tradeStatus) && rec.unfilledVolume
                               /* && rec.submitTime > marketTime->GetLoginMarketTime(GetExchangeTypeFromStr(rec.exchangeID))*/)
                            {
                                if (rec.exchangeID == "SHFE"
                                    && (rec.combOffset == CombOffsetType::CLOSE || rec.combOffset == CombOffsetType::CLOSE_YESTERDAY))
                                    _account.SetFrozen(rec.instrumentID, -1*rec.unfilledVolume, rec.direction, true);
                                else if (rec.combOffset == CombOffsetType::CLOSE || rec.combOffset == CombOffsetType::ClOSE_TODAY)
                                {
                                    _account.SetFrozen(rec.instrumentID, -1 * rec.unfilledVolume, rec.direction);
                                    _page->onUpdateFrozenData(rec.instrumentID, rec.direction, -1 * rec.unfilledVolume);
                                }
                            }
                            _page->onUpdateOrderRec(rec);
                            std::lock_guard<std::mutex> lk0(_strategyMut);
                            if (_strategyRefs.find(rec.orderRef) != _strategyRefs.end())
                            {
                                OrderSubmitRecord trec(rec);
                                trec.RequestID = _strategyRefs.at(rec.orderRef);
                                _strategy->GetTradeDataSender().send(trec);
                            }
                        }
                        else if(rec.submitStatus!=OrderSubmitStatus::InsertRejected
                            /*&& rec.submitTime> marketTime->GetLoginMarketTime(GetExchangeTypeFromStr(rec.exchangeID))*/)
                        {
                            if (rec.exchangeID == "SHFE"
                                && (rec.combOffset == CombOffsetType::CLOSE || rec.combOffset == CombOffsetType::CLOSE_YESTERDAY))
                                _account.SetFrozen(rec.instrumentID, rec.orderVolume, rec.direction, true);
                            else if(rec.combOffset== CombOffsetType::CLOSE || rec.combOffset == CombOffsetType::ClOSE_TODAY)
                            {
                                _account.SetFrozen(rec.instrumentID, rec.orderVolume, rec.direction);
                                _page->onUpdateFrozenData(rec.instrumentID, rec.direction, rec.orderVolume);
                            }
                        }
                    })
                .handle<OrderTradeRecord>([&](const OrderTradeRecord& rec)
                    {
                        if (rec.tradedTime > marketTime->GetLoginMarketTime(GetExchangeTypeFromStr(rec.exchangeID)))
                        {
                            if (rec.exchangeID == "SHFE"
                                && (rec.combOffset == CombOffsetType::CLOSE || rec.combOffset == CombOffsetType::CLOSE_YESTERDAY))
                                _account.SetFrozen(rec.instrumentID, -1*rec.tradedVolume, rec.direction, true);
                            else
                                _account.SetFrozen(rec.instrumentID, -1*rec.tradedVolume, rec.direction);
                            std::unique_lock<std::mutex> lk(_account._cposMut);
                            auto& trec = _account._calculatedPos[rec.instrumentID];
                            if (rec.combOffset == CombOffsetType::OPEN)
                            {
                                if (rec.direction == TradeDirectionType::BUY)
                                {
                                    trec.TodayLong += rec.tradedVolume;
                                    trec.TotalLong += rec.tradedVolume;
                                }
                                else
                                {
                                    trec.TodayShort += rec.tradedVolume;
                                    trec.TotalShort += rec.tradedVolume;
                                }
                            }
                            else
                            {
                                if (rec.direction == TradeDirectionType::SELL)
                                {
                                    if (trec.TodayLong > rec.tradedVolume)
                                        trec.TodayLong -= rec.tradedVolume;
                                    else
                                    {
                                        trec.PreviousLong -= (rec.tradedVolume - trec.TodayLong);
                                        trec.TodayLong = 0;
                                    }
                                    trec.TotalLong -= rec.tradedVolume;
                                }
                                else
                                {
                                    if (trec.TodayShort > rec.tradedVolume)
                                        trec.TodayShort -= rec.tradedVolume;
                                    else
                                    {
                                        trec.PreviousShort -= (rec.tradedVolume - trec.TodayShort);
                                        trec.TodayShort = 0;
                                    }
                                    trec.TotalShort -= rec.tradedVolume;
                                }
                            }
                            _strategy->GetQryDataSender().send(trec);
                        }
                        _page->onUpdateTradeRec(rec);
                        std::lock_guard<std::mutex> lk(_strategyMut);
                        if (_strategyRefs.find(rec.orderRef) != _strategyRefs.end())
                        {
                            OrderTradeRecord trec(rec);
                            trec.RequestID = _strategyRefs.at(rec.orderRef);
                            _strategy->GetTradeDataSender().send(trec);
                        }
                    });
        }
    }
    catch (const Done&)
    {
        if (_TdApi)
        {
            _TdApi->Release();
            _TdApi = nullptr;
        }
        if (_TdSpi)
        {
            delete _TdSpi;
            _TdSpi = nullptr;
        }
    }
}

void CTPTrader::MarketDataRun()
{ 
    try
    {
        while (true)
        {
            _marketDataReceiver.wait()
                .handle<Tick>([&](const Tick& tick)
                    {
                        _strategy->GetMarketDataSender().send(tick);
                        auto iter = _curTick.find(tick.instrumentID);
                        if (iter == _curTick.end())
                        {
                            _curTick[tick.instrumentID].upperLimit = tick.upperLimitPrice;
                            _curTick[tick.instrumentID].lowerLimit = tick.lowerLimitPrice;
                        }
                        _curTick[tick.instrumentID].lastPrice1.store(tick.lastPrice);
                        _curTick[tick.instrumentID].bidPrice1.store(tick.bidPrice1);
                        _curTick[tick.instrumentID].askPrice1.store(tick.askPrice1);
                        emit _page->tickArrived(tick);              
                    });
        }
    }
    catch (const Done&)
    {
        Sender(_receiver).send(Done());
        _strategy->GetMarketDataSender().send(Done());
        marketTime->Stop();
        _updateFundRunning = false;
        _timerOrderRunning = false;
    }
}

void CTPTrader::UpdateFundRun()
{
    while (!IsResumeFinished());
    while (_updateFundRunning)
    {
        QryTradingAccount();
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}

void CTPTrader::TimerOrderRun()
{
    while (_timerOrderRunning)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
        std::lock_guard<std::mutex> lk(_timerMut);
        if (_timerOrderBook.empty() && _timerWaitingBook.empty())
            continue;
        else
        {
            for (auto iter=_timerWaitingBook.begin();iter!=_timerWaitingBook.end();)
            {
                std::lock_guard<std::mutex> lk1(_orderMut);
                auto iter0= _orderSubmitRec.find(*iter);
                if (IsFinalTradeStatus(iter0->second.tradeStatus))
                {
                    int unfinishedvol = iter0->second.unfilledVolume;
                    if (!unfinishedvol)
                        continue;
                    double p = iter0->second.direction == TradeDirectionType::BUY ?
                        _curTick[iter0->second.instrumentID].upperLimit :
                        _curTick[iter0->second.instrumentID].lowerLimit;
                    auto tparam = OrderInsertParam(iter0->second.instrumentID,
                        p,
                        iter0->second.unfilledVolume,
                        iter0->second.direction,
                        iter0->second.combOffset);
                    tparam.UserID = iter0->second.UserID;
                    tparam.RequestID = iter0->second.RequestID;
                    GetSender().send(tparam);
                    iter = _timerWaitingBook.erase(iter);
                }
                else
                    ++iter;
            }
            auto curT = marketTime->GetMarketTime();
            for (auto iter = _timerOrderBook.begin(); iter != _timerOrderBook.end();/* ++iter*/)
            {
                std::lock_guard<std::mutex> lk1(_orderMut);
                auto order_iter = _orderSubmitRec.find(iter->second._orderRef);
                if (order_iter == _orderSubmitRec.end())
                {
                    iter = _timerOrderBook.erase(iter);
                    continue;
                }
                else
                {
                    if (IsFinalTradeStatus(order_iter->second.tradeStatus))
                    {
                        iter = _timerOrderBook.erase(iter);
                        continue;
                    }
                    if ((iter->second._param.deleteCondition == OrderDeleteCondition::TIME_OUT
                        || iter->second._param.deleteCondition == OrderDeleteCondition::TIME_OUT_AND_ADVERSE_PRICE_MOVEMENT)
                        && order_iter->second.submitTime.seconds_count()>0 
                        && curT - order_iter->second.submitTime >= iter->second._param.timeOutSeconds)
                    {
                        GetSender().send(OrderDeleteParam(iter->second._orderRef));
                        if(iter->second._param.action==ActionAfterDelete::MARKET_ORDER)
                            _timerWaitingBook.emplace(iter->second._orderRef);
                        iter = _timerOrderBook.erase(iter);
                        continue;
                    }
                    if (iter->second._param.deleteCondition == OrderDeleteCondition::ADVERSE_PRICE_MOVEMENT
                        || iter->second._param.deleteCondition == OrderDeleteCondition::TIME_OUT_AND_ADVERSE_PRICE_MOVEMENT)
                    {
                        double curP = 0.0;
                        if (iter->second._param.adversePriceType == PriceType::ASK)
                            curP = _curTick[order_iter->second.instrumentID].askPrice1.load();
                        else if(iter->second._param.adversePriceType == PriceType::BID)
                            curP = _curTick[order_iter->second.instrumentID].bidPrice1.load();
                        else
                            curP= _curTick[order_iter->second.instrumentID].lastPrice1.load();
                        //Log(QString::number(curP).append("+").append(QString::number(iter->second._param.priceMovement)));
                        if ((iter->second._param.baseParam.direction == TradeDirectionType::BUY
                            && curP - order_iter->second.orderPrice >= iter->second._param.priceMovement)
                            || (iter->second._param.baseParam.direction == TradeDirectionType::SELL
                                && order_iter->second.orderPrice - curP >= iter->second._param.priceMovement))
                        {
                            
                            GetSender().send(OrderDeleteParam(iter->second._orderRef));
                            if (iter->second._param.action == ActionAfterDelete::MARKET_ORDER)
                                _timerWaitingBook.emplace(iter->second._orderRef);
                            iter = _timerOrderBook.erase(iter);
                            continue;
                        }
                    }
                    ++iter;
                }
            }
        }
    }
}

std::string CTPTrader::GetBroker() const
{
	return _loginInfo.BrokerID;
}

