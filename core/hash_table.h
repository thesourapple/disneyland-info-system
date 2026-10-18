// core/hash_table.h
// Статическая хеш-таблица для Справочника_1 (Персонажи).
// Ключ — CompositeKey (Имя + Франшиза + Костюмер), уникален. Значение — индекс записи
// в массиве справочника. Сами записи в таблице НЕ хранятся.
// Открытая адресация, линейное пробирование с шагом k = 1 (как в задании 2.3).
// Статус слота: 0 — свободен, 1 — занят, 2 — удалён.

#ifndef CORE_HASH_TABLE_H
#define CORE_HASH_TABLE_H

#include <vector>
#include <iostream>
#include "key.h"

enum SlotStatus { SLOT_EMPTY = 0, SLOT_OCCUPIED = 1, SLOT_DELETED = 2 };

struct HashSlot {
    int status;        // 0 / 1 / 2
    CompositeKey key;  // актуально при status == 1
    int arrayIndex;    // индекс записи в массиве (при status == 1)
};

class HashTable {
public:
    explicit HashTable(int capacity);
    ~HashTable();

    // Первичная хеш-функция (свёртка ключа + мультипликативный метод, см. key.h).
    int primaryHash(const CompositeKey& key) const;

    // Вторичная хеш-функция — шаг пробирования, для линейного пробирования 1.
    int secondaryHash(const CompositeKey& key) const;

    // Добавление пары (ключ, индекс). duplicateKey — ключ уже есть; overflow — нет места.
    bool insert(const CompositeKey& key, int arrayIndex, bool& duplicateKey, bool& overflow);

    // Удаление по ключу: слот получает статус 2.
    bool remove(const CompositeKey& key);

    // Поиск: индекс в массиве или -1; steps — число просмотренных слотов.
    int search(const CompositeKey& key, int& steps) const;

    // Печать для окна отладки: слот, статус, ключ, h1, h2, индекс в массиве.
    void print(std::ostream& os = std::cout) const;

    int getCapacity() const { return capacity_; }
    int getCount() const { return count_; }
    const HashSlot& slotAt(int index) const { return table_[index]; }

private:
    std::vector<HashSlot> table_;
    int capacity_;
    int count_;

    int probe(const CompositeKey& key, int attempt) const;
    int findSlot(const CompositeKey& key, int& steps) const;
};

#endif // CORE_HASH_TABLE_H
