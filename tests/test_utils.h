// tests/test_utils.h — общий макрос проверок для тестов курсовой.
#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
static const bool consoleUtf8 = (SetConsoleOutputCP(CP_UTF8), true);
#endif

#include <iostream>

static int testsRun = 0;
static int testsFailed = 0;

#define CHECK(cond, msg)                                                    \
    do {                                                                    \
        ++testsRun;                                                         \
        if (!(cond)) {                                                      \
            ++testsFailed;                                                  \
            std::cout << "  [FAIL] " << msg << "  (" << #cond << ")\n";     \
        } else {                                                            \
            std::cout << "  [ OK ] " << msg << "\n";                        \
        }                                                                   \
    } while (0)

static int finishTests() {
    std::cout << "\nВсего проверок: " << testsRun << ", провалено: " << testsFailed << "\n";
    return testsFailed == 0 ? 0 : 1;
}

#endif
