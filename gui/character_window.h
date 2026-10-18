// gui/character_window.h
// Окно «ХТ» — Справочник 1 (Персонажи). Своя панель инструментов (QToolBar —
// аналог ToolStrip), таблица справочника и окно отладки: хеш-таблица
// (слот, статус, h1, h2, ключ, индекс в массиве).

#ifndef GUI_CHARACTER_WINDOW_H
#define GUI_CHARACTER_WINDOW_H

#include <QMainWindow>
#include <vector>
#include "app_state.h"

class QTableWidget;
class QLabel;

class CharacterWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit CharacterWindow(AppState& state, QWidget* parent = nullptr);

public slots:
    void refreshAll();   // массив целиком + окно отладки

signals:
    void dataChanged();  // справочник изменился (нужно обновить второе окно)

private slots:
    void onLoad();
    void onSave();
    void onAdd();
    void onRemove();
    void onSearch();
    void onFilter();
    void onShowAll();
    void onSetCapacity();
    void onReport();

private:
    AppState& state_;
    QTableWidget* table_;      // записи справочника (массив)
    QTableWidget* hashView_;   // окно отладки: хеш-таблица
    QLabel* status_;

    void showRows(const std::vector<Character>& items, const std::vector<int>& indexes);
    void refreshDebugView();
};

#endif // GUI_CHARACTER_WINDOW_H
