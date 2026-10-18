// core/hash_table.cpp
#include "hash_table.h"
#include <iomanip>

HashTable::HashTable(int capacity)
    : table_(capacity > 0 ? capacity : 1), capacity_(capacity > 0 ? capacity : 1), count_(0) {
    for (auto& s : table_) s = HashSlot{SLOT_EMPTY, CompositeKey{}, -1};
}

HashTable::~HashTable() {}

int HashTable::primaryHash(const CompositeKey& key) const {
    return primaryHashKey(key, capacity_);
}

int HashTable::secondaryHash(const CompositeKey& /*key*/) const {
    return 1;
}

int HashTable::probe(const CompositeKey& key, int attempt) const {
    long long h = (static_cast<long long>(primaryHash(key)) +
                   static_cast<long long>(attempt) * secondaryHash(key)) % capacity_;
    return static_cast<int>(h);
}

// Статус 0 обрывает цепочку пробирования, статус 2 пропускается.
int HashTable::findSlot(const CompositeKey& key, int& steps) const {
    steps = 0;
    for (int attempt = 0; attempt < capacity_; ++attempt) {
        int idx = probe(key, attempt);
        ++steps;
        if (table_[idx].status == SLOT_EMPTY) return -1;
        if (table_[idx].status == SLOT_OCCUPIED && table_[idx].key == key) return idx;
    }
    return -1;
}

bool HashTable::insert(const CompositeKey& key, int arrayIndex, bool& duplicateKey, bool& overflow) {
    duplicateKey = false;
    overflow = false;
    int firstDeleted = -1;
    for (int attempt = 0; attempt < capacity_; ++attempt) {
        int idx = probe(key, attempt);
        const HashSlot& s = table_[idx];
        if (s.status == SLOT_EMPTY) {
            int at = (firstDeleted != -1) ? firstDeleted : idx;
            table_[at] = HashSlot{SLOT_OCCUPIED, key, arrayIndex};
            ++count_;
            return true;
        }
        if (s.status == SLOT_DELETED) {
            if (firstDeleted == -1) firstDeleted = idx;
        } else if (s.key == key) {
            duplicateKey = true;
            return false;
        }
    }
    if (firstDeleted != -1) {
        table_[firstDeleted] = HashSlot{SLOT_OCCUPIED, key, arrayIndex};
        ++count_;
        return true;
    }
    overflow = true;
    return false;
}

bool HashTable::remove(const CompositeKey& key) {
    int steps = 0;
    int idx = findSlot(key, steps);
    if (idx == -1) return false;
    table_[idx].status = SLOT_DELETED;
    --count_;
    return true;
}

int HashTable::search(const CompositeKey& key, int& steps) const {
    int idx = findSlot(key, steps);
    return idx == -1 ? -1 : table_[idx].arrayIndex;
}

void HashTable::print(std::ostream& os) const {
    os << "Хеш-таблица: размер = " << capacity_ << ", занято = " << count_
       << ", заполнение = " << std::fixed << std::setprecision(2)
       << static_cast<double>(count_) / capacity_ << "\n";
    for (int i = 0; i < capacity_; ++i) {
        const HashSlot& s = table_[i];
        os << "[" << std::setw(3) << i << "] статус=" << s.status;
        if (s.status == SLOT_OCCUPIED) {
            os << "  h1=" << std::setw(3) << primaryHash(s.key)
               << "  h2=" << secondaryHash(s.key)
               << "  ключ=(" << s.key.name << "; " << s.key.franchise << "; " << s.key.costumer << ")"
               << "  индекс=" << s.arrayIndex;
        }
        os << "\n";
    }
}
