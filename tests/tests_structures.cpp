// tests/tests_structures.cpp
// Тестирование структур данных: дата и составной ключ, хеш-таблица, АВЛ-дерево.
// Структуры проверяются напрямую, без справочников и GUI.

#include "test_utils.h"
#include "../core/date.h"
#include "../core/key.h"
#include "../core/hash_table.h"
#include "../core/avl_tree.h"
#include <vector>
#include <string>

// ---------- вспомогательные ----------

static CompositeKey K(const std::string& name, const std::string& fr = "F", const std::string& c = "C") {
    return CompositeKey{name, fr, c};
}

// n разных ключей с одинаковым h1 == slot
static std::vector<CompositeKey> collidingKeys(const HashTable& t, int slot, int n) {
    std::vector<CompositeKey> keys;
    for (int i = 0; (int)keys.size() < n && i < 1000000; ++i) {
        CompositeKey k{"Персонаж" + std::to_string(i), "Франшиза"};
        if (t.primaryHash(k) == slot) keys.push_back(k);
    }
    return keys;
}

static AVLNode* ins(AVLNode* t, const std::string& name, int index) {
    bool inserted;
    return insertIndex(t, K(name), index, inserted);
}

// Проверочная (только в тестах) рекурсивная высота: сверяем с ней показатели bal,
// которые дерево поддерживает без вычисления высот.
static int realHeight(const AVLNode* t) {
    if (t == nullptr) return 0;
    int l = realHeight(t->left), r = realHeight(t->right);
    return 1 + (l > r ? l : r);
}

static bool isBalanced(const AVLNode* t) {
    if (t == nullptr) return true;
    int d = realHeight(t->right) - realHeight(t->left);
    return d >= -1 && d <= 1 && t->bal == d && isBalanced(t->left) && isBalanced(t->right);
}

static bool isOrdered(const AVLNode* t) {
    std::vector<CompositeKey> keys;
    inorderTraversal(t, [&](const CompositeKey& k, int) {
        if (keys.empty() || !(keys.back() == k)) keys.push_back(k);
    });
    for (size_t i = 1; i < keys.size(); ++i)
        if (!(keys[i - 1] < keys[i])) return false;
    return true;
}

static std::vector<int> chainOf(const AVLNode* n) {
    std::vector<int> v;
    for (const IndexNode* p = n ? n->indexes : nullptr; p; p = p->next) v.push_back(p->arrayIndex);
    return v;
}

// =====================================================================
static void test_date() {
    std::cout << "Тест: дата формата «DD мес YYYY»\n";
    CHECK(dateToString(Date{5, 6, 2024}) == "05 июн 2024", "форматирование 05 июн 2024");
    Date d{};
    CHECK(parseDate("05 июн 2024", d) && d.day == 5 && d.month == 6 && d.year == 2024, "разбор строки");
    CHECK(!parseDate("05 xyz 2024", d), "неизвестный месяц отклонён");
    CHECK(isValidDate(Date{29, 2, 2024}), "29 фев 2024 — корректна");
    CHECK(!isValidDate(Date{29, 2, 2023}), "29 фев 2023 — некорректна");
    CHECK(!isValidDate(Date{31, 4, 2024}), "31 апр — некорректна");
}

