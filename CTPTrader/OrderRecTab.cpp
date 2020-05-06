#include "OrderRecTab.h"
#include "AccountPage.h"
#include "qdebug.h"

OrderRecTab::OrderRecTab(QWidget* parent):
	QTabWidget(parent),
	orderRec(new QTableWidget(this)),
	tradeRec(new QTableWidget(this))
{
    QStringList orderRecHeader, tradeRecHeader;
    orderRecHeader << QStringLiteral("报单编号")
        << QStringLiteral("合约")
        << QStringLiteral("买卖")
        << QStringLiteral("开平")
        << QStringLiteral("挂单状态")
        << QStringLiteral("成交状态")
        << QStringLiteral("报单价格")
        << QStringLiteral("报单手数")
        << QStringLiteral("未成交手数")
        << QStringLiteral("成交手数")
        << QStringLiteral("报单时间")
        << QStringLiteral("交易所");
    tradeRecHeader << QStringLiteral("成交编号")
        << QStringLiteral("合约")
        << QStringLiteral("买卖")
        << QStringLiteral("开平")
        << QStringLiteral("成交手数")
        << QStringLiteral("成交价格")
        << QStringLiteral("成交时间")
        << QStringLiteral("报单编号")
        << QStringLiteral("交易所");
    orderRec->setColumnCount(orderRecHeader.size());
    orderRec->setHorizontalHeaderLabels(orderRecHeader);
    orderRec->setEditTriggers(QAbstractItemView::NoEditTriggers);
    orderRec->setSelectionBehavior(QAbstractItemView::SelectRows);
    orderRec->setSelectionMode(QAbstractItemView::SingleSelection);
    tradeRec->setColumnCount(tradeRecHeader.size());
    tradeRec->setHorizontalHeaderLabels(tradeRecHeader);
    tradeRec->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tradeRec->setSelectionBehavior(QAbstractItemView::SelectRows);
    tradeRec->setSelectionMode(QAbstractItemView::SingleSelection);
    insertTab(0, orderRec, QStringLiteral("委托记录"));
    insertTab(1, tradeRec, QStringLiteral("成交记录"));
    connect(orderRec, SIGNAL(cellDoubleClicked(int, int)), this, SLOT(onOrderRecRowDoubleClicked(int, int)));
}

void OrderRecTab::OrderRecUpdate(const OrderSubmitRecord& rec)
{
    auto curNRow = orderRec->rowCount();
    auto curNCol = orderRec->columnCount();
    int find = -1;
    if (!std::string(rec.orderSysID).empty())
    {
        for (int i = 0; i < curNRow; ++i)
        {
            if (orderRec->item(i, 0)->text().toInt() == std::stoi(rec.orderSysID) && orderRec->item(i, 11)->text().toStdString() == rec.exchangeID)
            {
                find = i;
                break;
            }
        }
    }
    if (find == -1)
    {
        RowsMoveDown(orderRec);
        orderRec->setItem(0, 0, new QTableWidgetItem(/*QString::number*/(rec.orderSysID)));
        orderRec->setItem(0, 1, new QTableWidgetItem(rec.instrumentID.c_str()));
        orderRec->setItem(0, 2, new QTableWidgetItem(TranslateTradeDirectionType(rec.direction)));
        orderRec->setItem(0, 3, new QTableWidgetItem(TranslateComboOffsetType(rec.combOffset)));
        orderRec->setItem(0, 4, new QTableWidgetItem(TranslateOrderSubmitStatus(rec.submitStatus)));
        orderRec->setItem(0, 5, new QTableWidgetItem(TranslateOrderTradeStatus(rec.tradeStatus)));
        orderRec->setItem(0, 6, new QTableWidgetItem(QString::number(rec.orderPrice)));
        orderRec->setItem(0, 7, new QTableWidgetItem(QString::number(rec.orderVolume)));
        orderRec->setItem(0, 8, new QTableWidgetItem(QString::number(rec.unfilledVolume)));
        orderRec->setItem(0, 9, new QTableWidgetItem(QString::number(rec.filledVolume)));
        orderRec->setItem(0, 10, new QTableWidgetItem(rec.submitTime.time_str()));
        orderRec->setItem(0, 11, new QTableWidgetItem(rec.exchangeID.c_str()));
    }
    else
    {
        orderRec->item(find, 4)->setText(TranslateOrderSubmitStatus(rec.submitStatus));
        orderRec->item(find, 5)->setText(TranslateOrderTradeStatus(rec.tradeStatus));
        orderRec->item(find, 8)->setText(QString::number(rec.unfilledVolume));
        orderRec->item(find, 9)->setText(QString::number(rec.filledVolume));
    }
    orderRec->resizeColumnsToContents();
    orderRec->resizeRowsToContents();
}

void OrderRecTab::TradeRecUpdate(const OrderTradeRecord& rec)
{
    RowsMoveDown(tradeRec);
    tradeRec->setItem(0, 0, new QTableWidgetItem(QString(rec.tradeID)));
    tradeRec->setItem(0, 1, new QTableWidgetItem(rec.instrumentID.c_str()));
    tradeRec->setItem(0, 2, new QTableWidgetItem(TranslateTradeDirectionType(rec.direction)));
    tradeRec->setItem(0, 3, new QTableWidgetItem(TranslateComboOffsetType(rec.combOffset)));
    tradeRec->setItem(0, 4, new QTableWidgetItem(QString::number(rec.tradedVolume)));
    tradeRec->setItem(0, 5, new QTableWidgetItem(QString::number(rec.tradedPrice)));
    tradeRec->setItem(0, 6, new QTableWidgetItem(rec.tradedTime.time_str()));
    tradeRec->setItem(0, 7, new QTableWidgetItem(rec.orderSysID));
    tradeRec->setItem(0, 8, new QTableWidgetItem(rec.exchangeID.c_str()));
    tradeRec->resizeColumnsToContents();
    tradeRec->resizeRowsToContents();
}

void OrderRecTab::SetAccountPage(AccountPage* page)
{
    _page = page;
}

void OrderRecTab::RowsMoveDown(QTableWidget* table)
{
    auto curNRow = table->rowCount();
    auto curNCol = table->columnCount();
    table->insertRow(curNRow);
    for (int i = curNRow - 1; i >= 0; --i)
    {
        for (int j = 0; j < curNCol; ++j)
        {
            table->setItem(i + 1, j, table->takeItem(i, j));
        }
    }
}

void OrderRecTab::onOrderRecRowDoubleClicked(int row, int col)
{
    auto status = orderRec->item(row, 5)->text();
    if (status == QStringLiteral("全部成交")
        || status == QStringLiteral("部成部撤")
        || status == QStringLiteral("报单被CTP拒绝")
        || status == QStringLiteral("被撤"))
        return;
    else
    {
        _page->GetTraderSender().send(OrderDeleteParamBySysID(
            orderRec->item(row, 0)->text().toInt(),
            orderRec->item(row, 11)->text().toStdString()
        ));
    }
}
