#pragma once
#include "ThostFtdcUserApiStruct.h"
#include <string>
#include <qstring.h>
#include <atomic>
#include "date.h"
#include "Message.h"

struct Tick
{
    Tick() = default;
    Tick(CThostFtdcDepthMarketDataField* pDepthMarketData);

    std::string instrumentID;
    chbTime updateTime;
    double lastPrice, preClose;
    double bidPrice1, bidPrice2, bidPrice3, bidPrice4, bidPrice5;
    double askPrice1, askPrice2, askPrice3, askPrice4, askPrice5;
    int bidVolume1, bidVolume2, bidVolume3, bidVolume4, bidVolume5;
    int askVolume1, askVolume2, askVolume3, askVolume4, askVolume5;
    int accumulatedVolume;
    int openInterest;
    double turnOver;
    double open, high, low;
    double upperLimitPrice, lowerLimitPrice;
};

struct Bar
{
    std::string instrumentID;
    chbTime startTime, endTime;
    double open, high, low, close;
    int volume;
    double amount;
};

struct AtomicTick
{
    std::atomic<double> askPrice1=0.;
    std::atomic<double> bidPrice1=0.;
    std::atomic<double> lastPrice1 = 0;
    double upperLimit = 0;
    double lowerLimit = 0;
};

template<class T>
struct Subscription
{
    T contents;
    Subscription(const T& sub):contents(sub){}
};

template<class T>
struct UnSubscription
{
    T contents;
    UnSubscription(const T& unsub) :contents(unsub) {}
};

struct BrokerChoice
{
    QString broker;
    BrokerChoice(const QString& str):broker(str){}
};

struct SenderRegistration
{
    Sender sender;
    SenderRegistration(const Sender& sender):sender(sender){}
};