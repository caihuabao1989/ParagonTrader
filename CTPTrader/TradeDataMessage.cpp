#include "TradeDataMessage.h"

OrderSubmitRecord::OrderSubmitRecord(CThostFtdcOrderField* pOrder) :
    nRequestID(-1),
    orderRef(std::stoi(pOrder->OrderRef)),
    instrumentID(pOrder->InstrumentID),
    direction(static_cast<TradeDirectionType>(pOrder->Direction)),
    combOffset(static_cast<CombOffsetType>(pOrder->CombOffsetFlag[0])),
    orderPrice(pOrder->LimitPrice),
    orderVolume(pOrder->VolumeTotalOriginal),
    exchangeID(pOrder->ExchangeID),
    frontID(pOrder->FrontID),
    sessionID(pOrder->SessionID),
    submitStatus(static_cast<OrderSubmitStatus>(pOrder->OrderSubmitStatus)),
    tradeStatus(static_cast<OrderTradeStatus>(pOrder->OrderStatus)),
    unfilledVolume(pOrder->VolumeTotal),
    filledVolume(pOrder->VolumeTraded),
    submitTime(chbTime(pOrder->InsertTime, ':'))
{
    strcpy(orderSysID, pOrder->OrderSysID);
}

OrderSubmitRecord::OrderSubmitRecord(const OrderSubmitRecord& other) :
    nRequestID(other.nRequestID),
    frontID(other.frontID),
    sessionID(other.sessionID),
    orderRef(other.orderRef),
    instrumentID(other.instrumentID),
    direction(other.direction),
    combOffset(other.combOffset),
    submitStatus(other.submitStatus),
    tradeStatus(other.tradeStatus),
    orderPrice(other.orderPrice),
    orderVolume(other.orderVolume),
    unfilledVolume(other.unfilledVolume),
    filledVolume(other.filledVolume),
    submitTime(other.submitTime),
    exchangeID(other.exchangeID)
{
    strcpy(orderSysID, other.orderSysID);
}

OrderSubmitRecord& OrderSubmitRecord::operator=(const OrderSubmitRecord& other)
{
    nRequestID = other.nRequestID;
    frontID = other.frontID;
    sessionID = other.sessionID;
    orderRef = other.orderRef;
    instrumentID = other.instrumentID;
    direction = other.direction;
    combOffset = other.combOffset;
    submitStatus = other.submitStatus;
    tradeStatus = other.tradeStatus;
    orderPrice = other.orderPrice;
    orderVolume = other.orderVolume;
    unfilledVolume = other.unfilledVolume;
    filledVolume = other.filledVolume;
    submitTime = other.submitTime;
    exchangeID = other.exchangeID;
    strcpy(orderSysID, other.orderSysID);
    return *this;
}

void OrderSubmitRecord::FirstCreate(CThostFtdcInputOrderField* ptr, int requestID)
{
    nRequestID = requestID;
    orderRef = std::stoi(ptr->OrderRef);
    instrumentID = ptr->InstrumentID;
    direction = static_cast<TradeDirectionType>(ptr->Direction);
    combOffset = static_cast<CombOffsetType>(ptr->CombOffsetFlag[0]);
    orderPrice = ptr->LimitPrice;
    orderVolume = ptr->VolumeTotalOriginal;
    exchangeID = ptr->ExchangeID;
}

void OrderSubmitRecord::FirstCreate(CThostFtdcOrderField* pOrder)
{
    nRequestID = -1;
    orderRef = std::stoi(pOrder->OrderRef);
    instrumentID = pOrder->InstrumentID;
    direction = static_cast<TradeDirectionType>(pOrder->Direction);
    combOffset = static_cast<CombOffsetType>(pOrder->CombOffsetFlag[0]);
    orderPrice = pOrder->LimitPrice;
    orderVolume = pOrder->VolumeTotalOriginal;
    exchangeID = pOrder->ExchangeID;
    Update(pOrder);
}

void OrderSubmitRecord::Update(CThostFtdcOrderField* ptr)
{
    frontID = ptr->FrontID;
    sessionID = ptr->SessionID;
    strcpy(orderSysID, ptr->OrderSysID);
    submitStatus = static_cast<OrderSubmitStatus>(ptr->OrderSubmitStatus);
    tradeStatus = static_cast<OrderTradeStatus>(ptr->OrderStatus);
    unfilledVolume = ptr->VolumeTotal;
    filledVolume = ptr->VolumeTraded;
    submitTime = chbTime(ptr->InsertTime, ':');
}

void OrderSubmitRecord::Update(const OrderSubmitRecord& rec)
{
    frontID = rec.frontID;
    sessionID = rec.sessionID;
    strcpy(orderSysID, rec.orderSysID);
    submitStatus = rec.submitStatus;
    tradeStatus = rec.tradeStatus;
    unfilledVolume = rec.unfilledVolume;
    filledVolume = rec.filledVolume;
    submitTime = rec.submitTime;
}



