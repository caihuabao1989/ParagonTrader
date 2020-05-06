#include "OperationWindow.h"
#include "HandTradePage.h"
#include <qdebug.h>
#include <qboxlayout.h>
#include "MarketData.h"

OperationWindow* mainWindow = nullptr;
extern std::shared_ptr<MarketData> marketData;
extern LogSystem* globalLogger;

OperationWindow::OperationWindow() :
	menuBar(new QMenuBar(this)),
	mainWidget(new QWidget(this)),
	lbLogger(new QLabel(mainWidget)),
	statusBar(new QStatusBar(this)),
	accountTab(new QTabWidget(mainWidget)),
	lbSHFE(new QLabel(QStringLiteral("上期:"), statusBar)),
	lbDCE(new QLabel(QStringLiteral("大商:"), statusBar)),
	lbCZCE(new QLabel(QStringLiteral("郑商:"), statusBar)),
	lbFFEX(new QLabel(QStringLiteral("中金:"), statusBar)),
	lbINE(new QLabel(QStringLiteral("能源:"), statusBar)),
	lbSHFETime(new QLabel(statusBar)),
	lbDCETime(new QLabel(statusBar)),
	lbCZCETime(new QLabel(statusBar)),
	lbFFEXTime(new QLabel(statusBar)),
	lbINETime(new QLabel(statusBar))
{
	SetUpUi();
	globalLogger = this;
}

void OperationWindow::UpdateTime(const QStringList& timelist)
{
	//emit TimeUpdate(timelist);
	lbSHFETime->setText(timelist[0]);
	lbDCETime->setText(timelist[1]);
	lbCZCETime->setText(timelist[2]);
	lbFFEXTime->setText(timelist[3]);
	lbINETime->setText(timelist[4]);
}

void OperationWindow::AddAccount(AccountPage* page)
{
	emit addAccount(page);
}

void OperationWindow::SetApplication(QApplication* app)
{
	_app = app;
}

void OperationWindow::OpenOperationWindow()
{
	emit openOperationWindow();
}

void OperationWindow::Log(const QString& str)
{
	std::lock_guard<std::mutex> lk(_logMut);
	lbLogger->setText(str);
}

void OperationWindow::Log(const std::string& str)
{
	Log(QString::fromStdString(str));
}

void OperationWindow::SetUpUi()
{
	resize(1000, 600);
	statusBar->addWidget(lbSHFE);
	statusBar->addWidget(lbSHFETime);
	statusBar->addWidget(lbDCE);
	statusBar->addWidget(lbDCETime);
	statusBar->addWidget(lbCZCE);
	statusBar->addWidget(lbCZCETime);
	statusBar->addWidget(lbFFEX);
	statusBar->addWidget(lbFFEXTime);
	statusBar->addWidget(lbINE);
	statusBar->addWidget(lbINETime);
	auto layoutMainWidget = new QVBoxLayout;
	layoutMainWidget->addWidget(lbLogger);
	layoutMainWidget->addWidget(accountTab);
	mainWidget->setLayout(layoutMainWidget);
	setCentralWidget(mainWidget);

	QMenu* menuAddAccount = new QMenu(QStringLiteral("添加账号"), menuBar);
	QAction* actAddAccount = new QAction(QStringLiteral("添加一个账号"), menuBar);
	menuAddAccount->addAction(actAddAccount);
	menuBar->addMenu(menuAddAccount);
	setMenuBar(menuBar);
	setStatusBar(statusBar);
	connect(this, SIGNAL(openOperationWindow()), this, SLOT(show()));
	connect(this, SIGNAL(addAccount(void*)), this, SLOT(onAddAccount(void*)));
	connect(actAddAccount, SIGNAL(triggered()), this, SLOT(onActionAddAccount()));
	//connect(this, SIGNAL(close()), _app, SLOT(quit()));
}

void OperationWindow::closeEvent(QCloseEvent* event)
{
	_app->exit(0);
	marketData->GetSender().send(Done());
	event->accept();
}

void OperationWindow::onAddAccount(void* page)
{
	AccountPage* tp = static_cast<AccountPage*>(page);
	accountTab->insertTab(accountTab->count(), tp, tp->GetAccountID());
}

void OperationWindow::onActionAddAccount()
{
	auto newtrader = new CTPTrader;
	LoginDialog* ldialog = new LoginDialog(newtrader);
	AccountPage* page = new AccountPage(newtrader, this);
	ldialog->SetUpUi();
	ldialog->show();
	_traders.emplace(newtrader);
	newtrader->Detach();
}


