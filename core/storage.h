// core/storage.h
// Чтение/запись справочников из текстовых файлов в массивы (std::vector).
// Формат строки (поля разделены ';', т.к. текстовые поля могут содержать пробелы):
//   Персонажи:  Имя;Костюмер;Франшиза
//   Фотосессии: ИмяПерсонажа;Франшиза;Костюмер;Посетитель;ДД мес ГГГГ;Стоимость

#ifndef CORE_STORAGE_H
#define CORE_STORAGE_H

#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>
#include "models.h"

// Открытие файла по пути в UTF-8 (так приходит путь из Qt). Через
// filesystem::u8path путь с кириллицей корректно открывается и на Windows.
inline std::filesystem::path utf8Path(const std::string& s) {
    return std::filesystem::u8path(s);
}

struct LoadStats {
    int totalLines = 0;
    int loaded = 0;
    int badFormat = 0;      // не удалось разобрать строку
    int invalidData = 0;    // данные разобраны, но не прошли проверку целостности
    int duplicateKey = 0;   // только для Персонажей (уникальный ключ)
    int overflow = 0;       // только для Персонажей (переполнение ХТ)
    int unknownCharacter = 0; // только для Фотосессий (нет такого персонажа в Справочнике_1)
    bool fileOpened = true;   // false, если файл не удалось открыть
};

// Чтение справочника из потока (строки текста) в массив.
LoadStats readCharacters(std::istream& in, std::vector<Character>& out);
LoadStats readPhotoSessions(std::istream& in, std::vector<PhotoSession>& out);

// Запись массива справочника в поток.
void writeCharacters(std::ostream& out, const std::vector<Character>& items);
void writePhotoSessions(std::ostream& out, const std::vector<PhotoSession>& items);

// Чтение справочника из файла в массив (порядок строк файла сохраняется).
// В массив попадают только разобранные и прошедшие проверку целостности записи.
LoadStats readCharactersFile(const std::string& filename, std::vector<Character>& out);
LoadStats readPhotoSessionsFile(const std::string& filename, std::vector<PhotoSession>& out);

// Запись массива справочника в файл в том же формате.
bool writeCharactersFile(const std::string& filename, const std::vector<Character>& items);
bool writePhotoSessionsFile(const std::string& filename, const std::vector<PhotoSession>& items);

#endif // CORE_STORAGE_H