static void test_key() {
    std::cout << "Тест: составной ключ и свёртка\n";
    CompositeKey a{"Микки Маус", "Disney Classics", "Петров Ю."}, b{"Микки Маус", "Toontown", "Петров Ю."};
    CompositeKey c{"Микки Маус", "Disney Classics", "Иванов А."};
    CHECK(a == CompositeKey({"Микки Маус", "Disney Classics", "Петров Ю."}), "одинаковые ключи равны");
    CHECK(!(a == b) && a < b, "разная франшиза: ключи различны, сравнение по франшизе");
    CHECK(!(a == c) && c < a, "разный костюмер: ключи различны, сравнение по костюмеру");
    CHECK(CompositeKey({"А", "Я", "Я"}) < CompositeKey({"Б", "А", "А"}), "сначала сравнивается имя");
    CHECK(isValidKey(a) && !isValidKey(CompositeKey{"", "X", "Y"}) && !isValidKey(CompositeKey{"X", "", "Y"}) &&
          !isValidKey(CompositeKey{"X", "Y", ""}), "ключ с пустым полем некорректен");
    CHECK(foldStringToInt("Marvel") == 615 && foldStringToInt("Тор") == 3232 && foldStringToInt("Петров Ю.") == 7618,
          "свёртка = сумма кодов символов (латиница и кириллица)");
    CHECK(primaryHashKey(CompositeKey{"Тор", "Marvel", "Петров Ю."}, 20) == 15,
          "h1(Тор, Marvel, Петров Ю.) = floor(20 * frac(11465 * A)) = 15");
    // длинная кириллица считается корректно
    std::vector<int> used(17, 0);
    for (int i = 0; i < 200; ++i)
        used[primaryHashKey(CompositeKey{"Длинное имя персонажа " + std::to_string(i), "Франшиза", "Костюмер"}, 17)] = 1;
    int distinct = 0;
    for (int u : used) distinct += u;
    CHECK(distinct >= 15, "200 длинных ключей распределены по большинству из 17 слотов");
}

// =====================================================================
//                         Хеш-таблица
// =====================================================================

static void test_ht_empty() {
    std::cout << "Тест ХТ: конструктор, хеш-функции\n";
    HashTable t(17);
    CHECK(t.getCapacity() == 17 && t.getCount() == 0, "размер 17, записей 0");
    bool allEmpty = true;
    for (int i = 0; i < 17; ++i) if (t.slotAt(i).status != SLOT_EMPTY) allEmpty = false;
    CHECK(allEmpty, "все слоты имеют статус 0");
    bool inRange = true;
    for (int i = 0; i < 300; ++i) {
        int h = t.primaryHash(K("k" + std::to_string(i)));
        if (h < 0 || h >= 17) inRange = false;
    }
    CHECK(inRange, "h1 в диапазоне [0, 17)");
    CHECK(t.secondaryHash(K("x")) == 1, "h2 = 1");
}

static void test_ht_insert_search() {
    std::cout << "Тест ХТ: вставка, поиск, значение — индекс в массиве\n";
    HashTable t(17);
    bool dup, ovf;
    CHECK(t.insert(K("Микки"), 4, dup, ovf), "вставка (Микки, F) -> 4");
    int steps;
    CHECK(t.search(K("Микки"), steps) == 4 && steps == 1, "поиск вернул 4 за 1 шаг");
    CHECK(t.search(K("Микки", "Другая"), steps) == -1, "та же фамилия, другая франшиза — не найдено");
    CHECK(!t.insert(K("Микки"), 5, dup, ovf) && dup, "повторный ключ отклонён");
}

static void test_ht_collisions() {
    std::cout << "Тест ХТ: три ключа с одинаковым h1 — линейное пробирование\n";
    HashTable t(17);
    auto k = collidingKeys(t, 16, 3);
    bool dup, ovf;
    for (int i = 0; i < 3; ++i) t.insert(k[i], i, dup, ovf);
    CHECK(t.slotAt(16).key == k[0], "1-й ключ — слот 16 (h1)");
    CHECK(t.slotAt(0).key == k[1], "2-й ключ — слот 0 (переход через конец)");
    CHECK(t.slotAt(1).key == k[2], "3-й ключ — слот 1");
    int steps;
    t.search(k[2], steps);
    CHECK(steps == 3, "поиск 3-го ключа — 3 шага");
}

