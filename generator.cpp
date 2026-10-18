// generator.cpp
// Простой генератор входных файлов для справочников «Персонажи» и «Фотосессии».
// Формат строк — тот же, что ожидает core/storage.cpp (поля через ';'):
//   Персонажи:  Имя;Костюмер;Франшиза
//   Фотосессии: ИмяПерсонажа;Франшиза;Посетитель;ДД мес ГГГГ;Стоимость
//
// Персонажи для фотосессий берутся из уже сгенерированного списка персонажей,
// поэтому файлы получаются согласованными (отчёт сможет находить совпадения).
//
// Использование:
//   generator <персонажи.txt> <фотосессии.txt> <кол-во_персонажей> <кол-во_фотосессий>

#include <iostream>
#include <fstream>
#include <random>
#include <string>
#include <vector>
#include <algorithm>
#include <array>
#include <cstdio>

struct CharacterInfo {
    std::string name;
    std::string franchise;
};

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cerr << "Использование: " << argv[0]
                   << " <персонажи.txt> <фотосессии.txt> <кол-во_персонажей> <кол-во_фотосессий>\n";
        return 1;
    }
    const std::string charactersFile = argv[1];
    const std::string sessionsFile = argv[2];
    int wantCharacters = std::stoi(argv[3]);
    int wantSessions = std::stoi(argv[4]);

    // Фиксированный пул персонажей (имя жёстко привязано к франшизе, как в реальном
    // справочнике — один и тот же персонаж не "переезжает" между вселенными).
    std::vector<CharacterInfo> pool = {
        {"Микки Маус", "Disney Classics"}, {"Минни Маус", "Disney Classics"},
        {"Дональд Дак", "Disney Classics"}, {"Гуфи", "Disney Classics"},
        {"Плуто", "Disney Classics"},
        {"Белль", "Принцессы"}, {"Ариэль", "Принцессы"}, {"Золушка", "Принцессы"},
        {"Рапунцель", "Принцессы"}, {"Жасмин", "Принцессы"},
        {"Человек-паук", "Marvel"}, {"Железный человек", "Marvel"},
        {"Тор", "Marvel"}, {"Чёрная пантера", "Marvel"},
        {"Базз Лайтер", "Pixar"}, {"Вуди", "Pixar"}, {"Молния Маккуин", "Pixar"},
        {"Дарт Вейдер", "Star Wars"}, {"Йода", "Star Wars"}, {"Чубакка", "Star Wars"}
    };

    std::vector<std::string> costumerSurnames = {
        "Иванов", "Петров", "Сидоров", "Кузнецов", "Смирнов", "Попов",
        "Соколов", "Лебедев", "Козлов", "Новиков"
    };
    std::vector<std::string> visitorFirstNames = {
        "Анна", "Ольга", "Мария", "Дмитрий", "Сергей", "Андрей",
        "Екатерина", "Иван", "Наталья", "Алексей"
    };
    std::vector<std::string> visitorSurnames = {
        "Морозова", "Волкова", "Зайцев", "Соловьёв", "Виноградов",
        "Богданов", "Воробьёв", "Фёдоров", "Михайлов", "Беляев"
    };
    static const std::array<std::string, 12> MONTHS = {
        "янв", "фев", "мар", "апр", "май", "июн",
        "июл", "авг", "сен", "окт", "ноя", "дек"
    };

    std::random_device rd;
    std::mt19937 gen(rd());

    // ---------- Персонажи ----------
    std::shuffle(pool.begin(), pool.end(), gen);
    if (wantCharacters > static_cast<int>(pool.size())) {
        std::cerr << "В пуле только " << pool.size() << " уникальных персонажей, "
                   << "будет сгенерировано " << pool.size() << " вместо " << wantCharacters << ".\n";
        wantCharacters = static_cast<int>(pool.size());
    }
    std::vector<CharacterInfo> chosen(pool.begin(), pool.begin() + wantCharacters);

    std::vector<std::string> initials = {
        "А", "Б", "В", "Г", "Д", "Е", "И", "К", "Л", "М",
        "Н", "О", "П", "Р", "С", "Т", "Ю", "Я"
    };
    std::uniform_int_distribution<> costumerSurnameDist(0, static_cast<int>(costumerSurnames.size()) - 1);
    std::uniform_int_distribution<> initialDist(0, static_cast<int>(initials.size()) - 1);

    std::ofstream charOut(charactersFile);
    if (!charOut.is_open()) {
        std::cerr << "Не удалось открыть файл для записи: " << charactersFile << "\n";
        return 1;
    }
    for (const auto& c : chosen) {
        std::string costumer = costumerSurnames[costumerSurnameDist(gen)] + " " +
                                initials[initialDist(gen)] + ".";
        charOut << c.name << ";" << costumer << ";" << c.franchise << "\n";
    }
    charOut.close();
    std::cout << "Сгенерирован файл '" << charactersFile << "' с " << chosen.size() << " персонажами.\n";

    if (chosen.empty()) {
        std::cerr << "Персонажей нет — фотосессии генерировать не из чего.\n";
        return 0;
    }

    // ---------- Фотосессии ----------
    std::uniform_int_distribution<> characterIndexDist(0, static_cast<int>(chosen.size()) - 1);
    std::uniform_int_distribution<> visitorFirstDist(0, static_cast<int>(visitorFirstNames.size()) - 1);
    std::uniform_int_distribution<> visitorSurnameDist(0, static_cast<int>(visitorSurnames.size()) - 1);
    std::uniform_int_distribution<> dayDist(1, 28);
    std::uniform_int_distribution<> monthDist(0, 11);
    std::uniform_int_distribution<> yearDist(2022, 2025);
    std::uniform_real_distribution<> costDist(500.0, 5000.0);

    std::ofstream sessOut(sessionsFile);
    if (!sessOut.is_open()) {
        std::cerr << "Не удалось открыть файл для записи: " << sessionsFile << "\n";
        return 1;
    }
    for (int i = 0; i < wantSessions; ++i) {
        const CharacterInfo& c = chosen[characterIndexDist(gen)];
        std::string visitor = visitorSurnames[visitorSurnameDist(gen)] + " " +
                               visitorFirstNames[visitorFirstDist(gen)];
        char dateBuf[32];
        std::snprintf(dateBuf, sizeof(dateBuf), "%02d %s %04d",
                      dayDist(gen), MONTHS[monthDist(gen)].c_str(), yearDist(gen));
        char costBuf[32];
        std::snprintf(costBuf, sizeof(costBuf), "%.2f", costDist(gen));

        sessOut << c.name << ";" << c.franchise << ";" << visitor << ";"
                << dateBuf << ";" << costBuf << "\n";
    }
    sessOut.close();
    std::cout << "Сгенерирован файл '" << sessionsFile << "' с " << wantSessions << " фотосессиями.\n";

    return 0;
}
