#pragma once
#include <QTextBrowser>
#include <mutex>
#include "LogSystem.h"

class LogBrowser :public QTextBrowser, public LogSystem
{
public:
    LogBrowser(QWidget* parent=nullptr):QTextBrowser(parent){}
    void Log(const QString&) override;
    void Log(const std::string&) override;
private:
    std::mutex mut;
};
