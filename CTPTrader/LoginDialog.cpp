#include "LoginDialog.h"
#include "MarketData.h"
#include "CTPTrader.h"
#include "MarketDataMessage.h"
#include "TradeDataMessage.h"
#include <fstream>

extern std::shared_ptr<MarketData> marketData;
extern LogSystem* globalLogger;

std::once_flag LoginDialog::_flag;
nlohmann::json LoginDialog::brokerInfo;

LoginDialog::LoginDialog(CTPTrader* trader) :
	_trader(trader)
{
    _trader->SetLoginDialog(this);
    _trader->SetCurrentLogger(this);
    
    setAttribute(Qt::WA_DeleteOnClose);
   // SetUpUi();
}

void LoginDialog::SetUpUi()
{
    Vlay = new QVBoxLayout(this);
    VlayLabels = new QVBoxLayout;
    VlayInputs = new QVBoxLayout;
    HlayBtns = new QHBoxLayout;
    Hlay = new QHBoxLayout;

    lbBrokerID = new QLabel(QStringLiteral("�ڻ���˾"), this);
    lbUserID = new QLabel(QStringLiteral("�˺�"), this);
    lbPassWord = new QLabel(QStringLiteral("����"), this);
    VlayLabels->addWidget(lbBrokerID);
    VlayLabels->addWidget(lbUserID);
    VlayLabels->addWidget(lbPassWord);

    combboxSelectTrader = new QComboBox(this);
    //combboxSelectTrader->addItems(_trader->GetBrokerIDList());
    editUserID = new QLineEdit(this);
    editPassWord = new QLineEdit(this);
    VlayInputs->addWidget(combboxSelectTrader);
    VlayInputs->addWidget(editUserID);
    VlayInputs->addWidget(editPassWord);

    btnOK = new QPushButton(QStringLiteral("ȷ��"), this);
    btnCancel = new QPushButton(QStringLiteral("ȡ��"), this);
    HlayBtns->addWidget(btnOK);
    HlayBtns->addWidget(btnCancel);

    Hlay->addLayout(VlayLabels);
    Hlay->addLayout(VlayInputs);

    lbLog = new QLabel(this);
    Vlay->addLayout(Hlay);
    Vlay->addLayout(HlayBtns);
    Vlay->addWidget(lbLog);
    std::call_once(_flag, &LoginDialog::GetBrokerInfo);
    combboxSelectTrader->addItems(GetBrokerNames());
    connect(btnOK, SIGNAL(clicked()), this, SLOT(onBtnOK()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(onBtnCancel()));
    connect(this, SIGNAL(closeDialog()), this, SLOT(close()));
    connect(this, SIGNAL(enableBtnOK(bool)), btnOK, SLOT(setEnabled(bool)));
}

void LoginDialog::CloseDialog()
{
    _trader->SetLoginDialog(nullptr);
    _trader->SetCurrentLogger(nullptr);
    emit closeDialog();
}

void LoginDialog::Join()
{
    //std::thread(&CTPTrader::Join, _trader).detach();
}

void LoginDialog::EnableBtnOK(bool flag)
{
    emit enableBtnOK(flag);
}

bool LoginDialog::GetBrokerInfo()
{
    std::ifstream in("brokerInfo.json");
    if (!in.is_open())
    {
        globalLogger->Log(QStringLiteral("Error:δ���ҵ�brokerInfo�����ļ�."));
        return false;
    }
    try
    {
        in >> brokerInfo;
        return true;
    }
    catch (const std::exception& e)
    {
        globalLogger->Log(QStringLiteral("Error:brokerInfo�����ļ���ʽ��."));
        return false;
    }
}

QStringList LoginDialog::GetBrokerNames()
{
    QStringList ret;
    for (const auto& item : brokerInfo)
        ret << QString::fromStdString(item["brokerName"]);
    return ret;
}

void LoginDialog::Log(const QString& log)
{
    std::lock_guard<std::mutex> lk(_logMut);
    lbLog->setText(log);
}

void LoginDialog::Log(const std::string& log)
{
    std::lock_guard<std::mutex> lk(_logMut);
    lbLog->setText(QString::fromStdString(log));
}

void LoginDialog::onBtnOK()
{
    auto broker = combboxSelectTrader->currentText();
    auto user = editUserID->text();
    auto password = editPassWord->text();
    if (!_trader->IsFrontConnected() || !_trader->IsAccountLogin())
    {
        if(broker.isEmpty() || user.isEmpty() || password.isEmpty())
            Log(QStringLiteral("��������д��½��Ϣ!"));
        else
        {
            _trader->GetSender().send(TraderLoginInfo(broker, user, password));
            btnOK->setDisabled(true);
        }
        
    } 
}

void LoginDialog::onBtnCancel() {
    close();
    exit(0);
}

FirstLoginDialog::FirstLoginDialog(CTPTrader* trader) :
    LoginDialog(trader)
{
    _trader->SetLoginDialog(this);
    _trader->SetCurrentLogger(this);
    globalLogger = this;
    setAttribute(Qt::WA_DeleteOnClose);
}

/*void FirstLoginDialog::SetUpUi()
{
    Vlay = new QVBoxLayout(this);
    VlayLabels = new QVBoxLayout;
    VlayInputs = new QVBoxLayout;
    HlayBtns = new QHBoxLayout;
    Hlay = new QHBoxLayout;

    lbQuote = new QLabel(QStringLiteral("����Դ"), this);
    lbBrokerID = new QLabel(QStringLiteral("�ڻ���˾ID"), this);
    lbUserID = new QLabel(QStringLiteral("�˺�"), this);
    lbPassWord = new QLabel(QStringLiteral("����"), this);
    VlayLabels->addWidget(lbQuote);
    VlayLabels->addWidget(lbBrokerID);
    VlayLabels->addWidget(lbUserID);
    VlayLabels->addWidget(lbPassWord);

    combboxSelectQuote = new QComboBox(this);
    combboxSelectQuote->addItems(marketData->GetBrokerIDList());
    combboxSelectTrader = new QComboBox(this);
    //combboxSelectTrader->addItems(_trader->GetBrokerIDList());
    editUserID = new QLineEdit(this);
    editPassWord = new QLineEdit(this);
    VlayInputs->addWidget(combboxSelectQuote);
    VlayInputs->addWidget(combboxSelectTrader);
    VlayInputs->addWidget(editUserID);
    VlayInputs->addWidget(editPassWord);

    btnOK = new QPushButton(QStringLiteral("ȷ��"), this);
    btnCancel = new QPushButton(QStringLiteral("ȡ��"), this);
    HlayBtns->addWidget(btnOK);
    HlayBtns->addWidget(btnCancel);

    Hlay->addLayout(VlayLabels);
    Hlay->addLayout(VlayInputs);

    lbLog = new QLabel(this);
    Vlay->addLayout(Hlay);
    Vlay->addLayout(HlayBtns);
    Vlay->addWidget(lbLog);
    std::call_once(_flag, &LoginDialog::GetBrokerInfo);
    combboxSelectTrader->addItems(GetBrokerNames());
    connect(btnOK, SIGNAL(clicked()), this, SLOT(onBtnOK()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(onBtnCancel()));
    connect(this, SIGNAL(closeDialog()), this, SLOT(close()));
    connect(this, SIGNAL(enableBtnOK(bool)), btnOK, SLOT(setEnabled(bool)));
}*/

void FirstLoginDialog::onBtnOK()
{
    auto broker = combboxSelectTrader->currentText();
    auto user = editUserID->text();
    auto password = editPassWord->text();
    if (!marketData->IsFrontConnected() || !marketData->IsAccountLogin())
     {
         if(broker.isEmpty())
             Log(QStringLiteral("��������д��½��Ϣ!"));
         else
             marketData->GetSender().send(BrokerChoice(broker));
     }
    if (!_trader->IsFrontConnected() || !_trader->IsAccountLogin())
    {
        if (broker.isEmpty() || user.isEmpty() || password.isEmpty())
            Log(QStringLiteral("��������д��½��Ϣ!"));
        else
        {
            _trader->GetSender().send(TraderLoginInfo(broker, user, password));
            btnOK->setDisabled(true);
        }
    }
    
}

