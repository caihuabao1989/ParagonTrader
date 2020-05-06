#include "Account.h"
#include "InstrumentInfo.h"
#include "CTPTrader.h"

extern std::unordered_map<std::string, InstrumentInfo> instrumentInfo;

AccountPositionDetail::AccountPositionDetail(CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail):
	InstrumentID(pInvestorPositionDetail->InstrumentID),
	Direction(static_cast<TradeDirectionType>(pInvestorPositionDetail->Direction)),
	OpenDate(std::stoi(pInvestorPositionDetail->OpenDate)),
	Volume(pInvestorPositionDetail->Volume),
	OpenPrice(pInvestorPositionDetail->OpenPrice),
	Margin(pInvestorPositionDetail->Margin),
	ExchangeID(pInvestorPositionDetail->ExchangeID),
	CloseVolume(pInvestorPositionDetail->CloseVolume),
	CloseAmount(pInvestorPositionDetail->CloseAmount)
{
	strcpy(TradeID, pInvestorPositionDetail->TradeID);
}

Account::Account(CTPTrader* trader):
	_trader(trader),
	_requestID(-1)
{

}

void Account::InsertDetailPosition(CThostFtdcInvestorPositionDetailField* pInvestorPositionDetail)
{
	_detailPos.emplace(pInvestorPositionDetail->InstrumentID, AccountPositionDetail(pInvestorPositionDetail));
}

AccountPosition Account::GetPosition(const std::string& instrument) const
{
	if (instrumentInfo.find(instrument) == instrumentInfo.end())
	{
		_trader->Log(QStringLiteral("查询实际仓位失败，合约ID不合法。"));
	}
	std::lock_guard<std::mutex> lk(_posMut);
	auto iter = _pos.find(instrument);
	if (iter == _pos.end())
	{
		return AccountPosition();
	}
	else
	{
		return iter->second;
	}
}

AccountPosition Account::GetCalculatedPosition(const std::string& instrument) const
{
	if (instrumentInfo.find(instrument) == instrumentInfo.end())
	{
		_trader->Log(QStringLiteral("查询计算仓位失败，合约ID不合法。"));
	}
	std::lock_guard<std::mutex> lk(_cposMut);
	auto iter = _calculatedPos.find(instrument);
	if (iter == _calculatedPos.end())
	{
		return AccountPosition();
	}
	else
	{
		return iter->second;
	}
}

double Account::GetAvailable() const
{
	return _available.load();
}

double Account::GetCalculatedAvailable() const
{
	return _cavailable.load();
}

double Account::GetCurrentMargin() const
{
	return _currmargin.load();
}

double Account::GetCalculatedCurrentMargin() const
{
	return _ccurrmargin.load();
}

void Account::SetAvailable(double val)
{
	_available.store(val);
}

void Account::SetCalculatedAvailable(double val)
{
	_cavailable.store(val);
}

void Account::SetCurrentMargin(double val)
{
	_currmargin.store(val);
}

void Account::SetCalculatedCurrentMargin(double val)
{
	_ccurrmargin.store(val);
}


void Account::UpdatePositionFromDetail()
{
	std::lock_guard<std::mutex> lk1(_posMut);
	std::unique_lock<std::mutex> lk2(_cposMut);
	auto curDate = chbDateTime::get_system_time().date();
	if (!_requestID)
	{
		_pos.clear(); _calculatedPos.clear();
		for (const auto& pos : _detailPos)
		{
			//qDebug() << pos.second.InstrumentID.c_str() << pos.second.Volume << pos.second.OpenDate.date_num() << pos.second.TradeID;
			int vol = pos.second.Volume;// -pos.second.CloseVolume;
			if (!vol)
				continue;
			auto& instr = _pos[pos.first];
			auto& cinstr = _calculatedPos[pos.first];
			instr.InstrumentID = instr.InstrumentID.empty() ? pos.first : instr.InstrumentID;
			cinstr.InstrumentID = cinstr.InstrumentID.empty() ? pos.first : cinstr.InstrumentID;

			if (pos.second.OpenDate < curDate )
			{
				if (pos.second.Direction == TradeDirectionType::BUY)
				{
					instr.PreviousLong += vol;
					cinstr.PreviousLong += vol;
					instr.TotalLong += vol;
					cinstr.TotalLong += vol;
				}
				else if (pos.second.Direction == TradeDirectionType::SELL)
				{
					instr.PreviousShort += vol;
					cinstr.PreviousShort += vol;
					instr.TotalShort += vol;
					cinstr.TotalShort += vol;
				}
			}
			else if (pos.second.OpenDate == curDate)
			{
				if (pos.second.Direction == TradeDirectionType::BUY)
				{
					instr.TodayLong += vol;
					cinstr.TodayLong += vol;
					instr.TotalLong += vol;
					cinstr.TotalLong += vol;
				}
				else if (pos.second.Direction == TradeDirectionType::SELL)
				{
					instr.TodayShort += vol;
					cinstr.TodayShort += vol;
					instr.TotalShort += vol;
					cinstr.TotalShort += vol;
				}
			}
		}
	}
	else
	{
		lk2.unlock();
		_pos.clear();
		for (const auto& pos : _detailPos)
		{
			int vol = pos.second.Volume; //- pos.second.CloseVolume;
			if (!vol)
				continue;
			auto& instr = _pos[pos.first];
			instr.InstrumentID = instr.InstrumentID.empty() ? pos.first : instr.InstrumentID;

			if (pos.second.OpenDate < curDate)
			{
				if (pos.second.Direction == TradeDirectionType::BUY)
				{
					instr.PreviousLong += vol;
					instr.TotalLong += vol;
				}
				else if (pos.second.Direction == TradeDirectionType::SELL)
				{
					instr.PreviousShort += vol;
					instr.TotalShort += vol;
				}
			}
			else if (pos.second.OpenDate == curDate)
			{
				if (pos.second.Direction == TradeDirectionType::BUY)
				{
					instr.TodayLong += vol;
					instr.TotalLong += vol;
				}
				else if (pos.second.Direction == TradeDirectionType::SELL)
				{
					instr.TodayShort += vol;
					instr.TotalShort += vol;
				}
			}
		}
	}
	
}

