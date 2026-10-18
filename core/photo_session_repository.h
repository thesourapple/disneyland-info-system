// core/photo_session_repository.h
// Справочник_2 (Фотосессии): массив записей + АВЛ-дерево как его отображение.
//
// Записи хранятся ТОЛЬКО в массиве. Дерево хранит «ключ (Имя персонажа+Франшиза+Костюмер)
// -> цепочка индексов в массиве» (ключ не уникален). Изменять дерево напрямую
// нельзя: добавление и удаление идут через массив (add/remove).
// Удаление не сдвигает элементы массива: ячейка помечается active = false.

#ifndef CORE_PHOTO_SESSION_REPOSITORY_H
#define CORE_PHOTO_SESSION_REPOSITORY_H

#include <vector>
#include <functional>
#include <iostream>
#include "avl_tree.h"
#include "storage.h"

struct PhotoSessionSlot {
    bool active;
    PhotoSession data;
};

class PhotoSessionRepository {
public:
    PhotoSessionRepository();
    ~PhotoSessionRepository();

    PhotoSessionRepository(const PhotoSessionRepository&) = delete;
    PhotoSessionRepository& operator=(const PhotoSessionRepository&) = delete;

    // Загрузка: массив и дерево очищаются и заполняются заново.
    // keyExists (если задан) — проверка ссылочной целостности: записи, для
    // которых он вернул false, не загружаются (stats.unknownCharacter).
    LoadStats loadFromFile(const std::string& filename,
                           const std::function<bool(const CompositeKey&)>& keyExists = nullptr);
    LoadStats loadFromStream(std::istream& in,
                             const std::function<bool(const CompositeKey&)>& keyExists = nullptr);
    bool saveToFile(const std::string& filename) const;
    void saveToStream(std::ostream& out) const;

    // Добавление: проверка полей, запись в конец массива, пара (ключ, индекс) в дерево.
    bool add(const PhotoSession& p, bool& validData);

    // Удаление по полному совпадению всех полей (кандидаты — цепочка узла дерева).
    bool remove(const PhotoSession& p);

    // Поиск по ключу через дерево: все фотосессии персонажа; steps — просмотрено узлов.
    std::vector<PhotoSession> findByKey(const CompositeKey& key, int& steps) const;
    bool hasKey(const CompositeKey& key) const;

    // Фильтры по Посетителю и по Дате — просмотр массива.
    std::vector<PhotoSession> filterByVisitor(const std::string& visitor) const;
    std::vector<PhotoSession> filterByDate(const Date& date) const;

    std::vector<PhotoSession> items() const;
    int size() const { return static_cast<int>(items_.size()); }
    int activeCount() const;
    bool isActive(int index) const { return items_[index].active; }
    const PhotoSession& at(int index) const { return items_[index].data; }

    const AVLNode* tree() const { return root_; }   // только для чтения
    void printArray(std::ostream& os) const;

private:
    std::vector<PhotoSessionSlot> items_;
    AVLNode* root_;
};

#endif // CORE_PHOTO_SESSION_REPOSITORY_H
