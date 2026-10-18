// tests/tests_functional.cpp
// Тестирование функционала справочников и отчёта (через массивы + СД):
// загрузка/сохранение, добавление/удаление только через массив, синхронность СД,
// стабильность индексов, поиск, фильтры, размер ХТ, ссылочная целостность, отчёт.

#include "test_utils.h"
#include "../core/app_state.h"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <string>
#include <vector>

static Character C(const std::string& n, const std::string& cos, const std::string& f) {
    return Character{n, cos, f};
}
static PhotoSession P(const std::string& n, const std::string& f, const std::string& c, const std::string& v,
                      int d, int m, int y, double cost) {
    return PhotoSession{n, f, c, v, Date{d, m, y}, cost};
}

static std::string tmpFile(const std::string& name, const std::string& content) {
    std::string path = "test_tmp_" + name;
    std::ofstream out(path);
    out << content;
    return path;
}

// каждый занятый слот ХТ указывает на активную запись с тем же ключом, и их поровну
static bool tableMatchesArray(const CharacterRepository& r) {
    int occupied = 0;
    for (int i = 0; i < r.table().getCapacity(); ++i) {
        const HashSlot& s = r.table().slotAt(i);
        if (s.status != SLOT_OCCUPIED) continue;
        ++occupied;
        if (!r.isActive(s.arrayIndex) || !(keyOf(r.at(s.arrayIndex)) == s.key)) return false;
    }
    return occupied == r.activeCount();
}

static bool treeMatchesArray(const PhotoSessionRepository& r) {
    int n = 0;
    bool ok = true;
    inorderTraversal(r.tree(), [&](const CompositeKey& k, int idx) {
        ++n;
        if (!r.isActive(idx) || !(keyOf(r.at(idx)) == k)) ok = false;
    });
    return ok && n == r.activeCount();
}

// =====================================================================
//                     Справочник_1: Персонажи
// =====================================================================

static void test_char_add_find() {
    std::cout << "Тест: Персонажи — добавление и поиск по ключу\n";
    CharacterRepository r(17);
    bool v, d, o;
    CHECK(r.add(C("Микки", "Иванов", "Classics"), v, d, o), "добавлен Микки");
    CHECK(r.add(C("Микки", "Петров", "Toontown"), v, d, o), "тот же персонаж другой франшизы — другой ключ");
    CHECK(!r.add(C("Микки", "Иванов", "Classics"), v, d, o) && d, "повтор ключа (Микки, Classics, Иванов) отклонён");
    CHECK(r.add(C("Микки", "Сидоров", "Classics"), v, d, o), "другой костюмер — другой ключ, добавлен");
    CHECK(!r.add(C("Гуфи", "", "Classics"), v, d, o) && !v, "пустой костюмер — нарушение целостности");
    CHECK(!r.add(C("Гуфи;Плуто", "Иванов", "Classics"), v, d, o) && !v, "символ ';' в поле — отклонено");
    CHECK(!r.add(C(" Гуфи", "Иванов", "Classics"), v, d, o) && !v, "пробел в начале поля — отклонено");
    CHECK(!r.add(C(std::string(51 * 2, 'a').substr(0, 51), "Иванов", "Classics"), v, d, o) && !v,
          "имя длиннее 50 символов — отклонено");
    CHECK(r.add(C("Чёрная пантера из Ваканды и Ко", "Иванов", "Marvel"), v, d, o),
          "кириллица 30 символов (60 байт) — принято: длина в символах, а не в байтах");
    Character out;
    int steps;
    CHECK(r.findByKey({"Микки", "Toontown", "Петров"}, out, steps) && out.costumer == "Петров",
          "найден (Микки, Toontown, Петров)");
    CHECK(!r.findByKey({"Микки", "Toontown", "Иванов"}, out, steps), "(Микки, Toontown, Иванов) не найден");
    CHECK(r.size() == 4 && tableMatchesArray(r), "в массиве 4 записи, ХТ — их отображение");
}

