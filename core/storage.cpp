// core/storage.cpp
#include "storage.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <locale>

namespace {

std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// Убирает UTF-8 BOM в начале строки (Блокнот Windows добавляет его в начало файла).
void stripBom(std::string& s) {
    if (s.size() >= 3 && (unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB &&
        (unsigned char)s[2] == 0xBF)
        s.erase(0, 3);
}

// Число с точкой или запятой, не зависит от языковых настроек системы.
bool parseCost(std::string text, double& out) {
    for (char& ch : text) if (ch == ',') ch = '.';
    std::istringstream iss(text);
    iss.imbue(std::locale::classic());
    if (!(iss >> out)) return false;
    char rest;
    return !(iss >> rest);   // после числа ничего быть не должно
}

std::vector<std::string> splitBySemicolon(const std::string& line) {
    std::vector<std::string> parts;
    std::stringstream ss(line);
    std::string part;
    while (std::getline(ss, part, ';')) {
        parts.push_back(trim(part));
    }
    return parts;
}

bool parseCharacterLine(const std::string& line, Character& c) {
    auto parts = splitBySemicolon(line);
    if (parts.size() != 3) return false;
    c = Character{parts[0], parts[1], parts[2]};
    return true;
}

bool parsePhotoSessionLine(const std::string& line, PhotoSession& p) {
    auto parts = splitBySemicolon(line);
    if (parts.size() != 6) return false;

    Date date{};
    if (!parseDate(parts[4], date)) return false;

    double cost;
    if (!parseCost(parts[5], cost)) return false;
    p = PhotoSession{parts[0], parts[1], parts[2], parts[3], date, cost};
    return true;
}

} // namespace

// ================= Чтение в массивы =================

LoadStats readCharacters(std::istream& in, std::vector<Character>& out) {
    LoadStats stats;
    std::string line;
    bool first = true;
    while (std::getline(in, line)) {
        if (first) { stripBom(line); first = false; }
        if (trim(line).empty()) continue;
        ++stats.totalLines;

        Character c;
        if (!parseCharacterLine(line, c)) { ++stats.badFormat; continue; }
        if (!isValidCharacter(c)) { ++stats.invalidData; continue; }
        out.push_back(c);
        ++stats.loaded;
    }
    return stats;
}

LoadStats readPhotoSessions(std::istream& in, std::vector<PhotoSession>& out) {
    LoadStats stats;
    std::string line;
    bool first = true;
    while (std::getline(in, line)) {
        if (first) { stripBom(line); first = false; }
        if (trim(line).empty()) continue;
        ++stats.totalLines;

        PhotoSession p;
        if (!parsePhotoSessionLine(line, p)) { ++stats.badFormat; continue; }
        if (!isValidPhotoSession(p)) { ++stats.invalidData; continue; }
        out.push_back(p);
        ++stats.loaded;
    }
    return stats;
}

LoadStats readCharactersFile(const std::string& filename, std::vector<Character>& out) {
    std::ifstream in(utf8Path(filename));
    if (!in.is_open()) { LoadStats s; s.fileOpened = false; return s; }
    return readCharacters(in, out);
}

LoadStats readPhotoSessionsFile(const std::string& filename, std::vector<PhotoSession>& out) {
    std::ifstream in(utf8Path(filename));
    if (!in.is_open()) { LoadStats s; s.fileOpened = false; return s; }
    return readPhotoSessions(in, out);
}

// ================= Запись массивов =================

void writeCharacters(std::ostream& out, const std::vector<Character>& items) {
    for (const auto& c : items)
        out << c.name << ";" << c.costumer << ";" << c.franchise << "\n";
}

void writePhotoSessions(std::ostream& out, const std::vector<PhotoSession>& items) {
    out << std::fixed << std::setprecision(2);
    for (const auto& p : items)
        out << p.characterName << ";" << p.franchise << ";" << p.costumer << ";" << p.visitor << ";"
            << dateToString(p.date) << ";" << p.cost << "\n";
}

bool writeCharactersFile(const std::string& filename, const std::vector<Character>& items) {
    std::ofstream out(utf8Path(filename));
    if (!out.is_open()) return false;
    writeCharacters(out, items);
    return true;
}

bool writePhotoSessionsFile(const std::string& filename, const std::vector<PhotoSession>& items) {
    std::ofstream out(utf8Path(filename));
    if (!out.is_open()) return false;
    writePhotoSessions(out, items);
    return true;
}
