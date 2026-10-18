// core/report.h
// Отчёт: выборка из двух справочников по полям фильтра
//   Справочник_1 — Костюмер; Справочник_2 — Посетитель и Дата.
// Пустой фильтр (или useDate = false) означает «любое значение».
// Связь справочников — по ключу Имя персонажа + Франшиза + Костюмер: для каждого
// подходящего персонажа его фотосессии ищутся через АВЛ-дерево (не перебором).
// В строке отчёта — все поля обоих справочников.

#ifndef CORE_REPORT_H
#define CORE_REPORT_H

#include <vector>
#include <string>
#include <iostream>
#include "character_repository.h"
#include "photo_session_repository.h"

struct ReportFilter {
    std::string costumer;   // Справочник_1
    std::string visitor;    // Справочник_2
    bool useDate = false;   // Справочник_2
    Date date{};
};

struct ReportRow {
    Character character;
    PhotoSession session;
};

struct Report {
    std::vector<ReportRow> rows;
    int charactersMatched = 0;  // персонажей, прошедших фильтр по костюмеру
    int treeSteps = 0;          // суммарно шагов поиска в АВЛ-дереве
};

Report buildReport(const CharacterRepository& characters, const PhotoSessionRepository& sessions,
                   const ReportFilter& filter);

void printReport(const Report& report, std::ostream& os = std::cout);

bool saveReport(const Report& report, const std::string& filename);

#endif // CORE_REPORT_H