static void test_char_remove() {
    std::cout << "Тест: Персонажи — удаление по полному совпадению, индексы не сдвигаются\n";
    CharacterRepository r(17);
    bool v, d, o;
    r.add(C("Белль", "А", "Принцессы"), v, d, o);
    r.add(C("Тор", "Б", "Marvel"), v, d, o);
    r.add(C("Гуфи", "В", "Classics"), v, d, o);
    CHECK(!r.remove(C("Тор", "Другой", "Marvel")), "другой костюмер — не удалено");
    CHECK(r.remove(C("Тор", "Б", "Marvel")), "Тор удалён");
    CHECK(!r.isActive(1) && r.size() == 3 && r.activeCount() == 2, "ячейка 1 помечена удалённой, без сдвига");
    Character out;
    int steps;
    CHECK(r.findByKey({"Гуфи", "Classics", "В"}, out, steps) && out.costumer == "В", "Гуфи по-прежнему находится");
    CHECK(r.table().search({"Гуфи", "Classics", "В"}, steps) == 2, "в ХТ у Гуфи прежний индекс 2");
    CHECK(tableMatchesArray(r), "ХТ соответствует массиву");
    CHECK(r.add(C("Тор", "Г", "Marvel"), v, d, o) && r.size() == 4, "Тор добавлен снова — в конец (индекс 3)");
}

static void test_char_filter_capacity() {
    std::cout << "Тест: Персонажи — фильтр по костюмеру, изменение размера ХТ\n";
    CharacterRepository r(5);
    bool v, d, o;
    r.add(C("A", "Иванов", "F"), v, d, o);
    r.add(C("B", "Петров", "F"), v, d, o);
    r.add(C("C", "Иванов", "F"), v, d, o);
    auto f = r.filterByCostumer("Иванов");
    CHECK(f.size() == 2 && f[0].name == "A" && f[1].name == "C", "по костюмеру Иванов — A и C");
    r.remove(C("A", "Иванов", "F"));
    CHECK(r.filterByCostumer("Иванов").size() == 1, "удалённая запись в фильтр не попадает");
    CHECK(!r.setCapacity(1), "размер 1 меньше числа записей — отказ");
    CHECK(r.table().getCapacity() == 5, "таблица не изменилась");
    CHECK(r.setCapacity(19) && r.table().getCapacity() == 19, "размер изменён на 19");
    CHECK(r.table().getCount() == 2 && tableMatchesArray(r), "в новой ХТ 2 записи, индексы прежние");
    r.add(C("D", "Х", "F"), v, d, o);
    r.add(C("E", "Х", "F"), v, d, o);
    CharacterRepository small(2);
    small.add(C("A", "Х", "F"), v, d, o);
    small.add(C("B", "Х", "F"), v, d, o);
    CHECK(!small.add(C("C", "Х", "F"), v, d, o) && o, "ХТ размера 2 переполнена на 3-й записи");
}

static void test_char_load_save() {
    std::cout << "Тест: Персонажи — загрузка и сохранение файла\n";
    std::string in = tmpFile("chars.txt",
        "Золушка;Новиков П.;Принцессы\n"
        "плохая строка\n"
        "Тор;;Marvel\n"
        "Золушка;Новиков П.;Принцессы\n"
        "\n"
        "Тор;Петров Ю.;Marvel\n");
    CharacterRepository r(11);
    LoadStats s = r.loadFromFile(in);
    CHECK(s.totalLines == 5 && s.loaded == 2, "строк 5, загружено 2");
    CHECK(s.badFormat == 1 && s.invalidData == 1 && s.duplicateKey == 1, "1 формат, 1 целостность, 1 дубликат");
    CHECK(r.items()[1].name == "Тор", "порядок записей — как в файле");
    r.remove(C("Золушка", "Новиков П.", "Принцессы"));
    CHECK(r.saveToFile("test_tmp_out.txt"), "сохранение выполнено");
    CharacterRepository r2(11);
    LoadStats s2 = r2.loadFromFile("test_tmp_out.txt");
    CHECK(s2.loaded == 1 && r2.items()[0].name == "Тор", "сохранены только активные записи");
    CHECK(!r2.loadFromFile("нет_такого_файла.txt").fileOpened, "несуществующий файл — fileOpened = false");
    CHECK(r2.activeCount() == 1, "при ошибке открытия данные не стёрты");
    std::remove(in.c_str());
    std::remove("test_tmp_out.txt");
}

