//#include "stdafx.h"
#include "MainWindow.h"
#include <QtWidgets/QApplication>
#include "date.h"
#include <qdebug.h>
#include "CTPTrader.h"
#include "MarketData.h"
#include "MarketTime.h"
#include "OperationWindow.h"

#pragma comment(lib,"StrategyAPI.lib")
#pragma comment(lib,"thostmduserapi_se.lib")
#pragma comment(lib,"thosttraderapi_se.lib")

extern std::shared_ptr<MarketData> marketData;
extern std::shared_ptr<MarketTime> marketTime;
extern OperationWindow* mainWindow;

int main(int argc, char *argv[])
{
	qRegisterMetaType<QVector<int>>("QVector<int>");
	qRegisterMetaType<QItemSelection>("QItemSelection");
	qRegisterMetaType<Tick>("Tick");
	QApplication* a=new QApplication(argc, argv);
	//a.setMainWidget
	a->connect(a, SIGNAL(lastWindowClosed()), a, SLOT(quit()));
	mainWindow = new OperationWindow;
	mainWindow->SetApplication(a);
	CTPTrader trader0;
	FirstLoginDialog* fdialog=new FirstLoginDialog(&trader0);
	AccountPage* page = new AccountPage(&trader0, mainWindow);

	fdialog->SetUpUi();
	fdialog->show();
	
	int ret = a->exec();
	marketTime->Join();
	marketData->Join();
	trader0.Join();
	
	delete a;
	return ret;
}
