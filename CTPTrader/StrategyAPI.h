#pragma once
#include "Message.h"
#include "TimerOrder.h"
#include "Account.h"


#ifdef _STRATEGY_
#define STRATEGY_API __declspec(dllexport)
#else
#define STRATEGY_API __declspec(dllimport)
#endif

class StrategyBase;

class STRATEGY_API StrategyAPI
{
public:
	static std::shared_ptr<StrategyAPI> CreateStrategyApi();

	//��������ú��������Բ�Ӧʹ��
	virtual Sender GetMarketDataSender() = 0;
	virtual Sender GetTradeDataSender() = 0;
	virtual Sender GetQryDataSender() = 0;
	virtual Sender GetTimerSender() = 0;
	virtual void SetReceiver(const Sender& sender) = 0;
	virtual void Init() = 0;
	virtual void Join() = 0;
	virtual void Detach() = 0;
	virtual void Release() = 0;
	virtual bool InsertStrategy(const std::shared_ptr<StrategyBase>& ptr) = 0;
	virtual bool SetRunning(int id, bool flag) = 0;
	virtual bool IsRunning(int id) = 0;

	//�����־��Ϣ
	virtual void Log(const std::string& str) = 0;
	
	/*
	�޼��µ�����
	@instrument ��Լ����
	@price      �۸�
	@volume     ����
	@id         ����ID������ID�ɲ�������ά����Ϊϵͳʶ����Եı�ʶ��������ȷ����Է��ض�Ӧ�ĳɽ��ر������ݣ�
	@return     ����ID������IDΪÿ�ʱ������˻�����Ϊ���Ա������ɵı�ʶ����ϵͳ���ɵ�orderRef�γɶ�Ӧ���Ա�ʶ���Ӧ�ı����ɽ��ر���
	*/
	virtual int BuyOpen(const std::string& instrument, double price, int volume, int id) = 0;
	virtual int BuyClose(const std::string& instrument, double price, int volume, int id) = 0;
	virtual int SellOpen(const std::string& instrument, double price, int volume, int id) = 0;
	virtual int SellClose(const std::string& instrument, double price, int volume, int id) = 0;

	/*
	��ʱ���µ�������������δ�ɽ�ǰ���£�����һ�������ᱻ����������������ѡ��
	����1��ָ��ʱ���δ�ɽ�;
	����2��ָ���۸�ƫ�뱨���۴ﵽָ��ֵ��
	����������һ��ȫѡ
	�����󣺲�����/�м۲�����
	*/
	virtual int TimerBuyOpen(const std::string& instrument, double price, int volume, int id,
		int seconds,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;
	virtual int TimerBuyOpen(const std::string& instrument, double price, int volume, int id,
		double priceSpread,
		PriceType priceType = PriceType::LAST,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;
	virtual int TimerBuyOpen(const std::string& instrument, double price, int volume, int id,
		int seconds,
		double priceSpread,
		PriceType priceType = PriceType::LAST,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;

	virtual int TimerBuyClose(const std::string& instrument, double price, int volume, int id,
		int seconds,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;
	virtual int TimerBuyClose(const std::string& instrument, double price, int volume, int id,
		double priceSpread,
		PriceType priceType = PriceType::LAST,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;
	virtual int TimerBuyClose(const std::string& instrument, double price, int volume, int id,
		int seconds,
		double priceSpread,
		PriceType priceType = PriceType::LAST,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;

	virtual int TimerSellOpen(const std::string& instrument, double price, int volume, int id,
		int seconds,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;
	virtual int TimerSellOpen(const std::string& instrument, double price, int volume, int id,
		double priceSpread,
		PriceType priceType = PriceType::LAST,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;
	virtual int TimerSellOpen(const std::string& instrument, double price, int volume, int id,
		int seconds,
		double priceSpread,
		PriceType priceType = PriceType::LAST,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;

	virtual int TimerSellClose(const std::string& instrument, double price, int volume, int id,
		int seconds,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;
	virtual int TimerSellClose(const std::string& instrument, double price, int volume, int id,
		double priceSpread,
		PriceType priceType = PriceType::LAST,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;
	virtual int TimerSellClose(const std::string& instrument, double price, int volume, int id,
		int seconds,
		double priceSpread,
		PriceType priceType = PriceType::LAST,
		ActionAfterDelete action = ActionAfterDelete::MARKET_ORDER) = 0;

	/*
	��������
	1��orderRef�ڱ����ر��л�ã��뱨��������ID��Ӧ(��orderRefΪ����������,ֻҪ��������������)
	2��orderSysIDΪ���������ɣ��������ܵ�����£����ֶ�Ϊ��,Ҳ�޷���������
	*/
	virtual void DeleteOrder(int orderRef) = 0;
	virtual void DeleteOrder(int orderSysID, const std::string& exchange) = 0;

	/*
	��ѯ��ӿ�
	��λ��ѯ���ص��������˻��Ĳ�λ����Ϊ����������ʵʱ����ά�ֵĲ�λ�������Ƽ��������������Բ�λ
	�ʽ��ѯ���ص��ǹ�̨���ص��ʽ�ÿ�����һ�Σ�����ʵʱ
	*/
	virtual int LongPosition(const std::string& instrument) const = 0;
	virtual int ShortPosition(const std::string& instrument) const = 0;
	virtual AccountPosition GetPosition(const std::string& instrument) const = 0;
	virtual double AvailableCash() const = 0;
	virtual double CurrentMarginUsed() const = 0;
};