// =====================================================================
//                     Справочник_2: Фотосессии
// =====================================================================

static void test_ps_add_find() {
    std::cout << "Тест: Фотосессии — добавление, неуникальный ключ, поиск\n";
    PhotoSessionRepository r;
    bool v;
    r.add(P("Тор", "Marvel", "Петров Ю.", "Иванова А.", 1, 1, 2024, 100), v);
    r.add(P("Белль", "Принцессы", "Сидоров М.", "Петров С.", 2, 2, 2024, 200), v);
    r.add(P("Тор", "Marvel", "Петров Ю.", "Сидоров К.", 3, 3, 2024, 300), v);
    CHECK(!r.add(P("Тор", "Marvel", "Петров Ю.", "Х", 30, 2, 2024, 1), v) && !v, "дата 30 фев — отклонена");
    CHECK(!r.add(P("Тор", "Marvel", "Петров Ю.", "Х", 1, 1, 2024, -5), v) && !v, "отрицательная стоимость — отклонена");
    CHECK(!r.add(P("Тор", "Marvel", "Петров Ю.", "Х", 1, 1, 2024, 0), v) && !v, "нулевая стоимость — отклонена");
    CHECK(!r.add(P("Тор", "Marvel", "Петров Ю.", "Х", 1, 1, 2024, 1000000.01), v) && !v, "стоимость больше 1 000 000 — отклонена");
    int steps;
    auto list = r.findByKey({"Тор", "Marvel", "Петров Ю."}, steps);
    CHECK(list.size() == 2 && list[0].visitor == "Иванова А." && list[1].visitor == "Сидоров К.",
          "у Тора 2 фотосессии в порядке добавления");
    CHECK(countNodes(r.tree()) == 2 && treeMatchesArray(r), "2 узла дерева, дерево — отображение массива");
}

static void test_ps_remove() {
    std::cout << "Тест: Фотосессии — удаление по полному совпадению\n";
    PhotoSessionRepository r;
    bool v;
    r.add(P("Тор", "Marvel", "Петров Ю.", "Иванова А.", 1, 1, 2024, 100), v);
    r.add(P("Тор", "Marvel", "Петров Ю.", "Иванова А.", 1, 1, 2024, 150), v);
    r.add(P("Гуфи", "Classics", "Новиков П.", "Петров С.", 5, 5, 2024, 50), v);
    CHECK(!r.remove(P("Тор", "Marvel", "Петров Ю.", "Иванова А.", 1, 1, 2024, 999)), "другая стоимость — не удалено");
    CHECK(r.remove(P("Тор", "Marvel", "Петров Ю.", "Иванова А.", 1, 1, 2024, 150)), "удалена фотосессия за 150");
    int steps;
    auto list = r.findByKey({"Тор", "Marvel", "Петров Ю."}, steps);
    CHECK(list.size() == 1 && list[0].cost == 100, "у Тора осталась фотосессия за 100");
    CHECK(!r.isActive(1) && r.size() == 3, "ячейка 1 помечена удалённой, без сдвига");
    r.remove(P("Тор", "Marvel", "Петров Ю.", "Иванова А.", 1, 1, 2024, 100));
    CHECK(!r.hasKey({"Тор", "Marvel", "Петров Ю."}) && countNodes(r.tree()) == 1, "цепочка пуста — узел Тора удалён");
    CHECK(r.findByKey({"Гуфи", "Classics", "Новиков П."}, steps).size() == 1 && treeMatchesArray(r),
          "Гуфи находится, дерево соответствует массиву");
}

