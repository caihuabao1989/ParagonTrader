#pragma once
#include <QString>
#include "ThostFtdcUserApiStruct.h"
#include "date.h"
//#include "StrategyBase.h"

enum class TradeDirectionType { BUY = '0', SELL = '1' };
enum class CombOffsetType 
{ 
    OPEN = '0', 
    CLOSE = '1' ,
    FORCE_CLOSE='2',
    ClOSE_TODAY='3',
    CLOSE_YESTERDAY='4',
    FORCE_OFF='5',
    LOCAL_FORCE_CLOSE='6'
};
enum class OrderTradeStatus
{
    AllTraded = '0', //final
    PartTradedQueueing = '1',
    PartTradedNotQueueing = '2',//final
    NoTradeQueueing = '3',
    NoTradeNotQueueing = '4',//final
    Canceled = '5',//final
    Unknown = 'a',
    NotTouched = 'b',
    Touched = 'c'
};
bool IsFinalTradeStatus(OrderTradeStatus status);

enum class OrderSubmitStatus
{
    InsertSubmitted = '0',
    CancelSubmitted = '1',
    ModifySubmitted = '2',
    Accepted = '3',
    InsertRejected = '4',
    CancelRejected = '5',
    ModifyRejected = '6'
};

struct OrderSubmitRecord
{
    int nRequestID;
    int frontID;
    int sessionID;
    int orderRef;
    char orderSysID[21];
    std::string instrumentID;
    TradeDirectionType direction;
    CombOffsetType combOffset;
    OrderSubmitStatus submitStatus;
    OrderTradeStatus tradeStatus;
    double orderPrice;
    int    orderVolume;
    int    unfilledVolume;
    int    filledVolume;
    chbTime  submitTime;
    std::string exchangeID;
    int UserID = -1;
    int RequestID = -1;

    OrderSubmitRecord() = default;
    OrderSubmitRecord(CThostFtdcOrderField* pOrder);
    OrderSubmitRecord(const OrderSubmitRecord& other);
    OrderSubmitRecord& operator=(const OrderSubmitRecord& other);
    void FirstCreate(CThostFtdcInputOrderField*, int);
    void FirstCreate(CThostFtdcOrderField* pOrder);
    void Update(CThostFtdcOrderField*);
    void Update(const OrderSubmitRecord& rec);
};

struct OrderTradeRecord
{
    char tradeID[21];
    char orderSysID[21];
    int orderRef;
    std::string instrumentID;
    TradeDirectionType direction;
    CombOffsetType combOffset;
    double tradedPrice;
    int tradedVolume;
    chbTime tradedTime;
    std::string exchangeID;
    int UserID = -1;
    int RequestID = -1;
    OrderTradeRecord() = default;
    OrderTradeRecord(const OrderTradeRecord& other);
    OrderTradeRecord& operator=(const OrderTradeRecord& other);
    OrderTradeRecord(CThostFtdcTradeField*);
};

struct OrderInsertParam
{
    std::string instrumentID;
    double price;
    int volume;
    TradeDirectionType direction;
    CombOffsetType offset;
    int UserID = -1;
    int RequestID = -1;
    OrderInsertParam() = default;
    OrderInsertParam(const std::string& instrument,
        double price,
        int volume,
        TradeDirectionType direction,
        CombOffsetType offset):
        instrumentID(instrument),
        price(price),
        volume(volume),
        direction(direction),
        offset(offset){}
};

struct MarketOrderInsertParam
{
    std::string instrumentID;
    int volume;
    TradeDirectionType direction;
    CombOffsetType offset;
    int UserID = -1;
    int RequestID = -1;
    MarketOrderInsertParam(const std::string& instrument,
        int volume,
        TradeDirectionType direction,
        CombOffsetType offset):
        instrumentID(instrument),
        volume(volume),
        direction(direction),
        offset(offset) {}
};

struct OrderDeleteParam
{
    int orderRef;
    int UserID = -1;
    int RequestID = -1;
    OrderDeleteParam(int ref):orderRef(ref){}
};

struct OrderDeleteParamBySysID
{
    int orderSysID;
    std::string exchangeID;
    int UserID = -1;
    int RequestID = -1;
    OrderDeleteParamBySysID(int sysid,const std::string& exchange):
        orderSysID(sysid),exchangeID(exchange){}
};


struct TraderLoginInfo
{
    QString brokerID;
    QString accountID;
    QString passWord;
    TraderLoginInfo(const QString& broker, const QString& account, const QString& password):
        brokerID(broker),accountID(account),passWord(password){}
};

struct TraderFrontConnected{};

struct LoginSuccess{};

struct AuthenticateSuccess {};

class StrategyBase;

struct StrategyChoice
{
    std::shared_ptr<StrategyBase> strategy;
    StrategyChoice(const std::shared_ptr<StrategyBase>& strategy):
        strategy(strategy){}
};

struct StrategyInsertSuccess
{
    int id = -1;
    StrategyInsertSuccess(int id):id(id){}
};

struct StrategyChangeStatus
{
    int id = -1;
    StrategyChangeStatus(int id):id(id){}
};

struct StrategyChangeStatusSuccess
{
    int id = -1;
    bool flag = false;
    StrategyChangeStatusSuccess(int id,bool flag):
        id(id),flag(flag){}
};

QString TranslateTradeDirectionType(TradeDirectionType type);
QString TranslateComboOffsetType(CombOffsetType type);
QString TranslateOrderSubmitStatus(OrderSubmitStatus status);
QString TranslateOrderTradeStatus(OrderTradeStatus status);