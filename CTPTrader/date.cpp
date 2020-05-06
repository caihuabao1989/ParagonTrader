#include "date.h"
#include <iostream>

bool is_leap_year(int year) {
	if (year % 4)
		return false;
	else if (year % 100)
		return true;
	else if (year % 400)
		return false;
	return true;
}

int get_month_days(int year, int month) {
	if (year < 0 || month < 1 || month>12) {
		std::cerr << InvalidDate().what() << std::endl;
		throw InvalidDate();
	}
	switch (month) {
	case 4:
	case 6:
	case 9:
	case 11:
		return 30;
	case 2:
		return is_leap_year(year) ? 29 : 28;
	default:
		return 31;
	}
}

chbDate::chbDate(int year, int month, int day) :_year(year), _month(month), _day(day) {
	if (!valid_date_check()) {
		std::cerr << InvalidDate().what() << std::endl;
		throw InvalidDate();
	}
}

chbDate::chbDate(int datenum) :_year(datenum / 10000), _month(datenum / 100 - _year * 100), _day(datenum % 100) {
	if (!valid_date_check()) {
		std::cerr << InvalidDate().what() << std::endl;
		throw InvalidDate();
	}
}

chbDate::chbDate(const char* datestr, char delimiter) {
	std::istringstream ss(datestr);
	std::string cell;
	std::getline(ss, cell, delimiter); _year = std::stoi(cell);
	std::getline(ss, cell, delimiter); _month = std::stoi(cell);
	std::getline(ss, cell, delimiter); _day = std::stoi(cell);
	if (!valid_date_check()) {
		std::cerr << InvalidDate().what() << std::endl;
		throw InvalidDate();
	}
}

chbDate::chbDate(const char* datestr) {
	std::string str = datestr;
	std::string cell;
	std::vector<int> store;
	for (const auto& ch : str) {
		if (isdigit(ch))
			cell.push_back(ch);
		else {
			store.push_back(std::stoi(cell));
			cell.clear();
		}
	}
	store.push_back(std::stoi(cell));
	_year = store[0];
	_month = store[1];
	_day = store[2];
	if (!valid_date_check()) {
		std::cerr << InvalidDate().what() << std::endl;
		throw InvalidDate();
	}
}

const char* chbDate::date_str(const char* delimiter) const {
	static char ret[256];
	std::string rt = std::to_string(year()) + (month() < 10 ? "0" : "") + delimiter + std::to_string(month()) + (day() < 10 ? "0" : "") + delimiter + std::to_string(day());
	strcpy_s(ret, rt.c_str());
	return ret;
}

int chbDate::weekday() const {
	int y = year();
	int m = month();
	int d = day();
	y -= m < 3;
	int day_of_week = (y + y / 4 - y / 100 + y / 400 + "-bed=pen+mad."[m] + d) % 7;
	return day_of_week;
}

chbDate& chbDate::advance(int years, int months, int days) {
	_year += years; _month += months; _day += days;
	while (_month > 12) {
		++_year;
		_month -= 12;
	}
	while (_month <= 0) {
		--_year;
		_month += 12;
	}
	while (_day > get_month_days(_year, _month)) {
		_day -= get_month_days(_year, _month++);
		while (_month > 12) {
			++_year;
			_month -= 12;
		}
	}
	while (_day <= 0) {
		_day += get_month_days(_year, _month--);
		while (_month <= 0) {
			--_year;
			_month += 12;
		}
	}
	if (!valid_date_check()) {
		std::cerr << InvalidDate().what() << std::endl;
		throw InvalidDate();
	}
	return *this;
}

chbDate& chbDate::operator+(int days) {
	if (days < 0) return *this - (-1 * days);
	_day += days;
	while (_day > get_month_days(_year, _month)) {
		_day -= get_month_days(_year, _month++);
		while (_month > 12) {
			++_year;
			_month -= 12;
		}
	}
	if (!valid_date_check()) {
		std::cerr << InvalidDate().what() << std::endl;
		throw InvalidDate();
	}
	return *this;
}

