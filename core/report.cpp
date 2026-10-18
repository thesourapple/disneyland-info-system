// core/report.cpp
#include "report.h"
#include <fstream>
#include <iomanip>

Report buildReport(const CharacterRepository& characters, const PhotoSessionRepository& sessions,
                   const ReportFilter& filter) {
    Report report;
    std::vector<Character> chars = filter.costumer.empty()
                                       ? characters.items()
                                       : characters.filterByCostumer(filter.costumer);
    report.charactersMatched = static_cast<int>(chars.size());

    for (const Character& c : chars) {
        int steps = 0;
        std::vector<PhotoSession> list = sessions.findByKey(keyOf(c), steps);  // поиск в АВЛ
        report.treeSteps += steps;
        for (const PhotoSession& p : list) {
            if (!filter.visitor.empty() && p.visitor != filter.visitor) continue;
            if (filter.useDate && (p.date.day != filter.date.day || p.date.month != filter.date.month ||
                                   p.date.year != filter.date.year))
                continue;
            report.rows.push_back(ReportRow{c, p});
        }
    }
    return report;
}

void printReport(const Report& report, std::ostream& os) {
    os << "Имя | Костюмер | Франшиза | Посетитель | Дата | Стоимость\n";
    os << std::fixed << std::setprecision(2);
    for (const auto& r : report.rows) {
        os << r.character.name << " | " << r.character.costumer << " | " << r.character.franchise
           << " | " << r.session.visitor << " | " << dateToString(r.session.date) << " | "
           << r.session.cost << "\n";
    }
    os << "Строк: " << report.rows.size() << " (персонажей по фильтру: " << report.charactersMatched
       << ", шагов поиска в дереве: " << report.treeSteps << ")\n";
}

bool saveReport(const Report& report, const std::string& filename) {
    std::ofstream out(utf8Path(filename));
    if (!out.is_open()) return false;
    printReport(report, out);
    return true;
}