static void test_ps_filters() {
    std::cout << "Тест: Фотосессии — фильтры по посетителю и по дате\n";
    PhotoSessionRepository r;
    bool v;
    r.add(P("Тор", "Marvel", "Петров Ю.", "Иванова А.", 1, 1, 2024, 100), v);
    r.add(P("Белль", "Принцессы", "Сидоров М.", "Иванова А.", 2, 2, 2024, 200), v);
    r.add(P("Гуфи", "Classics", "Новиков П.", "Петров С.", 1, 1, 2024, 300), v);
    CHECK(r.filterByVisitor("Иванова А.").size() == 2, "Иванова А. — 2 фотосессии");
    CHECK(r.filterByDate(Date{1, 1, 2024}).size() == 2, "01 янв 2024 — 2 фотосессии");
    r.remove(P("Тор", "Marvel", "Петров Ю.", "Иванова А.", 1, 1, 2024, 100));
    CHECK(r.filterByVisitor("Иванова А.").size() == 1 && r.filterByDate(Date{1, 1, 2024}).size() == 1,
          "удалённая запись в фильтры не попадает");
}

static void test_ps_load_save() {
    std::cout << "Тест: Фотосессии — загрузка и сохранение файла\n";
    std::string in = tmpFile("ps.txt",
        "Тор;Marvel;Петров Ю.;Иванова А.;05 июн 2024;1500.50\n"
        "Тор;Marvel;Петров Ю.;Петров С.;31 июн 2024;100\n"
        "Тор;Marvel;Петров Ю.;Петров С.;05 ххх 2024;100\n"
        "Тор;Marvel;Петров Ю.;Петров С.;05 июн 2024;abc\n"
        "Белль;Принцессы;Сидоров М.;Сидоров К.;10 дек 2023;700\n");
    PhotoSessionRepository r;
    LoadStats s = r.loadFromFile(in);
    CHECK(s.loaded == 2 && s.badFormat == 2 && s.invalidData == 1, "загружено 2, формат 2, целостность 1");
    CHECK(r.saveToFile("test_tmp_ps_out.txt"), "сохранение выполнено");
    std::ifstream f("test_tmp_ps_out.txt");
    std::string line;
    std::getline(f, line);
    CHECK(line == "Тор;Marvel;Петров Ю.;Иванова А.;05 июн 2024;1500.50", "строка сохранена в исходном формате");
    f.close();
    std::remove(in.c_str());
    std::remove("test_tmp_ps_out.txt");
}

static void test_load_formats() {
    std::cout << "Тест: загрузка файлов Windows — BOM, CRLF, запятая в стоимости\n";
    std::istringstream chars("\xEF\xBB\xBFТор;Петров Ю.;Marvel\r\nБелль;Сидоров М.;Принцессы\r\n");
    CharacterRepository r(11);
    LoadStats s = r.loadFromStream(chars);
    CHECK(s.loaded == 2, "BOM и CRLF не мешают загрузке");
    CHECK(r.contains({"Тор", "Marvel", "Петров Ю."}), "первая запись (после BOM) находится по ключу");
    std::istringstream ps("Тор;Marvel;Петров Ю.;Иванова А.;05 июн 2024;1500,50\r\nТор;Marvel;Петров Ю.;Х;05 июн 2024;12x\r\n");
    PhotoSessionRepository p;
    LoadStats s2 = p.loadFromStream(ps);
    CHECK(s2.loaded == 1 && p.items()[0].cost == 1500.5, "стоимость с запятой 1500,50 разобрана");
    CHECK(s2.badFormat == 1, "стоимость 12x — неверный формат");
}

// =====================================================================
//                  Целостность между справочниками и отчёт
// =====================================================================

static void fill(AppState& st) {
    bool v, d, o, u;
    st.characters().add(C("Тор", "Петров Ю.", "Marvel"), v, d, o);
    st.characters().add(C("Белль", "Сидоров М.", "Принцессы"), v, d, o);
    st.characters().add(C("Ариэль", "Петров Ю.", "Принцессы"), v, d, o);
    st.characters().add(C("Гуфи", "Новиков П.", "Classics"), v, d, o);
    st.addSession(P("Тор", "Marvel", "Петров Ю.", "Иванова А.", 1, 1, 2024, 100), v, u);
    st.addSession(P("Тор", "Marvel", "Петров Ю.", "Петров С.", 1, 1, 2024, 200), v, u);
    st.addSession(P("Ариэль", "Принцессы", "Петров Ю.", "Иванова А.", 1, 1, 2024, 300), v, u);
    st.addSession(P("Ариэль", "Принцессы", "Петров Ю.", "Иванова А.", 2, 1, 2024, 400), v, u);
    st.addSession(P("Белль", "Принцессы", "Сидоров М.", "Иванова А.", 1, 1, 2024, 500), v, u);
}

