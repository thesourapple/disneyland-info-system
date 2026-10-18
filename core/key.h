// core/key.h
// Составной (тройной) ключ обоих справочников: Имя персонажа + Франшиза + Костюмер.

#ifndef CORE_KEY_H
#define CORE_KEY_H

#include <string>

struct CompositeKey {
    std::string name;
    std::string franchise;
    std::string costumer;

    bool operator==(const CompositeKey& other) const {
        return name == other.name && franchise == other.franchise && costumer == other.costumer;
    }

    // Лексикографическое сравнение — нужно АВЛ-дереву для упорядочивания узлов.
    bool operator<(const CompositeKey& other) const {
        if (name != other.name) return name < other.name;
        if (franchise != other.franchise) return franchise < other.franchise;
        return costumer < other.costumer;
    }
};

// Ключ корректен, если все три поля непусты.
inline bool isValidKey(const CompositeKey& k) {
    return !k.name.empty() && !k.franchise.empty() && !k.costumer.empty();
}

// Сворачивает строку в число: сумма кодов символов (Unicode, строка в UTF-8
// декодируется посимвольно; для латиницы это коды ASCII, для кириллицы
// А=1040 ... я=1103). Результат неотрицателен.
long long foldStringToInt(const std::string& s);

// Первичная хеш-функция составного ключа (та же мультипликативная формула,
// что и в задании 2.3): сначала ключ сворачивается в число
// g = сумма кодов символов Имени, Франшизы и Костюмера, затем
// h1 = floor(capacity * frac(g * A)), где A — константа золотого сечения.
int primaryHashKey(const CompositeKey& key, int capacity);

#endif // CORE_KEY_H
