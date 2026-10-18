// core/app_state.h
// Общее состояние приложения: оба справочника + проверка ссылочной целостности
// между ними и отчёт. Оба окна GUI работают с одним объектом AppState.
// От Qt не зависит — тестируется вместе с ядром.
//
// Целостность:
//  - фотосессию можно добавить/загрузить, только если её персонаж (Имя+Франшиза+Костюмер)
//    есть в Справочнике_1 (проверка — поиском в хеш-таблице);
//  - персонажа нельзя удалить, пока у него есть фотосессии (проверка — поиском в АВЛ).

#ifndef CORE_APP_STATE_H
#define CORE_APP_STATE_H

#include "character_repository.h"
#include "photo_session_repository.h"
#include "report.h"

class AppState {
public:
    explicit AppState(int initialHashCapacity);

    CharacterRepository& characters() { return characters_; }
    PhotoSessionRepository& sessions() { return sessions_; }
    const CharacterRepository& characters() const { return characters_; }
    const PhotoSessionRepository& sessions() const { return sessions_; }

    // Удаление персонажа: сначала проверяется полное совпадение всех полей (нет — false,
    // hasSessions = false), затем наличие фотосессий (есть — false, hasSessions = true).
    bool removeCharacter(const Character& c, bool& hasSessions);

    // Добавление фотосессии: unknownCharacter = true — отказ, нет такого персонажа.
    bool addSession(const PhotoSession& p, bool& validData, bool& unknownCharacter);

    // Загрузка фотосессий с проверкой, что персонаж каждой записи существует.
    LoadStats loadSessions(const std::string& filename);
    LoadStats loadSessions(std::istream& in);

    Report buildReport(const ReportFilter& filter) const;

private:
    CharacterRepository characters_;
    PhotoSessionRepository sessions_;
};

#endif // CORE_APP_STATE_H
