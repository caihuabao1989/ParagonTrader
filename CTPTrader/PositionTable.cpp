#include "PositionTable.h"
#include "TradeDataMessage.h"
#include "qdebug.h"

PositionTable::PositionTable(QWidget* parent) :
	QTableWidget(parent)
{
	QStringList headers;
    headers << QStringLiteral("ºÏÔ¼")
        << QStringLiteral("ÂòÂô")
        << QStringLiteral("³Ö²Ö")
        << QStringLiteral("×ò²Ö")
        << QStringLiteral("½ñ²Ö")
        << QStringLiteral("¶³½á");
    setColumnCount(headers.size());
    setHorizontalHeaderLabels(headers);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionBehavior(QAbstractItemView::SelectRows);
    setSelectionMode(QAbstractItemView::SingleSelection);
}

void PositionTable::Init(const std::unordered_map<std::string, AccountPosition>& map)
{
    int nrow = 0;
    for (const auto& pos : map)
    {
        //qDebug() << pos.first.c_str() << pos.second.TodayLong << pos.second.TotalShort;
        if (pos.second.TotalLong > 0)
        {
            insertRow(nrow);
            setItem(nrow, 0, new QTableWidgetItem(QString::fromStdString(pos.second.InstrumentID)));
            setItem(nrow, 1, new QTableWidgetItem(QStringLiteral("Âò")));
            item(nrow, 1)->setTextColor(Qt::red);
            setItem(nrow, 2, new QTableWidgetItem(QString::number(pos.second.TotalLong)));
            setItem(nrow, 3, new QTableWidgetItem(QString::number(pos.second.PreviousLong)));
            setItem(nrow, 4, new QTableWidgetItem(QString::number(pos.second.TodayLong)));
            setItem(nrow, 5, new QTableWidgetItem(QString::number(pos.second.FrozenTotalLong)));
            nrow++;
        }
        if (pos.second.TotalShort > 0)
        {
            insertRow(nrow);
            setItem(nrow, 0, new QTableWidgetItem(QString::fromStdString(pos.second.InstrumentID)));
            setItem(nrow, 1, new QTableWidgetItem(QStringLiteral("Âô")));
            item(nrow, 1)->setTextColor(Qt::green);
            setItem(nrow, 2, new QTableWidgetItem(QString::number(pos.second.TotalShort)));
            setItem(nrow, 3, new QTableWidgetItem(QString::number(pos.second.PreviousShort)));
            setItem(nrow, 4, new QTableWidgetItem(QString::number(pos.second.TodayShort)));
            setItem(nrow, 5, new QTableWidgetItem(QString::number(pos.second.FrozenTotalShort)));
            nrow++;
        }
    }
    resizeColumnsToContents();
    resizeRowsToContents();
}

void PositionTable::Update(const OrderTradeRecord& rec)
{
    if (rec.tradedVolume <= 0) return;
    auto instr = QString::fromStdString(rec.instrumentID);
    if (rec.combOffset == CombOffsetType::OPEN)
    {
        auto direc = TranslateTradeDirectionType(rec.direction);
        auto find = Search(instr, direc);
        if (find == -1)
        {
            int nrow = rowCount();
            insertRow(nrow);
            setItem(nrow, 0, new QTableWidgetItem(instr));
            setItem(nrow, 1, new QTableWidgetItem(direc));
            item(nrow, 1)->setTextColor(rec.direction == TradeDirectionType::BUY ? Qt::red : Qt::green);
            setItem(nrow, 2, new QTableWidgetItem(QString::number(rec.tradedVolume)));
            setItem(nrow, 3, new QTableWidgetItem("0"));
            setItem(nrow, 4, new QTableWidgetItem(QString::number(rec.tradedVolume)));
            setItem(nrow, 5, new QTableWidgetItem("0"));
        }
        else
        {
            item(find, 2)->setText(QString::number(item(find, 2)->text().toInt() + rec.tradedVolume));
            item(find, 4)->setText(QString::number(item(find, 4)->text().toInt() + rec.tradedVolume));
        }
    }
    else
    {
        auto direc = TranslateTradeDirectionType(rec.direction == TradeDirectionType::BUY ?
            TradeDirectionType::SELL :
            TradeDirectionType::BUY);
        auto find= Search(instr, direc);
        if (find == -1)
        {
            //todo::output error
        }
        else
        {
            int total = item(find, 2)->text().toInt();
            int prev = item(find, 3)->text().toInt();
            int td = item(find, 4)->text().toInt();
            int frozen = item(find, 5)->text().toInt();
            if (total < rec.tradedVolume)
            {
                //todo::output error
            }
            else if(total>rec.tradedVolume)
            {
                item(find, 2)->setText(QString::number(total - rec.tradedVolume));
                item(find, 5)->setText(QString::number(frozen - rec.tradedVolume));
            }
            else
            {
                removeRow(find);
                return;
            }
            if (td >= rec.tradedVolume)
            {
                item(find, 4)->setText(QString::number(td - rec.tradedVolume));
                item(find, 5)->setText(QString::number(frozen - rec.tradedVolume));
            }
            else if(td>0 && td< rec.tradedVolume && prev>=rec.tradedVolume-td)
            {
                item(find, 4)->setText("0");
                item(find, 3)->setText(QString::number(prev - rec.tradedVolume + td));
                item(find, 5)->setText(QString::number(frozen - rec.tradedVolume));
            }
            else if (!td && prev >= rec.tradedVolume)
            {
                item(find, 3)->setText(QString::number(prev - rec.tradedVolume));
                item(find, 5)->setText(QString::number(frozen - rec.tradedVolume));
            }
            else
            {
                //todo::output error
            }
        }
    }
}

void PositionTable::UpdateFrozenData(const std::string& instrument, TradeDirectionType direction,int change)
{
    auto find = Search(QString::fromStdString(instrument), direction==TradeDirectionType::BUY?QStringLiteral("Âô"): QStringLiteral("Âò"));
    qDebug() << "updatefrozen:" << find << instrument.c_str() << (int)direction << change;
    if (find < 0)
        return;
    int frozen = item(find, 5)->text().toInt();
    item(find, 5)->setText(QString::number(frozen + change));
    
}

int PositionTable::Search(const QString& instrumentID, const QString& direction)
{
    for (int i = 0; i < rowCount(); ++i)
    {
        if (item(i, 0)->text() == instrumentID && item(i, 1)->text() == direction)
            return i;
    }
    return -1;
}
