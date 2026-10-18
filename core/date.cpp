// core/date.cpp
#include "date.h"
#include <sstream>
#include <cstdio>
#include <array>

namespace {
const std::array<std::string, 12> MONTHS = {
    "янв", "фев", "мар", "апр", "май", "июн",
    "июл", "авг", "сен", "окт", "ноя", "дек"
};
}

bool isValidDate(const Date& d) {
    if (d.year < 1 || d.year > 9999) return false;
    if (d.month < 1 || d.month > 12) return false;
    if (d.day < 1) return false;

    static const int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int maxDay = daysInMonth[d.month - 1];
    bool leap = (d.year % 4 == 0 && d.year % 100 != 0) || (d.year % 400 == 0);
    if (d.month == 2 && leap) maxDay = 29;

    return d.day <= maxDay;
}

std::string dateToString(const Date& d) {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%02d", d.day);
    std::string monthStr = (d.month >= 1 && d.month <= 12) ? MONTHS[d.month - 1] : "???";
    char yearBuf[8];
    std::snprintf(yearBuf, sizeof(yearBuf), "%04d", d.year);
    return std::string(buf) + " " + monthStr + " " + yearBuf;
}

bool parseDate(const std::string& text, Date& out) {
    std::istringstream iss(text);
    std::string dayStr, monthStr, yearStr;
    if (!(iss >> dayStr >> monthStr >> yearStr)) return false;

    int monthIndex = -1;
    for (size_t i = 0; i < MONTHS.size(); ++i) {
        if (MONTHS[i] == monthStr) { monthIndex = static_cast<int>(i); break; }
    }
    if (monthIndex == -1) return false;

    try {
        out.day = std::stoi(dayStr);
        out.year = std::stoi(yearStr);
        out.month = monthIndex + 1;
    } catch (...) {
        return false;
    }
    return true;
}