chbDate& chbDate::operator-(int days) {
	if (days < 0) return *this + (-1 * days);
	_day -= days;
	while (_day <= 0) {
		_day += get_month_days(_year, --_month);
		while (_month <= 0) {
			--_year;
			_month += 12;
		}
	}
	if (!valid_date_check()) {
		std::cerr << InvalidDate().what() << std::endl;
		throw InvalidDate();
	}
	return *this;
}

bool chbDate::valid_date_check() {
	return !(_year < 0 || _month < 1 || _month>12 || _day < 1 || _day>get_month_days(_year, _month));
}

bool operator==(const chbDate& lhs, const chbDate& rhs) {
	return lhs.date_num() == rhs.date_num();
}

bool operator!=(const chbDate& lhs, const chbDate& rhs) {
	return !(lhs == rhs);
}

bool operator>(const chbDate& lhs, const chbDate& rhs) {
	return lhs.date_num() > rhs.date_num();
}

bool operator<(const chbDate& lhs, const chbDate& rhs) {
	return lhs.date_num() < rhs.date_num();
}

bool operator>=(const chbDate& lhs, const chbDate& rhs) {
	return lhs.date_num() >= rhs.date_num();
}

bool operator<=(const chbDate& lhs, const chbDate& rhs) {
	return lhs.date_num() <= rhs.date_num();
}

int operator-(const chbDate& lhs, const chbDate& rhs) {
	if (lhs < rhs) return -1 * (rhs - lhs);
	int ret = 0;
	int tyear = rhs.year(), tmonth = rhs.month();
	while (tyear < lhs.year()) {
		while (tmonth < 13) {
			ret += get_month_days(tyear, tmonth++);
		}
		++tyear;
		tmonth -= 12;
	}
	while (tmonth < lhs.month()) {
		ret += get_month_days(lhs.year(), tmonth++);
	}
	return ret + lhs.day() - rhs.day();
}

std::ostream& operator<<(std::ostream& os, const chbDate& date) {
	os << date.date_num();
	return os;
}

chbTime::chbTime(int hour, int minute, int second) :_hour(hour), _minute(minute), _second(second) {
	if (!valid_time_check()) {
		std::cerr << InvalidTime().what() << std::endl;
		throw InvalidTime();
	}
}

chbTime::chbTime(int timenum) :_hour(timenum / 10000), _minute(timenum / 100 - _hour * 100), _second(timenum % 100) {
	if (!valid_time_check()) {
		std::cerr << InvalidTime().what() << std::endl;
		throw InvalidTime();
	}
}

chbTime::chbTime(const char* datestr, char delimiter)
{
	std::istringstream ss(datestr);
	std::string cell;
	std::getline(ss, cell, delimiter); _hour = std::stoi(cell);
	std::getline(ss, cell, delimiter); _minute = std::stoi(cell);
	std::getline(ss, cell, delimiter); _second = std::stoi(cell);
	if (!valid_time_check()) {
		std::cerr << InvalidDate().what() << std::endl;
		throw InvalidDate();
	}
}

chbTime::chbTime(const char* datestr)
{
	std::string str = datestr;
	std::string cell;
	std::vector<int> store;
	for (const auto& ch : str) {
		if (isdigit(ch))
			cell.push_back(ch);
		else {
			store.push_back(std::stoi(cell));
			cell.clear();
		}
	}
	store.push_back(std::stoi(cell));
	_hour = store[0];
	_minute = store[1];
	_second = store[2];
	if (!valid_time_check()) {
		std::cerr << InvalidDate().what() << std::endl;
		throw InvalidDate();
	}
}

const char* chbTime::time_str(const char* delimiter) const {
	static char ret[256];
	std::string rt = (hour() < 10 ? "0" : "") + std::to_string(hour()) + delimiter + (minute() < 10 ? "0" : "") + std::to_string(minute()) + delimiter + (second() < 10 ? "0" : "") + std::to_string(second());
	strcpy_s(ret, rt.c_str());
	return ret;
}

