#include "HandTradePage.h"
#include "InstrumentInfo.h"
#include "MarketData.h"
#include "AccountPage.h"
#include "qdebug.h"

std::shared_ptr<QCompleter> HandTradePage::completer;
QStringList HandTradePage::InstrumentList;
//std::once_flag HandTradePage::flag;
extern std::shared_ptr<MarketData> marketData;
extern std::unordered_map<std::string, InstrumentInfo> instrumentInfo;

HandTradePage::HandTradePage(QWidget* parent) :
	QWidget(parent),
	lbInstrument(new QLabel(QStringLiteral("合约"), this)),
	lbBidTip(new QLabel(QStringLiteral("买一"), this)),
	lbAskTip(new QLabel(QStringLiteral("卖一"), this)),
	lbUpperlimitTip(new QLabel(QStringLiteral("涨停"), this)),
	lbLowerlimitTip(new QLabel(QStringLiteral("跌停"), this)),
	lbTdVolume(new QLabel(QStringLiteral("手数"), this)),
	lbTdPrice(new QLabel(QStringLiteral("价格"), this)),
	editInstrument(new QLineEdit(this)),
	lbBidPrice(new QLabel(this)),
	lbAskPrice(new QLabel(this)),
	lbBidVol(new QLabel(this)),
	lbAskVol(new QLabel(this)),
	lbUpperlimitPrice(new QLabel(this)),
	lbLowerlimitPrice(new QLabel(this)),
	sboxVolume(new QSpinBox(this)),
	sdboxPrice(new QDoubleSpinBox(this)),
	rbtnOpposit(new QRadioButton(QStringLiteral("对手价"), this)),
	rbtnLocal(new QRadioButton(QStringLiteral("本方价"), this)),
	rbtnLast(new QRadioButton(QStringLiteral("指定价"), this)),
	btnBuyOpen(new QPushButton(QStringLiteral("买开"), this)),
	btnBuyClose(new QPushButton(QStringLiteral("买平"), this)),
	btnSellOpen(new QPushButton(QStringLiteral("卖开"), this)),
	btnSellClose(new QPushButton(QStringLiteral("卖平"), this)),
	chkTimeout(new QCheckBox(QStringLiteral("指定时间过后撤单"),this)),
	chkAdversePrice(new QCheckBox(QStringLiteral("指定价格偏离撤单"), this)),
	sboxSeconds(new QSpinBox(this)),
	sdboxPriceMovement(new QDoubleSpinBox(this)),
	combPriceType(new QComboBox(this)),
	combAction(new QComboBox(this)),
	lbActionTip(new QLabel(QStringLiteral("撤单后:"),this))
{
	auto font = lbInstrument->font();
	font.setBold(true);
	font.setPointSize(15);
	lbInstrument->setFont(font);
	lbInstrument->setGeometry(10, 10, 85, 40);
	editInstrument->setFont(font);
	editInstrument->setGeometry(105, 10, 140, 40);
	lbAskPrice->setFont(font); lbBidPrice->setFont(font);
	auto uppalette = lbUpperlimitPrice->palette();
	auto lowpalette = lbLowerlimitPrice->palette();
	uppalette.setColor(QPalette::WindowText, Qt::red);
	lowpalette.setColor(QPalette::WindowText, Qt::green);
	lbUpperlimitPrice->setPalette(uppalette);
	lbLowerlimitPrice->setPalette(lowpalette);
	lbUpperlimitTip->setGeometry(10, 70, 60, 40); lbUpperlimitPrice->setGeometry(85, 70, 80, 40);
	lbAskTip->setGeometry(10, 120, 60, 40); lbAskPrice->setGeometry(85, 120, 80, 40); lbAskVol->setGeometry(205, 120, 40, 40);
	lbBidTip->setGeometry(10, 170, 60, 40); lbBidPrice->setGeometry(85, 170, 80, 40); lbBidVol->setGeometry(205, 170, 40, 40);
	lbLowerlimitTip->setGeometry(10, 220, 60, 40); lbLowerlimitPrice->setGeometry(85, 220, 80, 40);

	font.setPointSize(20);
	lbTdVolume->setFont(font); lbTdPrice->setFont(font);
	font.setPointSize(12);
	sboxVolume->setFont(font); sdboxPrice->setFont(font);
	lbTdVolume->setGeometry(250, 80, 65, 60); sboxVolume->setGeometry(325, 90, 125, 40);
	rbtnLast->setChecked(true);
	rbtnOpposit->setGeometry(250, 150, 60, 30); rbtnLocal->setGeometry(320, 150, 60, 30); rbtnLast->setGeometry(390, 150, 60, 30);
	lbTdPrice->setGeometry(250, 190, 65, 60); sdboxPrice->setGeometry(325, 200, 125, 40);

	font.setPointSize(20);
	btnBuyOpen->setFont(font); btnSellOpen->setFont(font);
	btnBuyClose->setFont(font); btnSellClose->setFont(font);
	auto buypalette = btnBuyOpen->palette();
	auto sellpalette = btnSellOpen->palette();
	buypalette.setColor(QPalette::ButtonText, Qt::red);
	sellpalette.setColor(QPalette::ButtonText, Qt::green);
	btnBuyOpen->setPalette(buypalette); btnBuyClose->setPalette(buypalette);
	btnSellOpen->setPalette(sellpalette); btnSellClose->setPalette(sellpalette);
	btnBuyOpen->setGeometry(20, 260, 100, 50); btnSellOpen->setGeometry(130, 260, 100, 50);
	btnBuyClose->setGeometry(20, 320, 100, 50); btnSellClose->setGeometry(130, 320, 100, 50);

	chkTimeout->setGeometry(250, 260, 120, 20);
	sboxSeconds->setGeometry(270, 285, 60, 20);
	sboxSeconds->setSuffix(QStringLiteral(" 秒"));
	sboxSeconds->setValue(10);
	chkAdversePrice->setGeometry(250, 310, 120, 20);
	combPriceType->setGeometry(270, 335, 60, 20);
	combPriceType->addItem(QStringLiteral("买一价"));
	combPriceType->addItem(QStringLiteral("卖一价")); 
	combPriceType->addItem(QStringLiteral("最新价"));
	sdboxPriceMovement->setGeometry(340, 335, 60, 20);
	sdboxPriceMovement->setSingleStep(0.1);
	sdboxPriceMovement->setValue(0.0);
	sdboxPriceMovement->setSuffix(QStringLiteral(" 点"));
	lbActionTip->setGeometry(250, 360, 50, 20);
	combAction->setGeometry(310, 360, 70, 20);
	combAction->addItem(QStringLiteral("无动作"));
	combAction->addItem(QStringLiteral("市价补单"));
	connect(editInstrument, SIGNAL(textChanged(QString)), this, SLOT(onInstrumentTextChanged(const QString&)));
	connect(btnBuyOpen, SIGNAL(clicked()), this, SLOT(onBtnBuyOpenClicked()));
	connect(btnBuyClose, SIGNAL(clicked()), this, SLOT(onBtnBuyCloseClicked()));
	connect(btnSellOpen, SIGNAL(clicked()), this, SLOT(onBtnSellOpenClicked()));
	connect(btnSellClose, SIGNAL(clicked()), this, SLOT(onBtnSellCloseClicked()));
	connect(rbtnOpposit, SIGNAL(clicked()), this, SLOT(onRBtnOppositAndLocal()));
	connect(rbtnLocal, SIGNAL(clicked()), this, SLOT(onRBtnOppositAndLocal()));
	connect(rbtnLast, SIGNAL(clicked()), this, SLOT(onRBtnLast()));
}