static void test_integrity() {
    std::cout << "Тест: ссылочная целостность между справочниками\n";
    AppState st(17);
    fill(st);
    bool v, u, has;
    CHECK(!st.addSession(P("Тор", "DC", "Петров Ю.", "Х", 1, 1, 2024, 1), v, u) && u,
          "фотосессия несуществующего персонажа (Тор, DC, Петров Ю.) отклонена");
    CHECK(!st.addSession(P("Тор", "Marvel", "Иванов", "Х", 1, 1, 2024, 1), v, u) && u,
          "фотосессия с другим костюмером (Тор, Marvel, Иванов) отклонена");
    CHECK(!st.removeCharacter(C("Тор", "Петров Ю.", "Marvel"), has) && has,
          "персонажа с фотосессиями удалить нельзя");
    CHECK(!st.removeCharacter(C("Тор", "Петорв Ю.", "Marvel"), has) && !has,
          "опечатка в костюмере: запись не найдена (сначала проверяется полное совпадение)");
    CHECK(st.removeCharacter(C("Гуфи", "Новиков П.", "Classics"), has) && !has,
          "персонаж без фотосессий удалён");
    std::string in = tmpFile("ps_int.txt",
        "Тор;Marvel;Петров Ю.;Иванова А.;05 июн 2024;10\n"
        "Гуфи;Classics;Новиков П.;Иванова А.;05 июн 2024;10\n");
    LoadStats s = st.loadSessions(in);
    CHECK(s.loaded == 1 && s.unknownCharacter == 1, "при загрузке запись без персонажа (Гуфи) отброшена");
    std::remove(in.c_str());
}

static void test_report() {
    std::cout << "Тест: отчёт по фильтрам (Костюмер, Посетитель, Дата)\n";
    AppState st(17);
    fill(st);
    ReportFilter f;
    f.costumer = "Петров Ю.";
    f.visitor = "Иванова А.";
    f.useDate = true;
    f.date = Date{1, 1, 2024};
    Report r = st.buildReport(f);
    CHECK(r.charactersMatched == 2, "по костюмеру Петров Ю. — 2 персонажа (Тор, Ариэль)");
    CHECK(r.rows.size() == 2, "в отчёте 2 строки");
    CHECK(r.rows[0].character.name == "Тор" && r.rows[0].session.cost == 100, "строка 1: Тор, 100");
    CHECK(r.rows[1].character.name == "Ариэль" && r.rows[1].session.cost == 300, "строка 2: Ариэль, 300");
    CHECK(r.rows[1].character.costumer == "Петров Ю." && r.rows[1].session.visitor == "Иванова А.",
          "в строке есть поля обоих справочников");
    CHECK(r.treeSteps > 0, "фотосессии найдены через АВЛ-дерево (шаги > 0)");

    f.useDate = false;
    CHECK(st.buildReport(f).rows.size() == 3, "без фильтра по дате — 3 строки");
    f.visitor.clear();
    CHECK(st.buildReport(f).rows.size() == 4, "только по костюмеру — 4 строки");
    f.costumer = "Нет такого";
    CHECK(st.buildReport(f).rows.empty(), "несуществующий костюмер — пустой отчёт");
}

int main() {
    std::cout << "##### Функционал справочников и отчёта #####\n";
    test_char_add_find();
    test_char_remove();
    test_char_filter_capacity();
    test_char_load_save();
    test_ps_add_find();
    test_ps_remove();
    test_ps_filters();
    test_ps_load_save();
    test_load_formats();
    test_integrity();
    test_report();
    return finishTests();
}
