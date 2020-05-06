#include "InstrumentInfo.h"
#include <unordered_map>

std::unordered_map<std::string, InstrumentInfo> instrumentInfo;

QStringList GetAllInstrument()
{
	QStringList ret;
	for (const auto& instr : instrumentInfo)
	{
		ret << QString::fromStdString(instr.first);
	}
	return ret;
}