void HandTradePage::SetCompleter()
{
	if (!completer)
	{
		UpdateCompleter(GetAllInstrument());
		completer = std::make_shared<QCompleter>(InstrumentList);
		completer->setCaseSensitivity(Qt::CaseInsensitive);
	}
	editInstrument->setCompleter(completer.get());
}

void HandTradePage::UpdateCompleter(const QStringList& list)
{
	InstrumentList = list;	
}

void HandTradePage::SetAccountPage(AccountPage* page)
{
	_page = page;
	connect(page, SIGNAL(tickArrived(const Tick&)), this, SLOT(onTickArrived(const Tick&)));
}

double HandTradePage::GetBasePrice(TradeDirectionType direction)
{
	if (rbtnLast->isChecked())
		return sdboxPrice->value();
	else if (rbtnOpposit->isChecked())
	{
		return direction == TradeDirectionType::BUY ?
			(lbAskPrice->text().toDouble() + sdboxPrice->value()) :
			(lbBidPrice->text().toDouble() - sdboxPrice->value());
	}
	else
	{
		return direction == TradeDirectionType::BUY ?
			(lbBidPrice->text().toDouble() + sdboxPrice->value()) :
			(lbAskPrice->text().toDouble() - sdboxPrice->value());
	}
}

TimerOrderParam HandTradePage::GetTimerOrderParam()
{
	TimerOrderParam ret;
	if (chkTimeout->isChecked() && chkAdversePrice->isChecked())
		ret.deleteCondition = OrderDeleteCondition::TIME_OUT_AND_ADVERSE_PRICE_MOVEMENT;
	else if (chkTimeout->isChecked() && !chkAdversePrice->isChecked())
		ret.deleteCondition = OrderDeleteCondition::TIME_OUT;
	else if (!chkTimeout->isChecked() && chkAdversePrice->isChecked())
		ret.deleteCondition = OrderDeleteCondition::ADVERSE_PRICE_MOVEMENT;
	else
		return ret;
	ret.timeOutSeconds = sboxSeconds->value();
	ret.priceMovement = sdboxPriceMovement->value();
	auto ptstr = combPriceType->currentText();
	if (ptstr == QStringLiteral("卖一价"))
		ret.adversePriceType = PriceType::ASK;
	else if(ptstr == QStringLiteral("买一价"))
		ret.adversePriceType = PriceType::BID;
	else
		ret.adversePriceType = PriceType::LAST;
	auto actionstr = combAction->currentText();
	if (actionstr == QStringLiteral("无动作"))
		ret.action = ActionAfterDelete::NONE;
	else
		ret.action = ActionAfterDelete::MARKET_ORDER;
	return ret;
}

