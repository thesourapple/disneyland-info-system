// core/character_repository.cpp
#include "character_repository.h"

CharacterRepository::CharacterRepository(int hashTableCapacity)
    : table_(std::make_unique<HashTable>(hashTableCapacity)) {}

LoadStats CharacterRepository::loadFromFile(const std::string& filename) {
    std::ifstream in(utf8Path(filename));
    if (!in.is_open()) { LoadStats s; s.fileOpened = false; return s; }
    return loadFromStream(in);
}

LoadStats CharacterRepository::loadFromStream(std::istream& in) {
    std::vector<Character> parsed;
    LoadStats stats = readCharacters(in, parsed);

    items_.clear();
    table_ = std::make_unique<HashTable>(table_->getCapacity());
    stats.loaded = 0;
    for (const auto& c : parsed) {
        bool validData, duplicateKey, overflow;
        if (add(c, validData, duplicateKey, overflow)) ++stats.loaded;
        else if (duplicateKey) ++stats.duplicateKey;
        else if (overflow) ++stats.overflow;
        else ++stats.invalidData;
    }
    return stats;
}

bool CharacterRepository::saveToFile(const std::string& filename) const {
    return writeCharactersFile(filename, items());
}

void CharacterRepository::saveToStream(std::ostream& out) const {
    writeCharacters(out, items());
}

bool CharacterRepository::add(const Character& c, bool& validData, bool& duplicateKey, bool& overflow) {
    duplicateKey = overflow = false;
    validData = isValidCharacter(c);
    if (!validData) return false;

    int index = static_cast<int>(items_.size());
    if (!table_->insert(keyOf(c), index, duplicateKey, overflow)) return false;
    items_.push_back(CharacterSlot{true, c});
    return true;
}

bool CharacterRepository::remove(const Character& c) {
    int steps = 0;
    int index = table_->search(keyOf(c), steps);
    if (index == -1 || !recordEquals(items_[index].data, c)) return false;
    items_[index].active = false;          // 1) массив
    return table_->remove(keyOf(c));        // 2) таблица
}

bool CharacterRepository::findByKey(const CompositeKey& key, Character& out, int& steps) const {
    int index = table_->search(key, steps);
    if (index == -1) return false;
    out = items_[index].data;
    return true;
}

bool CharacterRepository::contains(const CompositeKey& key) const {
    int steps = 0;
    return table_->search(key, steps) != -1;
}

std::vector<Character> CharacterRepository::filterByCostumer(const std::string& costumer) const {
    std::vector<Character> result;
    for (const auto& s : items_)
        if (s.active && s.data.costumer == costumer) result.push_back(s.data);
    return result;
}

bool CharacterRepository::setCapacity(int newCapacity) {
    if (newCapacity < 1 || newCapacity < activeCount()) return false;
    auto newTable = std::make_unique<HashTable>(newCapacity);
    for (int i = 0; i < size(); ++i) {
        if (!items_[i].active) continue;
        bool duplicateKey, overflow;
        newTable->insert(keyOf(items_[i].data), i, duplicateKey, overflow);
    }
    table_ = std::move(newTable);
    return true;
}

std::vector<Character> CharacterRepository::items() const {
    std::vector<Character> result;
    for (const auto& s : items_)
        if (s.active) result.push_back(s.data);
    return result;
}

int CharacterRepository::activeCount() const {
    int n = 0;
    for (const auto& s : items_) if (s.active) ++n;
    return n;
}

void CharacterRepository::printArray(std::ostream& os) const {
    os << "Массив: ячеек = " << size() << ", активных = " << activeCount() << "\n";
    for (int i = 0; i < size(); ++i) {
        const Character& c = items_[i].data;
        os << "[" << i << "] " << (items_[i].active ? "" : "(удалена) ")
           << c.name << "; " << c.costumer << "; " << c.franchise << "\n";
    }
}