static void test_ht_delete() {
    std::cout << "Тест ХТ: удаление — статус 2, цепочка не обрывается, слот переиспользуется\n";
    HashTable t(17);
    auto k = collidingKeys(t, 5, 4);
    bool dup, ovf;
    for (int i = 0; i < 3; ++i) t.insert(k[i], i, dup, ovf);
    CHECK(t.remove(k[1]) && t.slotAt(6).status == SLOT_DELETED, "средний удалён, слот 6 — статус 2");
    int steps;
    CHECK(t.search(k[2], steps) == 2 && steps == 3, "3-й ключ найден через слот со статусом 2");
    CHECK(!t.remove(k[1]), "повторное удаление — false");
    CHECK(t.insert(k[3], 9, dup, ovf) && t.slotAt(6).key == k[3], "новый ключ занял слот 6");
}

static void test_ht_overflow() {
    std::cout << "Тест ХТ: переполнение статической таблицы\n";
    HashTable t(3);
    bool dup, ovf;
    for (int i = 0; i < 3; ++i) t.insert(K("k" + std::to_string(i)), i, dup, ovf);
    CHECK(!t.insert(K("k3"), 3, dup, ovf) && ovf, "4-я вставка в таблицу размера 3 — overflow");
}

// =====================================================================
//                         АВЛ-дерево
// =====================================================================
// Ключи-имена "A".."G" упорядочены как буквы.

static void test_avl_rotations_insert() {
    std::cout << "Тест АВЛ: повороты при вставке (RR, LL, LR, RL)\n";
    const char* cases[4][3] = {{"A", "B", "C"}, {"C", "B", "A"}, {"C", "A", "B"}, {"A", "C", "B"}};
    const char* names[4] = {"RR", "LL", "LR", "RL"};
    for (int c = 0; c < 4; ++c) {
        AVLNode* t = createEmptyTree();
        for (int i = 0; i < 3; ++i) t = ins(t, cases[c][i], i);
        CHECK(t->key.name == "B" && t->left->key.name == "A" && t->right->key.name == "C",
              std::string(names[c]) + ": корень B, слева A, справа C");
        freeTree(t);
    }
}

static void test_avl_chain() {
    std::cout << "Тест АВЛ: одинаковый ключ — цепочка индексов\n";
    AVLNode* t = createEmptyTree();
    t = ins(t, "A", 0); t = ins(t, "B", 1); t = ins(t, "A", 2); t = ins(t, "A", 5);
    CHECK(countNodes(t) == 2 && countIndexes(t) == 4, "2 узла, 4 индекса");
    CHECK((chainOf(searchByKey(t, K("A"))) == std::vector<int>{0, 2, 5}), "цепочка A: 0 -> 2 -> 5");
    bool removed;
    t = removeIndex(t, K("A"), 2, removed);
    CHECK(removed && (chainOf(searchByKey(t, K("A"))) == std::vector<int>{0, 5}), "удалён индекс 2: 0 -> 5");
    t = removeIndex(t, K("A"), 7, removed);
    CHECK(!removed, "индекса 7 в цепочке нет — false");
    t = removeIndex(t, K("A"), 0, removed);
    t = removeIndex(t, K("A"), 5, removed);
    CHECK(searchByKey(t, K("A")) == nullptr && countNodes(t) == 1, "цепочка пуста — узел A удалён");
    freeTree(t);
}

static void test_avl_search_steps() {
    std::cout << "Тест АВЛ: поиск и число шагов\n";
    AVLNode* t = createEmptyTree();
    for (const char* n : {"D", "B", "F", "A", "C", "E", "G"}) t = ins(t, n, n[0] - 'A');
    int steps;
    CHECK(searchByKey(t, K("D"), steps) != nullptr && steps == 1, "корень D — 1 шаг");
    CHECK(searchByKey(t, K("E"), steps) != nullptr && steps == 3, "лист E — 3 шага");
    CHECK(searchByKey(t, K("D", "Другая"), steps) == nullptr, "D с другой франшизой — не найден");
    freeTree(t);
}

