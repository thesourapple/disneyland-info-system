// core/character_repository.h
// Справочник_1 (Персонажи): массив записей + хеш-таблица как его отображение.
//
// Записи хранятся ТОЛЬКО в массиве. Хеш-таблица хранит пары «ключ (Имя+Франшиза+Костюмер)
// -> индекс в массиве». Изменять таблицу напрямую нельзя: добавление и удаление
// идут через массив (add/remove), таблица обновляется внутри этих методов.
// Удаление не сдвигает элементы массива: ячейка помечается active = false.

#ifndef CORE_CHARACTER_REPOSITORY_H
#define CORE_CHARACTER_REPOSITORY_H

#include <vector>
#include <memory>
#include <iostream>
#include "hash_table.h"
#include "storage.h"

struct CharacterSlot {
    bool active;     // false — запись удалена
    Character data;
};

class CharacterRepository {
public:
    explicit CharacterRepository(int hashTableCapacity);

    CharacterRepository(const CharacterRepository&) = delete;
    CharacterRepository& operator=(const CharacterRepository&) = delete;

    // Загрузка: массив и таблица (той же ёмкости) очищаются и заполняются заново.
    LoadStats loadFromFile(const std::string& filename);
    LoadStats loadFromStream(std::istream& in);
    bool saveToFile(const std::string& filename) const;   // только активные записи
    void saveToStream(std::ostream& out) const;

    // Добавление: проверка полей, уникальности ключа и места в таблице,
    // затем запись в конец массива и пара (ключ, индекс) в таблицу.
    bool add(const Character& c, bool& validData, bool& duplicateKey, bool& overflow);

    // Удаление по полному совпадению всех полей (индекс ищется через таблицу).
    bool remove(const Character& c);

    // Поиск по ключу через хеш-таблицу.
    bool findByKey(const CompositeKey& key, Character& out, int& steps) const;
    bool contains(const CompositeKey& key) const;

    // Фильтр по Костюмеру — просмотр массива (поле не является ключом).
    std::vector<Character> filterByCostumer(const std::string& costumer) const;

    // Пересоздать таблицу с новым размером и заново занести в неё все активные
    // записи массива. Если размер меньше числа записей — отказ (false), таблица не меняется.
    bool setCapacity(int newCapacity);

    std::vector<Character> items() const;   // активные записи в порядке массива
    int size() const { return static_cast<int>(items_.size()); }   // вместе с удалёнными
    int activeCount() const;
    bool isActive(int index) const { return items_[index].active; }
    const Character& at(int index) const { return items_[index].data; }

    const HashTable& table() const { return *table_; }   // только для чтения
    void printArray(std::ostream& os) const;

private:
    std::vector<CharacterSlot> items_;
    std::unique_ptr<HashTable> table_;
};

#endif // CORE_CHARACTER_REPOSITORY_H