void HandTradePage::onTickArrived(const Tick& tick)
{
	if (tick.instrumentID != m_currentInstrument.toStdString())
		return;
	lbBidPrice->setNum(tick.bidPrice1);
	lbBidVol->setNum(tick.bidVolume1);
	lbAskPrice->setNum(tick.askPrice1);
	lbAskVol->setNum(tick.askVolume1);
	auto bidpalette = lbBidPrice->palette();
	auto askpalette = lbAskPrice->palette();
	if (tick.bidPrice1 > tick.preClose)
		bidpalette.setColor(QPalette::WindowText, Qt::red);
	else if (tick.bidPrice1 < tick.preClose)
		bidpalette.setColor(QPalette::WindowText, Qt::green);
	else
		bidpalette.setColor(QPalette::WindowText, Qt::black);
	if (tick.askPrice1 > tick.preClose)
		askpalette.setColor(QPalette::WindowText, Qt::red);
	else if (tick.askPrice1 < tick.preClose)
		askpalette.setColor(QPalette::WindowText, Qt::green);
	else
		askpalette.setColor(QPalette::WindowText, Qt::black);
	lbBidPrice->setPalette(bidpalette);
	lbAskPrice->setPalette(askpalette);
	if (!_isLimitSet)
	{
		lbUpperlimitPrice->setNum(tick.upperLimitPrice);
		lbLowerlimitPrice->setNum(tick.lowerLimitPrice);
		curUpper = tick.upperLimitPrice;
		curLower = tick.lowerLimitPrice;		
		if (rbtnLast->isChecked())
		{
			sdboxPrice->setRange(tick.lowerLimitPrice, tick.upperLimitPrice);
			sdboxPrice->setValue(tick.lastPrice);
		}
		_isLimitSet = true;
	}
}

void HandTradePage::onBtnBuyOpenClicked()
{
	auto timer = GetTimerOrderParam();
	OrderInsertParam param(editInstrument->text().toStdString(),
		GetBasePrice(TradeDirectionType::BUY),
		sboxVolume->value(),
		TradeDirectionType::BUY,
		CombOffsetType::OPEN);
	if (timer.timeOutSeconds < 0)
		_page->GetTraderSender().send(param);
	else
	{
		timer.baseParam = param;
		TimerOrder order;
		order._param = timer;
		_page->GetTraderSender().send(order);
	}
}

void HandTradePage::onBtnBuyCloseClicked()
{
	auto timer = GetTimerOrderParam();
	OrderInsertParam param(editInstrument->text().toStdString(),
		GetBasePrice(TradeDirectionType::BUY),
		sboxVolume->value(),
		TradeDirectionType::BUY,
		CombOffsetType::CLOSE);
	if (timer.timeOutSeconds < 0)
		_page->GetTraderSender().send(param);
	else
	{
		timer.baseParam = param;
		TimerOrder order;
		order._param = timer;
		_page->GetTraderSender().send(order);
	}
}

void HandTradePage::onBtnSellOpenClicked()
{
	auto timer = GetTimerOrderParam();
	OrderInsertParam param(editInstrument->text().toStdString(),
		GetBasePrice(TradeDirectionType::SELL),
		sboxVolume->value(),
		TradeDirectionType::SELL,
		CombOffsetType::OPEN);
	if (timer.timeOutSeconds < 0)
		_page->GetTraderSender().send(param);
	else
	{
		timer.baseParam = param;
		TimerOrder order;
		order._param = timer;
		_page->GetTraderSender().send(order);
	}
}

void HandTradePage::onBtnSellCloseClicked()
{
	auto timer = GetTimerOrderParam();
	OrderInsertParam param(editInstrument->text().toStdString(),
		GetBasePrice(TradeDirectionType::SELL),
		sboxVolume->value(),
		TradeDirectionType::SELL,
		CombOffsetType::CLOSE);
	if (timer.timeOutSeconds < 0)
		_page->GetTraderSender().send(param);
	else
	{
		timer.baseParam = param;
		TimerOrder order;
		order._param = timer;
		_page->GetTraderSender().send(order);
	}
}


void HandTradePage::onRBtnOppositAndLocal()
{
	lbTdPrice->setText(QStringLiteral("超价"));
	sdboxPrice->setRange(-10000.0, 10000.0);
	sdboxPrice->setValue(0.0);
}

void HandTradePage::onRBtnLast()
{
	lbTdPrice->setText(QStringLiteral("价格"));
	sdboxPrice->setRange(curLower, curUpper);
	sdboxPrice->setValue(lbBidPrice->text().toDouble());
}



void HandTradePage::onInstrumentTextChanged(const QString& instr)
{
	if (InstrumentList.contains(instr))
	{
		if(!m_currentInstrument.isEmpty())
			marketData->GetSender().send(UnSubscription<QString>(m_currentInstrument));
		m_currentInstrument = instr;
		marketData->GetSender().send(Subscription<QString>(instr));
		sdboxPrice->setSingleStep(instrumentInfo[instr.toStdString()].PriceTick);
		_isLimitSet = false;
	}
}
