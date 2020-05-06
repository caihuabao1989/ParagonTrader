#pragma once
#include <qstring.h>

class LogSystem
{
public:
    virtual void Log(const QString& log) {};
    virtual void Log(const std::string& log) {};
    virtual ~LogSystem() {}
};