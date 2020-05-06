#pragma once
#include <qwidget.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qcompleter.h>
#include <qcheckbox.h>
#include <atomic>
#include <qcombobox.h>
#include "CTPTrader.h"
#include "TimerOrder.h"

class AccountPage;
class HandTradePage :public QWidget
{
	Q_OBJECT
public:
    HandTradePage(QWidget* parent=nullptr);
    void SetCompleter();
    static void UpdateCompleter(const QStringList& list);
    void SetAccountPage(AccountPage* page);
private:
	CTPTrader* _trader = nullptr;
	QLabel* lbInstrument, * lbBidTip, * lbAskTip, * lbBidPrice, * lbAskPrice, * lbBidVol, * lbAskVol;
	QLabel* lbTdVolume, * lbTdPrice, * lbUpperlimitTip, * lbLowerlimitTip, * lbUpperlimitPrice, * lbLowerlimitPrice;
    QSpinBox* sboxVolume;
    QDoubleSpinBox* sdboxPrice;
    QRadioButton* rbtnOpposit, * rbtnLocal, * rbtnLast;
    QPushButton* btnBuyOpen, * btnBuyClose, * btnSellOpen, * btnSellClose;
    QLineEdit* editInstrument;
    AccountPage* _page = nullptr;

    QCheckBox* chkTimeout, * chkAdversePrice;
    QSpinBox* sboxSeconds;
    QDoubleSpinBox* sdboxPriceMovement;
    QComboBox* combPriceType, *combAction;
    QLabel* lbActionTip;
    static std::shared_ptr<QCompleter> completer;
    static QStringList InstrumentList;
    
    QString m_currentInstrument;
    bool _isLimitSet = false;
    double curUpper = 0.0, curLower = 0.0;

    double GetBasePrice(TradeDirectionType direction);
    TimerOrderParam GetTimerOrderParam();
private slots:
    void onInstrumentTextChanged(const QString& instr);
    void onTickArrived(const Tick& tick);
    void onBtnBuyOpenClicked();
    void onBtnBuyCloseClicked();
    void onBtnSellOpenClicked();
    void onBtnSellCloseClicked();
    void onRBtnOppositAndLocal();
    void onRBtnLast();
};

