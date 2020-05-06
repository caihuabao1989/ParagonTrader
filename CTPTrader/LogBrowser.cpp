#include "LogBrowser.h"
#include "MarketTime.h"


extern std::shared_ptr<MarketTime> marketTime;
void LogBrowser::Log(const QString& msg)
{
    std::lock_guard<std::mutex> lk(mut);
    QString message = "[" + QString(marketTime->GetLocalTime().time_str()) + "] ";
    message.append(msg);
    append(message);
}

void LogBrowser::Log(const std::string& msg)
{
    Log(QString::fromStdString(msg));
}
