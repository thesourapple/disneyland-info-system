cmake_minimum_required(VERSION 3.16)
project(DisneylandCoursework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_PREFIX_PATH "C:/Qt/6.11.2/mingw_64" CACHE PATH "Path to Qt installation")

# ---------- Ядро (без Qt) — компилируется и тестируется отдельно ----------
set(CORE_SOURCES
    core/date.cpp
    core/key.cpp
    core/hash_table.cpp
    core/avl_tree.cpp
    core/storage.cpp
    core/report.cpp
    core/character_repository.cpp
    core/photo_session_repository.cpp
    core/app_state.cpp
)

add_library(disney_core STATIC ${CORE_SOURCES})
target_include_directories(disney_core PUBLIC core)

# ---------- Тесты (без Qt): СД отдельно, функционал отдельно ----------
add_executable(tests_structures tests/tests_structures.cpp)
target_link_libraries(tests_structures PRIVATE disney_core)
add_executable(tests_functional tests/tests_functional.cpp)
target_link_libraries(tests_functional PRIVATE disney_core)

# ---------- Генератор входных данных ----------
add_executable(disney_generator generator.cpp)

# ---------- Qt GUI ----------
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

add_executable(disney_gui
    gui/main.cpp
    gui/gui_common.h
    gui/character_window.h
    gui/character_window.cpp
    gui/photo_session_window.h
    gui/photo_session_window.cpp
)
target_link_libraries(disney_gui PRIVATE disney_core Qt${QT_VERSION_MAJOR}::Widgets)
# moc (обработка Q_OBJECT) нужен только окнам GUI, ядру и тестам — нет
set_target_properties(disney_gui PROPERTIES AUTOMOC ON WIN32_EXECUTABLE ON)
