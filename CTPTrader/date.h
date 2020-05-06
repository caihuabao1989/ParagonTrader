#pragma once
#include <string>
#include <utility>
#include <iostream>
#include <sstream>
#include <vector>
#include <ctime>


bool is_leap_year(int year);
int get_month_days(int year, int month);

class InvalidDate {
public:
	std::string what() { return "Invalid Date."; }
};

class InvalidTime {
public:
	std::string what() { return "Invalid Time."; }
};

class chbDate {
public:
	chbDate() = default;
	chbDate(int year, int month, int day);
	chbDate(int datenum);
	chbDate(const char* datestr, char delimiter);
	chbDate(const char* datestr);

	int date_num() const { return _year * 10000 + _month * 100 + _day; }
	const char* date_str(const char* delimiter = "") const;

	int year()  const { return _year; }
	int month() const { return _month; }
	int day()   const { return _day; }
	bool is_month_end() const { return _day == get_month_days(_year, _month); }
	int weekday() const;

	bool leap_year() const { return is_leap_year(_year); }

	virtual chbDate& advance(int years, int months, int days);

	chbDate& operator+(int days);

	chbDate& operator-(int days);
private:
	int _year{ 1900 }, _month{ 1 }, _day{ 1 };

	bool valid_date_check();
};


bool operator==(const chbDate& lhs, const chbDate& rhs);

bool operator!=(const chbDate& lhs, const chbDate& rhs);

bool operator>(const chbDate& lhs, const chbDate& rhs);

bool operator<(const chbDate& lhs, const chbDate& rhs);

bool operator>=(const chbDate& lhs, const chbDate& rhs);

bool operator<=(const chbDate& lhs, const chbDate& rhs);

int operator-(const chbDate& lhs, const chbDate& rhs);

std::ostream& operator<<(std::ostream& os, const chbDate& date);


class chbTime {
public:
	chbTime() = default;
	chbTime(int hour, int minute, int second);
	chbTime(int timenum);
	chbTime(const char* datestr, char delimiter);
	chbTime(const char* datestr);

	int time_num() const { return _hour * 10000 + _minute * 100 + _second; }
	const char* time_str(const char* delimiter = ":") const;

	int hour() const { return _hour; }
	int minute() const { return _minute; }
	int second() const { return _second; }

	int seconds_count() const { return _hour * 3600 + _minute * 60 + _second; }
	virtual chbTime& advance(int hours, int minutes, int seconds);

	chbTime& operator+(int seconds) { return advance(0, 0, seconds); }
	chbTime& operator-(int seconds) { return advance(0, 0, -1 * seconds); }
private:
	int _hour = 0, _minute = 0, _second = 0;

	bool valid_time_check();
};

bool operator==(const chbTime& lhs, const chbTime& rhs);

bool operator!=(const chbTime& lhs, const chbTime& rhs);

bool operator>(const chbTime& lhs, const chbTime& rhs);

bool operator<(const chbTime& lhs, const chbTime& rhs);

bool operator>=(const chbTime& lhs, const chbTime& rhs);

bool operator<=(const chbTime& lhs, const chbTime& rhs);

int operator-(const chbTime& lhs, const chbTime& rhs);

std::ostream& operator<<(std::ostream& os, const chbTime& time);

class chbDateTime :public chbDate, public chbTime {
public:
	chbDateTime() = default;
	chbDateTime(int year, int month, int day, int hour, int minute, int second) :chbDate(year, month, day), chbTime(hour, minute, second) {}
	chbDateTime(int datenum, int timenum) :chbDate(datenum), chbTime(timenum) {}
	chbDateTime(int datenum, int hour, int minute, int second) :chbDate(datenum), chbTime(hour, minute, second) {}
	chbDateTime(const chbDate& date, const chbTime& time) :chbDate(date.date_num()), chbTime(time.time_num()) {}

	const char* datetime_str(const char* date_delimiter = "", const char* time_delimiter = ":") const;

	chbDate date() const { return chbDate(date_num()); }
	chbTime time() const { return chbTime(time_num()); }

	chbDateTime& advance(int years, int months, int days, int hours, int minutes, int seconds);

	chbDateTime& operator+(int days);

	chbDateTime& operator-(int days);

	static chbDateTime get_system_time();
};

bool operator==(const chbDateTime& lhs, const chbDateTime& rhs);

bool operator!=(const chbDateTime& lhs, const chbDateTime& rhs);

bool operator>(const chbDateTime& lhs, const chbDateTime& rhs);

bool operator<(const chbDateTime& lhs, const chbDateTime& rhs);

bool operator>=(const chbDateTime& lhs, const chbDateTime& rhs);

bool operator<=(const chbDateTime& lhs, const chbDateTime& rhs);

std::pair<int, int> operator-(const chbDateTime& lhs, const chbDateTime& rhs);

