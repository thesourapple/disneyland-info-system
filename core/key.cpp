// core/key.cpp
#include "key.h"
#include <cmath>

namespace {
const double GOLDEN_RATIO_A = 0.6180339887498949;
}

long long foldStringToInt(const std::string& s) {
    // Строка хранится в UTF-8: символ занимает 1-4 байта. Собираем из байтов
    // код символа и прибавляем его к сумме.
    long long sum = 0;
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = static_cast<unsigned char>(s[i]);
        long long code;
        int len;
        if (c < 0x80)              { code = c;        len = 1; }
        else if ((c >> 5) == 0x6)  { code = c & 0x1F; len = 2; }
        else if ((c >> 4) == 0xE)  { code = c & 0x0F; len = 3; }
        else if ((c >> 3) == 0x1E) { code = c & 0x07; len = 4; }
        else                       { code = c;        len = 1; } // некорректный байт
        for (int k = 1; k < len && i + k < s.size(); ++k)
            code = (code << 6) | (static_cast<unsigned char>(s[i + k]) & 0x3F);
        sum += code;
        i += len;
    }
    return sum;
}

int primaryHashKey(const CompositeKey& key, int capacity) {
    long long g = foldStringToInt(key.name) + foldStringToInt(key.franchise) + foldStringToInt(key.costumer);

    double val = static_cast<double>(g) * GOLDEN_RATIO_A;
    double frac = val - std::floor(val);
    int h = static_cast<int>(std::floor(capacity * frac));
    if (h < 0) h = 0;
    if (h >= capacity) h = capacity - 1;
    return h;
}
