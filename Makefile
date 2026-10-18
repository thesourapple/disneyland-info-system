# Makefile курсовой. Запуск из терминала VS Code в папке KURSACH:
#   make gui        — собрать GUI (CMake + Qt MinGW), скопировать DLL и запустить
#   make gui-build  — только собрать GUI
#   make run        — запустить уже собранный GUI
#   make tests      — собрать и прогнать тесты ядра (без Qt)
#   make check      — показать, какой Qt и MinGW найдены
#   make clean      — удалить сборку
# Если make называется mingw32-make — те же цели: mingw32-make gui

# ---------- пути Qt (поменяйте, если Qt стоит в другом месте) ----------
QT    ?= C:/Qt/6.11.2/mingw_64
MINGW ?= $(firstword $(wildcard C:/Qt/Tools/mingw*_64))
CMAKE ?= cmake

BUILD = build
GUI   = $(BUILD)/disney_gui.exe

CMAKE_ARGS = "-DCMAKE_PREFIX_PATH=$(QT)" "-DCMAKE_C_COMPILER=$(MINGW)/bin/gcc.exe" \
             "-DCMAKE_CXX_COMPILER=$(MINGW)/bin/g++.exe" "-DCMAKE_MAKE_PROGRAM=$(MINGW)/bin/mingw32-make.exe"

# ---------- GUI ----------
gui: gui-build run

check:
	@echo Qt:    $(QT)
	@echo MinGW: $(MINGW)

gui-build: $(BUILD)/CMakeCache.txt
	$(CMAKE) --build $(BUILD) -j 4
	"$(QT)/bin/windeployqt.exe" --release --no-translations $(GUI)
	$(CMAKE) -E copy "$(MINGW)/bin/libgcc_s_seh-1.dll" "$(MINGW)/bin/libstdc++-6.dll" "$(MINGW)/bin/libwinpthread-1.dll" $(BUILD)
	$(CMAKE) -E copy input_characters.txt input_photo.txt examples/characters.txt examples/photo_sessions.txt $(BUILD)

# настройка CMake — один раз (или после make clean); компилятор берётся из Qt, а не из msys64
$(BUILD)/CMakeCache.txt:
	$(CMAKE) -S . -B $(BUILD) -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release $(CMAKE_ARGS)

run:
	$(CMAKE) -E chdir $(BUILD) "$(CURDIR)/$(GUI)"

# ---------- тесты ядра (без Qt) ----------
CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Icore
CORE = core/date.cpp core/key.cpp core/hash_table.cpp core/avl_tree.cpp core/storage.cpp \
       core/report.cpp core/character_repository.cpp core/photo_session_repository.cpp core/app_state.cpp
HDRS = $(wildcard core/*.h) tests/test_utils.h
T1 = $(BUILD)/tests_structures.exe
T2 = $(BUILD)/tests_functional.exe

tests: $(T1) $(T2)
	"$(CURDIR)/$(T1)"
	"$(CURDIR)/$(T2)"

$(T1): tests/tests_structures.cpp $(CORE) $(HDRS)
	$(CMAKE) -E make_directory $(BUILD)
	$(CXX) $(CXXFLAGS) tests/tests_structures.cpp $(CORE) -o $@

$(T2): tests/tests_functional.cpp $(CORE) $(HDRS)
	$(CMAKE) -E make_directory $(BUILD)
	$(CXX) $(CXXFLAGS) tests/tests_functional.cpp $(CORE) -o $@

clean:
	$(CMAKE) -E rm -rf $(BUILD)

.PHONY: gui gui-build check run tests clean
