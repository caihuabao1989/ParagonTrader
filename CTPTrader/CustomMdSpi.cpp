#include "CustomMdSpi.h"
#include "MarketDataMessage.h"
#include "MarketData.h"
#include "LogSystem.h"
#include <qdebug.h>

extern std::shared_ptr<MarketData> marketData;
extern LogSystem* globalLogger;

CustomMdSpi::CustomMdSpi()
{
   // _dataSender = marketData->_dataReceiver;
}

void CustomMdSpi::OnFrontConnected()
{
    marketData->_isFrontConnected = true;
    marketData->ReqUserLogin();
}

void CustomMdSpi::OnFrontDisconnected(int nReason)
{
    marketData->_isFrontConnected = false;
}

void CustomMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField* pRspUserLogin, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isError(pRspInfo))
    {
        marketData->_isLogin = true;
    }
}

void CustomMdSpi::OnRspUserLogout(CThostFtdcUserLogoutField* pUserLogout, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isError(pRspInfo))
    {
        marketData->_isLogin = false;
    }
}

void CustomMdSpi::OnRspError(CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    isError(pRspInfo);
}

void CustomMdSpi::OnRspSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isError(pRspInfo) && pSpecificInstrument)
    {
        marketData->AddCurrentSubscription(std::string(pSpecificInstrument->InstrumentID));
    }
}

void CustomMdSpi::OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField* pSpecificInstrument, CThostFtdcRspInfoField* pRspInfo, int nRequestID, bool bIsLast)
{
    if (!isError(pRspInfo) && pSpecificInstrument)
    {
        marketData->RemoveCurrentSubscription(std::string(pSpecificInstrument->InstrumentID));
    }
}

void CustomMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField* pDepthMarketData)
{
    Sender(marketData->_dataReceiver).send(Tick(pDepthMarketData));
}

bool CustomMdSpi::isError(CThostFtdcRspInfoField* pRspInfo)
{
    bool bResult = pRspInfo && (pRspInfo->ErrorID != 0);
    if (bResult)
    {
        globalLogger->Log("Error ID=" + QString(pRspInfo->ErrorID) + ",Error Message=" + QString(pRspInfo->ErrorMsg));
    }
    return bResult;
}

