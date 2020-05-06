#pragma once
#include <string>
#include "ThostFtdcUserApiStruct.h"
#include <qstringlist.h>

struct InstrumentInfo
{
    std::string InstrumentId;
    std::string ExchangeId;
    std::string UnderlyingInstrumentId;
    int DeliveryYear, DeliveryMonth;
    int MaxMarketOrderVol, MinMarketOrderVol, MaxLimitOrderVol, MinLimitOrderVol;
    int VolumeMultiple;
    double PriceTick;
    int OpenDate, ExpireDate;
    bool IsTrading;
    double LongMarginRatio, ShortMarginRatio;
    double StrikePrice;
    char OptionType;
    double UnderlyingMultiple;

    InstrumentInfo() = default;
    InstrumentInfo::InstrumentInfo(CThostFtdcInstrumentField* pInstrument) :
        InstrumentId(pInstrument->InstrumentID),
        ExchangeId(pInstrument->ExchangeID),
        UnderlyingInstrumentId(pInstrument->UnderlyingInstrID),
        DeliveryYear(pInstrument->DeliveryYear),
        DeliveryMonth(pInstrument->DeliveryMonth),
        MaxMarketOrderVol(pInstrument->MaxMarketOrderVolume),
        MinMarketOrderVol(pInstrument->MinMarketOrderVolume),
        MaxLimitOrderVol(pInstrument->MaxLimitOrderVolume),
        MinLimitOrderVol(pInstrument->MinLimitOrderVolume),
        VolumeMultiple(pInstrument->VolumeMultiple),
        PriceTick(pInstrument->PriceTick),
        OpenDate(std::stoi(pInstrument->OpenDate)),
        ExpireDate(std::stoi(pInstrument->ExpireDate)),
        IsTrading(pInstrument->IsTrading),
        LongMarginRatio(pInstrument->LongMarginRatio),
        ShortMarginRatio(pInstrument->ShortMarginRatio),
        StrikePrice(pInstrument->StrikePrice),
        OptionType(pInstrument->OptionsType),
        UnderlyingMultiple(pInstrument->UnderlyingMultiple) {}
};

QStringList GetAllInstrument();