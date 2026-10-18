// core/date.h
// Дата хранится как структура из трёх целых чисел (день, месяц, год) —
// число, а не строка. Формат отображения по заданию: "DD мес YYYY",
// где мес — русское сокращение месяца (янв, фев, ... дек).

#ifndef CORE_DATE_H
#define CORE_DATE_H

#include <string>

struct Date {
    int day;
    int month; // 1..12
    int year;
};

// Проверка календарной корректности (длина месяца, високосный год).
bool isValidDate(const Date& d);

// "DD мес YYYY", например "05 июн 2024".
std::string dateToString(const Date& d);

// Разбор строки вида "DD мес YYYY" (мес — ровно одно из: янв фев мар апр май
// июн июл авг сен окт ноя дек, строчными буквами). Возвращает false, если
// формат не распознан (при этом ничего не гарантируется про календарную
// корректность результата — это отдельно проверяет isValidDate).
bool parseDate(const std::string& text, Date& out);

#endif // CORE_DATE_H
