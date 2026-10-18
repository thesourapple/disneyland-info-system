// gui/gui_common.h
// Общие функции окон: чтение/запись текстовых файлов через QFile (корректно
// работает с путями на кириллице и с файлами в UTF-8 / Windows-1251),
// текст сообщения о результатах загрузки.

#ifndef GUI_COMMON_H
#define GUI_COMMON_H

#include <QString>
#include <QByteArray>
#include <QFile>
#include <QStringDecoder>
#include <string>
#include "storage.h"

namespace gui {

inline QString q(const std::string& s) { return QString::fromStdString(s); }

// Читает файл целиком и возвращает текст в UTF-8. Если файл не в UTF-8
// (например, сохранён в Блокноте как ANSI), он читается в кодировке системы (Windows-1251).
inline bool readTextFile(const QString& path, std::string& out) {
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly)) return false;
    QByteArray bytes = f.readAll();
    QStringDecoder decoder(QStringDecoder::Utf8);
    QString text = decoder.decode(bytes);
    if (decoder.hasError()) text = QString::fromLocal8Bit(bytes);
    out = text.toStdString();
    return true;
}

inline bool writeTextFile(const QString& path, const std::string& data) {
    QFile f(path);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false;
    return f.write(data.data(), static_cast<qint64>(data.size())) == static_cast<qint64>(data.size());
}

inline QString loadStatsText(const LoadStats& s) {
    QString t = QStringLiteral("Непустых строк: %1\nЗагружено: %2").arg(s.totalLines).arg(s.loaded);
    if (s.badFormat)        t += QStringLiteral("\nНеверный формат строки: %1").arg(s.badFormat);
    if (s.invalidData)      t += QStringLiteral("\nНекорректные данные: %1").arg(s.invalidData);
    if (s.duplicateKey)     t += QStringLiteral("\nПовтор ключа (Имя+Франшиза+Костюмер): %1").arg(s.duplicateKey);
    if (s.overflow)         t += QStringLiteral("\nНе хватило места в ХТ: %1 (увеличьте размер ХТ)").arg(s.overflow);
    if (s.unknownCharacter) t += QStringLiteral("\nНет персонажа в справочнике «Персонажи»: %1"
                                                "\n(сначала загрузите персонажей)").arg(s.unknownCharacter);
    if (s.totalLines > 0 && s.loaded == 0 && s.badFormat == s.totalLines)
        t += QStringLiteral("\n\nНи одна строка не разобрана. Формат строки:\n"
                            "Персонажи: Имя;Костюмер;Франшиза\n"
                            "Фотосессии: Имя;Франшиза;Костюмер;Посетитель;ДД мес ГГГГ;Стоимость");
    return t;
}

} // namespace gui

#endif // GUI_COMMON_H
