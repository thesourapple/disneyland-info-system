// gui/photo_session_window.h
// Окно «Дерево» — Справочник 2 (Фотосессии). Своя панель инструментов,
// таблица справочника и окно отладки: АВЛ-дерево с цепочками индексов.

#ifndef GUI_PHOTO_SESSION_WINDOW_H
#define GUI_PHOTO_SESSION_WINDOW_H

#include <QMainWindow>
#include <vector>
#include "app_state.h"

class QTableWidget;
class QTreeWidget;
class QTreeWidgetItem;
class QLabel;

class PhotoSessionWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit PhotoSessionWindow(AppState& state, QWidget* parent = nullptr);

public slots:
    void refreshAll();

private slots:
    void onLoad();
    void onSave();
    void onAdd();
    void onRemove();
    void onSearch();
    void onFilterByVisitor();
    void onFilterByDate();
    void onShowAll();

private:
    AppState& state_;
    QTableWidget* table_;     // записи справочника (массив)
    QTreeWidget* treeView_;   // окно отладки: АВЛ-дерево
    QLabel* status_;

    void showIndexes(const std::vector<int>& indexes);
    void refreshDebugView();
    void addTreeNode(QTreeWidgetItem* parent, const AVLNode* node, const QString& side);
};

#endif // GUI_PHOTO_SESSION_WINDOW_H