static void test_avl_delete_cases() {
    std::cout << "Тест АВЛ: удаление листа, узла с одним и с двумя потомками\n";
    bool removed;
    AVLNode* t = createEmptyTree();
    for (const char* n : {"D", "B", "F", "A", "C", "E", "G"}) t = ins(t, n, n[0] - 'A');

    t = removeIndex(t, K("A"), 0, removed);
    CHECK(removed && t->left->left == nullptr, "лист A удалён");

    t = removeIndex(t, K("C"), 2, removed);   // теперь B — лист
    t = ins(t, "BB", 9);                      // у B один (правый) потомок BB
    t = removeIndex(t, K("B"), 1, removed);
    CHECK(removed && t->left->key.name == "BB", "узел B с одним потомком заменён потомком BB");

    t = removeIndex(t, K("D"), 3, removed);
    CHECK(removed && t->key.name == "E", "корень D (два потомка) заменён минимумом справа — E");
    CHECK(chainOf(t) == std::vector<int>{4}, "вместе с ключом E перенесена его цепочка (4)");
    CHECK(isBalanced(t) && isOrdered(t), "дерево сбалансировано и упорядочено");
    freeTree(t);
}

static void test_avl_delete_rotation() {
    std::cout << "Тест АВЛ: удаление вызывает повороты\n";
    bool removed;
    AVLNode* t = createEmptyTree();
    for (const char* n : {"B", "A", "C", "D"}) t = ins(t, n, 0);
    t = removeIndex(t, K("A"), 0, removed);
    CHECK(t->key.name == "C" && isBalanced(t), "RR после удаления: корень C");
    freeTree(t);

    t = createEmptyTree();
    for (const char* n : {"C", "A", "D", "B"}) t = ins(t, n, 0);
    t = removeIndex(t, K("D"), 0, removed);
    CHECK(t->key.name == "B" && isBalanced(t), "LR после удаления: корень B");
    freeTree(t);
}

static void test_avl_big() {
    std::cout << "Тест АВЛ: 2000 вставок и удаление половины — баланс сохраняется\n";
    AVLNode* t = createEmptyTree();
    for (int i = 0; i < 2000; ++i) t = ins(t, "K" + std::to_string(10000 + i), i);
    bool removed, allRemoved = true;
    for (int i = 0; i < 2000; i += 2) {
        t = removeIndex(t, K("K" + std::to_string(10000 + i)), i, removed);
        if (!removed) allRemoved = false;
    }
    CHECK(allRemoved && countNodes(t) == 1000, "удалено 1000, осталось 1000 узлов");
    CHECK(isBalanced(t) && isOrdered(t) && nodeHeight(t) <= 15, "сбалансировано, высота <= 15");
    CHECK(nodeHeight(t) == realHeight(t), "высота по показателям bal совпадает с настоящей");
    freeTree(t);

    // случайные вставки и удаления: после каждой операции bal всех узлов верны
    t = createEmptyTree();
    unsigned seed = 12345;
    bool ok = true;
    std::vector<int> present(300, 0);
    for (int step = 0; step < 6000 && ok; ++step) {
        seed = seed * 1103515245u + 12345u;
        int k = (seed >> 8) % 300;
        if (present[k]) { t = removeIndex(t, K("R" + std::to_string(1000 + k)), k, removed); present[k] = 0; }
        else            { t = ins(t, "R" + std::to_string(1000 + k), k); present[k] = 1; }
        if (step % 50 == 0) ok = isBalanced(t) && isOrdered(t) && nodeHeight(t) == realHeight(t);
    }
    CHECK(ok && isBalanced(t), "6000 случайных вставок/удалений: показатели bal всегда верны");
    freeTree(t);
}

int main() {
    std::cout << "##### Структуры данных #####\n";
    test_date();
    test_key();
    test_ht_empty();
    test_ht_insert_search();
    test_ht_collisions();
    test_ht_delete();
    test_ht_overflow();
    test_avl_rotations_insert();
    test_avl_chain();
    test_avl_search_steps();
    test_avl_delete_cases();
    test_avl_delete_rotation();
    test_avl_big();
    return finishTests();
}