chbTime& chbTime::advance(int hours, int minutes, int seconds) {
	_hour += hours; _minute += minutes; _second += seconds;
	while (_second >= 60) {
		_second -= 60;
		++_minute;
	}
	while (_second < 0) {
		_second += 60;
		--_minute;
	}
	while (_minute >= 60) {
		_minute -= 60;
		++_hour;
	}
	while (_minute < 0) {
		_minute += 60;
		--_hour;
	}
	while (_hour >= 24)
		_hour -= 24;
	while (_hour < 0)
		_hour += 24;
	if (!valid_time_check()) {
		std::cerr << InvalidTime().what() << std::endl;
		throw InvalidTime();
	}
	return *this;
}

bool chbTime::valid_time_check() {
	return !(_hour < 0 || _hour >= 24 || _minute < 0 || _minute >= 60 || _second < 0 || _second >= 60);
}

bool operator==(const chbTime& lhs, const chbTime& rhs) {
	return (lhs.hour() == rhs.hour() && lhs.minute() == rhs.minute() && lhs.second() == rhs.second());
}

bool operator!=(const chbTime& lhs, const chbTime& rhs) {
	return !(lhs == rhs);
}

bool operator>(const chbTime& lhs, const chbTime& rhs) {
	return lhs.time_num() > rhs.time_num();
}

bool operator<(const chbTime& lhs, const chbTime& rhs) {
	return lhs.time_num() < rhs.time_num();
}

bool operator>=(const chbTime& lhs, const chbTime& rhs) {
	return lhs.time_num() >= rhs.time_num();
}

bool operator<=(const chbTime& lhs, const chbTime& rhs) {
	return lhs.time_num() <= rhs.time_num();
}

int operator-(const chbTime& lhs, const chbTime& rhs) {
	return lhs.seconds_count() - rhs.seconds_count();
}

std::ostream& operator<<(std::ostream& os, const chbTime& time) {
	os << time.time_num();
	return os;
}

const char* chbDateTime::datetime_str(const char* date_delimiter, const char* time_delimiter) const {
	static char ret[256];
	strcpy_s(ret, date_str(date_delimiter));
	strcat_s(ret, " ");
	strcat_s(ret, time_str(time_delimiter));
	return ret;
}

chbDateTime& chbDateTime::advance(int years, int months, int days, int hours, int minutes, int seconds) {
	chbDate::advance(years, months, days);
	chbTime::advance(hours, minutes, seconds);
	return *this;
}

chbDateTime& chbDateTime::operator+(int days) {
	chbDate::operator+(days);
	return *this;
}

chbDateTime& chbDateTime::operator-(int days) {
	chbDate::operator-(days);
	return *this;
}

chbDateTime chbDateTime::get_system_time() {
	time_t tt = ::time(nullptr);
	tm* t = localtime(&tt);
	return chbDateTime(t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
}

bool operator==(const chbDateTime& lhs, const chbDateTime& rhs) {
	return (lhs.year() == rhs.year() && lhs.month() == rhs.month() && lhs.day() == rhs.day() && lhs.hour() == rhs.hour() && lhs.minute() == rhs.minute() && lhs.second() == rhs.second());
}

bool operator!=(const chbDateTime& lhs, const chbDateTime& rhs) {
	return !(lhs == rhs);
}

bool operator>(const chbDateTime& lhs, const chbDateTime& rhs) {
	return ((lhs.date() > rhs.date()) || (lhs.date() == rhs.date() && lhs.time() > rhs.time()));
}

bool operator<(const chbDateTime& lhs, const chbDateTime& rhs) {
	return ((lhs.date() < rhs.date()) || (lhs.date() == rhs.date() && lhs.time() < rhs.time()));
}

bool operator>=(const chbDateTime& lhs, const chbDateTime& rhs) {
	return (lhs > rhs || lhs == rhs);
}

bool operator<=(const chbDateTime& lhs, const chbDateTime& rhs) {
	return (lhs < rhs || lhs == rhs);
}

std::pair<int, int> operator-(const chbDateTime& lhs, const chbDateTime& rhs) {
	return { lhs.date() - rhs.date(),lhs.time() - rhs.time() };
}