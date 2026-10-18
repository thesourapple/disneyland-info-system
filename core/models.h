// core/models.h
// Модели данных предметной области «Диснейленд».
// Справочник_1: Персонажи (Имя, Костюмер, Франшиза). Ключ — Имя+Франшиза+Костюмер, уникален.
// Справочник_2: Фотосессии (Имя персонажа, Франшиза, Костюмер, Посетитель, Дата, Стоимость).
//               Ключ — Имя персонажа+Франшиза+Костюмер, НЕ уникален.

#ifndef CORE_MODELS_H
#define CORE_MODELS_H

#include <string>
#include "key.h"
#include "date.h"

// Текстовое поле: не пустое, без пробелов в начале и в конце, без символа ';'
// (разделитель полей в файле), длина не больше MAX_TEXT_LEN символов.
const int MAX_TEXT_LEN = 50;

inline int utf8Length(const std::string& s) {
    int n = 0;
    for (unsigned char ch : s) if ((ch & 0xC0) != 0x80) ++n;   // не считаем байты-продолжения
    return n;
}

inline bool isValidText(const std::string& s) {
    if (s.empty() || s.front() == ' ' || s.back() == ' ') return false;
    if (s.find(';') != std::string::npos) return false;
    return utf8Length(s) <= MAX_TEXT_LEN;
}

// Стоимость фотосессии: больше 0 и не больше MAX_COST.
const double MAX_COST = 1000000.0;

// ---------- Справочник_1: Персонажи ----------

struct Character {
    std::string name;
    std::string costumer;
    std::string franchise;
};

inline CompositeKey keyOf(const Character& c) {
    return CompositeKey{c.name, c.franchise, c.costumer};
}

// Проверка корректности записи — все поля корректные текстовые строки.
inline bool isValidCharacter(const Character& c) {
    return isValidText(c.name) && isValidText(c.costumer) && isValidText(c.franchise);
}

// Полное совпадение всех полей — используется при удалении.
inline bool recordEquals(const Character& a, const Character& b) {
    return a.name == b.name && a.costumer == b.costumer && a.franchise == b.franchise;
}

// ---------- Справочник_2: Фотосессии ----------

struct PhotoSession {
    std::string characterName;
    std::string franchise;
    std::string costumer;
    std::string visitor;
    Date date;
    double cost;
};

inline CompositeKey keyOf(const PhotoSession& p) {
    return CompositeKey{p.characterName, p.franchise, p.costumer};
}

// Проверка корректности записи: текстовые поля корректны, дата существует, стоимость в (0; MAX_COST].
inline bool isValidPhotoSession(const PhotoSession& p) {
    return isValidText(p.characterName) && isValidText(p.franchise) && isValidText(p.costumer) &&
           isValidText(p.visitor) &&
           isValidDate(p.date) && p.cost > 0.0 && p.cost <= MAX_COST;
}

// Полное совпадение всех полей — используется при удалении.
inline bool recordEquals(const PhotoSession& a, const PhotoSession& b) {
    return a.characterName == b.characterName && a.franchise == b.franchise &&
           a.costumer == b.costumer && a.visitor == b.visitor &&
           a.date.day == b.date.day && a.date.month == b.date.month && a.date.year == b.date.year &&
           a.cost == b.cost;
}

#endif // CORE_MODELS_H
