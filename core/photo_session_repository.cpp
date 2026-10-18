// core/photo_session_repository.cpp
#include "photo_session_repository.h"
#include <iomanip>

PhotoSessionRepository::PhotoSessionRepository() : root_(createEmptyTree()) {}

PhotoSessionRepository::~PhotoSessionRepository() {
    freeTree(root_);
}

LoadStats PhotoSessionRepository::loadFromFile(const std::string& filename,
                                               const std::function<bool(const CompositeKey&)>& keyExists) {
    std::ifstream in(utf8Path(filename));
    if (!in.is_open()) { LoadStats s; s.fileOpened = false; return s; }
    return loadFromStream(in, keyExists);
}

LoadStats PhotoSessionRepository::loadFromStream(std::istream& in,
                                                 const std::function<bool(const CompositeKey&)>& keyExists) {
    std::vector<PhotoSession> parsed;
    LoadStats stats = readPhotoSessions(in, parsed);

    freeTree(root_);
    root_ = createEmptyTree();
    items_.clear();
    stats.loaded = 0;
    for (const auto& p : parsed) {
        if (keyExists && !keyExists(keyOf(p))) { ++stats.unknownCharacter; continue; }
        bool validData;
        if (add(p, validData)) ++stats.loaded;
        else ++stats.invalidData;
    }
    return stats;
}

bool PhotoSessionRepository::saveToFile(const std::string& filename) const {
    return writePhotoSessionsFile(filename, items());
}

void PhotoSessionRepository::saveToStream(std::ostream& out) const {
    writePhotoSessions(out, items());
}

bool PhotoSessionRepository::add(const PhotoSession& p, bool& validData) {
    validData = isValidPhotoSession(p);
    if (!validData) return false;
    int index = static_cast<int>(items_.size());
    items_.push_back(PhotoSessionSlot{true, p});
    bool inserted = false;
    root_ = insertIndex(root_, keyOf(p), index, inserted);
    return true;
}

bool PhotoSessionRepository::remove(const PhotoSession& p) {
    const AVLNode* node = searchByKey(root_, keyOf(p));
    if (node == nullptr) return false;
    int found = -1;
    for (const IndexNode* q = node->indexes; q != nullptr; q = q->next) {
        if (recordEquals(items_[q->arrayIndex].data, p)) { found = q->arrayIndex; break; }
    }
    if (found == -1) return false;
    items_[found].active = false;                       // 1) массив
    bool removed = false;
    root_ = removeIndex(root_, keyOf(p), found, removed);  // 2) дерево
    return removed;
}

std::vector<PhotoSession> PhotoSessionRepository::findByKey(const CompositeKey& key, int& steps) const {
    std::vector<PhotoSession> result;
    const AVLNode* node = searchByKey(root_, key, steps);
    if (node == nullptr) return result;
    for (const IndexNode* q = node->indexes; q != nullptr; q = q->next)
        result.push_back(items_[q->arrayIndex].data);
    return result;
}

bool PhotoSessionRepository::hasKey(const CompositeKey& key) const {
    return searchByKey(root_, key) != nullptr;
}

std::vector<PhotoSession> PhotoSessionRepository::filterByVisitor(const std::string& visitor) const {
    std::vector<PhotoSession> result;
    for (const auto& s : items_)
        if (s.active && s.data.visitor == visitor) result.push_back(s.data);
    return result;
}

std::vector<PhotoSession> PhotoSessionRepository::filterByDate(const Date& date) const {
    std::vector<PhotoSession> result;
    for (const auto& s : items_)
        if (s.active && s.data.date.day == date.day && s.data.date.month == date.month &&
            s.data.date.year == date.year)
            result.push_back(s.data);
    return result;
}

std::vector<PhotoSession> PhotoSessionRepository::items() const {
    std::vector<PhotoSession> result;
    for (const auto& s : items_)
        if (s.active) result.push_back(s.data);
    return result;
}

int PhotoSessionRepository::activeCount() const {
    int n = 0;
    for (const auto& s : items_) if (s.active) ++n;
    return n;
}

void PhotoSessionRepository::printArray(std::ostream& os) const {
    os << "Массив: ячеек = " << size() << ", активных = " << activeCount() << "\n";
    os << std::fixed << std::setprecision(2);
    for (int i = 0; i < size(); ++i) {
        const PhotoSession& p = items_[i].data;
        os << "[" << i << "] " << (items_[i].active ? "" : "(удалена) ")
           << p.characterName << "; " << p.franchise << "; " << p.costumer << "; " << p.visitor << "; "
           << dateToString(p.date) << "; " << p.cost << "\n";
    }
}