OrderTradeRecord::OrderTradeRecord(const OrderTradeRecord& other):
    orderRef(other.orderRef),
    instrumentID(other.instrumentID),
    direction(other.direction),
    combOffset(other.combOffset),
    tradedPrice(other.tradedPrice),
    tradedVolume(other.tradedVolume),
    tradedTime(other.tradedTime),
    exchangeID(other.exchangeID)
{
    strcpy(tradeID, other.tradeID);
    strcpy(orderSysID, other.orderSysID);
}

OrderTradeRecord& OrderTradeRecord::operator=(const OrderTradeRecord& other)
{
    orderRef = other.orderRef;
    instrumentID = other.instrumentID;
    direction = other.direction;
    combOffset = other.combOffset;
    tradedPrice = other.tradedPrice;
    tradedVolume = other.tradedVolume;
    tradedTime = other.tradedTime;
    exchangeID = other.exchangeID;
    strcpy(tradeID, other.tradeID);
    strcpy(orderSysID, other.orderSysID);
    return *this;
}

OrderTradeRecord::OrderTradeRecord(CThostFtdcTradeField* ptr) :
    orderRef(QString(ptr->OrderRef).toInt()),
    instrumentID(ptr->InstrumentID),
    direction(static_cast<TradeDirectionType>(ptr->Direction)),
    combOffset(static_cast<CombOffsetType>(ptr->OffsetFlag)),
    tradedPrice(ptr->Price),
    tradedVolume(ptr->Volume),
    tradedTime(chbTime(ptr->TradeTime, ':')),
    exchangeID(ptr->ExchangeID)
{
    memset(tradeID, 0, sizeof(tradeID));
    memset(orderSysID, 0, sizeof(orderSysID));
    strcpy(tradeID, ptr->TradeID);
    strcpy(orderSysID, ptr->OrderSysID);
}

bool IsFinalTradeStatus(OrderTradeStatus status)
{
    switch (status)
    {
    case OrderTradeStatus::AllTraded:
    case OrderTradeStatus::PartTradedNotQueueing:
    case OrderTradeStatus::NoTradeNotQueueing:
    case OrderTradeStatus::Canceled:
        return true;
    case OrderTradeStatus::PartTradedQueueing:
    case OrderTradeStatus::NoTradeQueueing:
    case OrderTradeStatus::Unknown:
    case OrderTradeStatus::NotTouched:
    case OrderTradeStatus::Touched:
    default:
        return false;
    }
}

QString TranslateTradeDirectionType(TradeDirectionType type)
{
    return type == TradeDirectionType::BUY ? QStringLiteral("��") : QStringLiteral("��");
}
QString TranslateComboOffsetType(CombOffsetType type)
{
    return type == CombOffsetType::OPEN ? QStringLiteral("��") : QStringLiteral("ƽ");
}
QString TranslateOrderSubmitStatus(OrderSubmitStatus status)
{
    switch (status)
    {
    case OrderSubmitStatus::InsertSubmitted:
        return QStringLiteral("�ѱ�");
    case OrderSubmitStatus::CancelSubmitted:
        return QStringLiteral("�����ѱ�");
    case OrderSubmitStatus::ModifySubmitted:
        return QStringLiteral("�޸��ѱ�");
    case OrderSubmitStatus::Accepted:
        return QStringLiteral("�ѽ���");
    case OrderSubmitStatus::InsertRejected:
        return QStringLiteral("��������");
    case OrderSubmitStatus::CancelRejected:
        return QStringLiteral("��������");
    case OrderSubmitStatus::ModifyRejected:
        return QStringLiteral("�޸ı���");
    default:
        return QStringLiteral("δ֪");
    }
}
QString TranslateOrderTradeStatus(OrderTradeStatus status)
{
    switch (status)
    {
    case OrderTradeStatus::AllTraded:
        return QStringLiteral("ȫ���ɽ�");
    case OrderTradeStatus::PartTradedQueueing:
        return QStringLiteral("���ֳɽ�");
    case OrderTradeStatus::PartTradedNotQueueing:
        return QStringLiteral("���ɲ���");
    case OrderTradeStatus::NoTradeQueueing:
        return QStringLiteral("δ�ɽ�");
    case OrderTradeStatus::NoTradeNotQueueing:
        return QStringLiteral("������CTP�ܾ�");
    case OrderTradeStatus::Canceled:
        return QStringLiteral("����");
    case OrderTradeStatus::NotTouched:
        return QStringLiteral("Ԥ��δ����");
    case OrderTradeStatus::Touched:
        return QStringLiteral("Ԥ���Ѵ���");
    case OrderTradeStatus::Unknown:
    default:
        return QStringLiteral("δ֪");
    }
}
