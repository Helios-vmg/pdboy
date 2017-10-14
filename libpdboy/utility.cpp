#include "utility.h"

DateTime DateTime::from_posix(posix_time_t posix){
	static const char days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	DateTime ret;
	ret.year = 1970;
	while (1){
		bool leap = !(ret.year % 4) & !!(ret.year % 100) | !(ret.year % 400);
		unsigned seconds_in_year = (365 + leap) * 86400;
		if (posix < seconds_in_year)
			break;
		posix -= seconds_in_year;
		ret.year++;
	}
	ret.month = 0;
	while (1){
		bool leap = !(ret.year % 4) & !!(ret.year % 100) | !(ret.year % 400);
		bool leap_february = leap & (ret.month == 2);
		unsigned seconds_in_month = (days[ret.month] + leap_february) * 86400;
		if (posix < seconds_in_month)
			break;
		posix -= seconds_in_month;
		ret.month++;
	}
	ret.month++;
	ret.day = (std::uint8_t)(posix / 86400 + 1);
	posix %= 86400;
	ret.hour = (std::uint8_t)(posix / 3600);
	posix %= 3600;
	ret.minute = (std::uint8_t)(posix / 60);
	posix %= 60;
	ret.second = (std::uint8_t)posix;
	return ret;
}

const char months_accumulated[] = { 0, 3, 3, 6, 8, 11, 13, 16, 19, 21, 24, 26 };

posix_time_t DateTime::to_posix() const{
	bool is_leap_year = !(this->year % 4) & !!(this->year % 100) | !(this->year % 400);
	auto year = this->year - 1900;
	auto month = this->month - 1;
	unsigned days = 28 * month + months_accumulated[month] + (is_leap_year & (month > 1)) + this->day - 1;

	days += (year + 299) / 400;
	days -= (year - 1) / 100;
	days += (year - 69) / 4;
	days += (year - 70) * 365;

	posix_time_t ret = days;
	ret *= 86400;
	ret += this->hour * 3600 + this->minute * 60 + this->second;

	return ret;
}

int days_from_date(const DateTime &date){
	int year = date.year;
	int month = date.month;
	int day = date.day;
	bool is_leap_year = !(year % 4) & !!(year % 100) | !(year % 400);
	year--;
	month--;
	int day_of_year = month * 28 + months_accumulated[month] + (is_leap_year & (month > 1)) + day;
	return year * 365 + year / 4 - year / 100 + year / 400 + day_of_year - 693594;
}

double DateTime::to_double_timestamp(){
	return days_from_date(*this) + this->hour * (1.0 / 24.0) + this->minute * (1.0 / (24 * 60)) + this->second * (1.0 / (24 * 60 * 60));
}

posix_time_t DateTime::double_timestamp_to_posix(double t){
	return (posix_time_t)((t - 25569) * 86400);
}
