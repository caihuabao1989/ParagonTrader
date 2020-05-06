#pragma once

#include <string>
#include "TradeDataMessage.h"
#include "date.h"
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <qdebug.h>

class CTPTrader;

struct AccountPosition
{
    std::string InstrumentID;
    int PreviousLong = 0, PreviousShort = 0, FrozenPreviousLong = 0, FrozenPreviousShort = 0;
    int TodayLong = 0, TodayShort = 0, FrozenTodayLong = 0, FrozenTodayShort = 0;
    int TotalLong = 0, TotalShort = 0, FrozenTotalLong = 0, FrozenTotalShort = 0;
};

struct AccountPositionDetail
{
    std::string InstrumentID;
    TradeDirectionType Direction;
    chbDate OpenDate;
    char TradeID[21];
    int Volume;
    double OpenPrice;
    double Margin;
    std::string ExchangeID;
    int CloseVolume;
    double CloseAmount;

    AccountPositionDetail(CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail);
};

class Account
{
public:
    friend class CTPTrader;
    Account(CTPTrader*);
    void ClearDetailPosition() { _detailPos.clear(); }
    void InsertDetailPosition(CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail);
    void SetRequestID(int id) { _requestID = id; }
    int GetCurrentRequestID() const { return _requestID; }
    AccountPosition GetPosition(const std::string& instrument) const;
    AccountPosition GetCalculatedPosition(const std::string& instrument) const;
    double GetAvailable() const;
    double GetCalculatedAvailable() const;
    double GetCurrentMargin() const;
    double GetCalculatedCurrentMargin() const;
    void SetAvailable(double);
    void SetCalculatedAvailable(double);
    void SetCurrentMargin(double);
    void SetCalculatedCurrentMargin(double);
    void UpdatePositionFromDetail();
    void UpdatePosition(CThostFtdcInvestorPositionField* pInvestorPosition);
    void SetFrozen(const std::string& instrument, int change,TradeDirectionType direction,bool isYd=false);
    const std::unordered_map<std::string, AccountPosition>& GetCalPositionMap() const { return _calculatedPos; }
private:
    CTPTrader* _trader;
    mutable std::mutex _posMut, _cposMut;
    std::atomic<double> _available, _cavailable, _currmargin, _ccurrmargin;
    std::unordered_map<std::string, AccountPosition> _pos, _calculatedPos;
    std::unordered_multimap<std::string, AccountPositionDetail> _detailPos;
    std::atomic_int _requestID;
};