void Account::UpdatePosition(CThostFtdcInvestorPositionField* pInvestorPosition)
{
	//qDebug() << pInvestorPosition->InstrumentID << pInvestorPosition->LongFrozen << pInvestorPosition->ShortFrozen;
	std::lock_guard<std::mutex> lk(_cposMut);
	auto iter = _calculatedPos.find(pInvestorPosition->InstrumentID);
	if (iter == _calculatedPos.end())
	{
		AccountPosition tpos;
		tpos.InstrumentID = pInvestorPosition->InstrumentID;
		if (pInvestorPosition->PosiDirection == THOST_FTDC_PD_Long)
		{
			tpos.TodayLong = pInvestorPosition->TodayPosition;
			tpos.TotalLong = pInvestorPosition->Position;
			tpos.PreviousLong = tpos.TotalLong - tpos.TodayLong;
			tpos.FrozenTotalLong = pInvestorPosition->ShortFrozen;
		}
		else if (pInvestorPosition->PosiDirection == THOST_FTDC_PD_Short)
		{
			tpos.TodayShort = pInvestorPosition->TodayPosition;
			tpos.TotalShort = pInvestorPosition->Position;
			tpos.PreviousShort = tpos.TotalShort - tpos.TodayShort;
			tpos.FrozenTotalShort = pInvestorPosition->LongFrozen;
		}
		_calculatedPos.emplace(tpos.InstrumentID, tpos);
	}
	else
	{
		if (pInvestorPosition->PosiDirection == THOST_FTDC_PD_Long)
		{
			iter->second.TodayLong = pInvestorPosition->TodayPosition;
			iter->second.TotalLong = pInvestorPosition->Position;
			iter->second.PreviousLong = iter->second.TotalLong - iter->second.TodayLong;
			iter->second.FrozenTotalLong = pInvestorPosition->ShortFrozen;
		}
		else if (pInvestorPosition->PosiDirection == THOST_FTDC_PD_Short)
		{
			iter->second.TodayShort = pInvestorPosition->TodayPosition;
			iter->second.TotalShort = pInvestorPosition->Position;
			iter->second.PreviousShort = iter->second.TotalShort - iter->second.TodayShort;
			iter->second.FrozenTotalShort = pInvestorPosition->LongFrozen;
		}
	}
}

void Account::SetFrozen(const std::string& instrument, int change, TradeDirectionType direction,bool isYd)
{
	std::lock_guard<std::mutex> lk(_cposMut);
	auto iter = _calculatedPos.find(instrument);
	if (iter == _calculatedPos.end())
		return;
	if (direction == TradeDirectionType::SELL)
	{
		iter->second.FrozenTotalLong += change;
		if (isYd)
		{
			iter->second.FrozenPreviousLong += change;
		}
		else
		{
			iter->second.FrozenTodayLong += change;
		}
	}
	else if (direction == TradeDirectionType::BUY)
	{
		iter->second.FrozenTotalShort += change;
		if (isYd)
		{
			iter->second.FrozenPreviousShort += change;
		}
		else
		{
			iter->second.FrozenTodayShort += change;
		}
	}
}


