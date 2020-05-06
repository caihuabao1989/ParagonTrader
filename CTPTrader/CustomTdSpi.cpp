#include "CustomTdSpi.h"
#include "CTPTrader.h"
#include "InstrumentInfo.h"
#include "OperationWindow.h"
#include "MarketTime.h"
#include <qdebug.h>

extern std::unordered_map<std::string, InstrumentInfo> instrumentInfo;
extern OperationWindow* mainWindow;
extern std::shared_ptr<MarketTime> marketTime;

CustomTdSpi::CustomTdSpi(CTPTrader* trader):
	_trader(trader),
    _dataSender(trader->_dataReceiver)
{
}

void CustomTdSpi::OnFrontConnected()
{
	_trader->_isFrontConnected = true;
    _trader->GetSender().send(TraderFrontConnected());
}

void CustomTdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isError(pRspInfo))
    {
        marketTime->SetStartLocalTime();
        marketTime->CalculateLags(pRspUserLogin);
        _trader->_isLogin = true;
        _trader->_frontID = pRspUserLogin->FrontID;
        _trader->_sessionID = pRspUserLogin->SessionID;
        _trader->_orderRef = std::stoi(pRspUserLogin->MaxOrderRef) + 1;
        _trader->GetSender().send(LoginSuccess());
    }
    else if (_trader->_loginDialog)
    {
        _trader->_loginDialog->EnableBtnOK(true);
    }
}

void CustomTdSpi::OnRspAuthenticate(CThostFtdcRspAuthenticateField* pRspAuthenticateField, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isError(pRspInfo))
    {
        _trader->GetSender().send(AuthenticateSuccess());
    }
}

void CustomTdSpi::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    isError(pRspInfo);
}

void CustomTdSpi::OnFrontDisconnected(int nReason)
{
	_trader->_isFrontConnected = false;
}

void CustomTdSpi::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isError(pRspInfo))
    {
        _trader->_isLogin = false;
    }
}

void CustomTdSpi::OnRspSettlementInfoConfirm(CThostFtdcSettlementInfoConfirmField* pSettlementInfoConfirm, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isError(pRspInfo))
    {
        _trader->QryInstrumentInfo("");
    }
}

void CustomTdSpi::OnRspQryInstrument(CThostFtdcInstrumentField* pInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isError(pRspInfo))
    {
        instrumentInfo[pInstrument->InstrumentID] = InstrumentInfo(pInstrument);
        if (bIsLast && !nRequestID)
        {
            HandTradePage::UpdateCompleter(GetAllInstrument());
            _trader->_page->SetCompleter();
            _trader->QryPosition("");
        }
    }
}

void CustomTdSpi::OnRspQryTradingAccount
(CThostFtdcTradingAccountField* pTradingAccount, 
 CThostFtdcRspInfoField* pRspInfo, 
 int nRequestID, 
 bool bIsLast)
{
    if (!isError(pRspInfo))
    {
        _trader->_account.SetAvailable(pTradingAccount->Available);
        _trader->_account.SetCurrentMargin(pTradingAccount->CurrMargin);
        emit _trader->_page->fundUpdateArrived(pTradingAccount->Available, pTradingAccount->CurrMargin);
        _trader->_strategy->GetQryDataSender().send(std::make_pair(pTradingAccount->Available, pTradingAccount->CurrMargin));
    }
    if (bIsLast && !nRequestID)
    {
        mainWindow->OpenOperationWindow();
        _trader->_loginDialog->CloseDialog();
        _trader->_page->SetThisAsAccountLogger();       
        marketTime->StartUpdateTime();
    }
}

void CustomTdSpi::OnRspQryInvestorPosition
(CThostFtdcInvestorPositionField* pInvestorPosition, 
    CThostFtdcRspInfoField* pRspInfo, 
    int nRequestID,
    bool bIsLast)
{
    if (!isError(pRspInfo) && pInvestorPosition)
    {
        _trader->_account.UpdatePosition(pInvestorPosition);
        if (bIsLast && !nRequestID)
        {
            auto posmap = _trader->_account.GetCalPositionMap();
            _trader->_page->onPositionInit(posmap);
            for (const auto& line : posmap)
            {
                _trader->_strategy->GetQryDataSender().send(line.second);
            }
            _trader->QryTradingAccount();
        }
    }
}

void CustomTdSpi::OnRspOrderAction(
    CThostFtdcInputOrderActionField* pInputOrderAction, 
    CThostFtdcRspInfoField* pRspInfo, 
    int nRequestID, 
    bool bIsLast)
{
    if (isError(pRspInfo))
    {
        _trader->Log(QStringLiteral("[³·µ¥´íÎó]")
            .append("OrderSysID->").append(pInputOrderAction->OrderSysID)
            .append("OrderRef->").append(pInputOrderAction->OrderRef));
    };
}

void CustomTdSpi::OnRtnOrder(CThostFtdcOrderField* pOrder)
{
    _dataSender.send(OrderSubmitRecord(pOrder));
}

void CustomTdSpi::OnRtnTrade(CThostFtdcTradeField* pTrade)
{
    _dataSender.send(OrderTradeRecord(pTrade));
}

void CustomTdSpi::OnRspQryInvestorPositionDetail
(CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail, 
 CThostFtdcRspInfoField* pRspInfo, 
 int nRequestID, 
 bool bIsLast)
{
    if (!isError(pRspInfo))
    {
        if (nRequestID != _trader->_account.GetCurrentRequestID())
        {
            _trader->_account.ClearDetailPosition();
            _trader->_account.SetRequestID(nRequestID);
        }
        if(pInvestorPositionDetail)
            _trader->_account.InsertDetailPosition(pInvestorPositionDetail);
        if (bIsLast)
        {
            _trader->_account.UpdatePositionFromDetail();
            auto posmap = _trader->_account.GetCalPositionMap();
            _trader->_page->onPositionInit(posmap);
            for (const auto& line : posmap)
            {
                _trader->_strategy->GetQryDataSender().send(line.second);
            }
            
        }
        if (bIsLast && !nRequestID)
        {
            _trader->QryTradingAccount();
        }
    }
}

void CustomTdSpi::OnErrRtnOrderInsert(CThostFtdcInputOrderField* pInputOrder, CThostFtdcRspInfoField* pRspInfo)
{
    isError(pRspInfo);
}

bool CustomTdSpi::isError(CThostFtdcRspInfoField* pRspInfo)
{
    bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
    if (bResult)
    {
        _trader->Log("Error ID=" + QString::number(pRspInfo->ErrorID) + ",Error Message=" + QString::fromLocal8Bit(pRspInfo->ErrorMsg));
    }
    return bResult;
}
