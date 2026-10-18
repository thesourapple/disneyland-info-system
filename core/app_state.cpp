// core/app_state.cpp
#include "app_state.h"

AppState::AppState(int initialHashCapacity) : characters_(initialHashCapacity) {}

bool AppState::removeCharacter(const Character& c, bool& hasSessions) {
    hasSessions = false;
    // 1) запись с полным совпадением всех полей должна существовать
    Character found;
    int steps = 0;
    if (!characters_.findByKey(keyOf(c), found, steps) || !recordEquals(found, c)) return false;
    // 2) ссылочная целостность: у персонажа не должно быть фотосессий
    hasSessions = sessions_.hasKey(keyOf(c));
    if (hasSessions) return false;
    return characters_.remove(c);
}

bool AppState::addSession(const PhotoSession& p, bool& validData, bool& unknownCharacter) {
    validData = isValidPhotoSession(p);
    unknownCharacter = false;
    if (!validData) return false;
    unknownCharacter = !characters_.contains(keyOf(p));
    if (unknownCharacter) return false;
    return sessions_.add(p, validData);
}

LoadStats AppState::loadSessions(const std::string& filename) {
    return sessions_.loadFromFile(filename, [this](const CompositeKey& k) {
        return characters_.contains(k);
    });
}

LoadStats AppState::loadSessions(std::istream& in) {
    return sessions_.loadFromStream(in, [this](const CompositeKey& k) {
        return characters_.contains(k);
    });
}

Report AppState::buildReport(const ReportFilter& filter) const {
    return ::buildReport(characters_, sessions_, filter);
}
