// core/avl_tree.h
// АВЛ-дерево для Справочника_2 (Фотосессии). Ключ — CompositeKey
// (Имя персонажа + Франшиза + Костюмер), НЕ уникален. Значение — индекс записи в массиве
// справочника. Записи с одинаковым ключом — цепочка индексов в одном узле.
// Сами записи в дереве НЕ хранятся.

#ifndef CORE_AVL_TREE_H
#define CORE_AVL_TREE_H

#include <string>
#include <functional>
#include <iostream>
#include "key.h"

// ---------- Элемент цепочки индексов (записи с одинаковым ключом) ----------
struct IndexNode {
    int arrayIndex;   // индекс записи в исходном массиве
    IndexNode* next;  // следующий элемент цепочки
};

// ---------- Узел АВЛ-дерева ----------
struct AVLNode {
    CompositeKey key;     // Имя персонажа + Франшиза + Костюмер
    IndexNode* indexes;   // голова цепочки индексов с этим ключом
    int indexCount;       // длина цепочки
    AVLNode* left;
    AVLNode* right;
    int bal;              // флаг балансировки (по Вирту): −1 перевес влево, 0 баланс, +1 перевес вправо
};

// 1. Инициализация пустого дерева.
AVLNode* createEmptyTree();

// 2. Добавление пары (ключ, индекс). Новый ключ — новый узел с балансировкой;
//    существующий ключ — индекс дописывается в конец цепочки узла.
AVLNode* insertIndex(AVLNode* root, const CompositeKey& key, int arrayIndex, bool& inserted);

// 3. Удаление пары (ключ, индекс). Если цепочка опустела — узел удаляется из
//    дерева; узел с двумя потомками заменяется минимальным узлом правого поддерева.
AVLNode* removeIndex(AVLNode* root, const CompositeKey& key, int arrayIndex, bool& removed);

// 4. Поиск узла по ключу. steps — количество просмотренных узлов.
const AVLNode* searchByKey(const AVLNode* root, const CompositeKey& key, int& steps);
const AVLNode* searchByKey(const AVLNode* root, const CompositeKey& key);

// 5. Печать структуры дерева (отступы, баланс, цепочки индексов).
void printTreeStructure(const AVLNode* root, std::ostream& os = std::cout, int depth = 0,
                        const std::string& prefix = "Root: ");

// 6. Обход слева направо (по возрастанию ключа: имя, затем франшиза); visit(ключ, индекс в массиве).
void inorderTraversal(const AVLNode* root, const std::function<void(const CompositeKey&, int)>& visit);

// 7. Освобождение памяти.
void freeTree(AVLNode* root);

// ---------- Служебные функции ----------
int nodeHeight(const AVLNode* node);      // высота дерева без рекурсии — спуск по bal
int balanceFactor(const AVLNode* node);   // bal узла
int countNodes(const AVLNode* root);    // количество узлов (уникальных ключей)
int countIndexes(const AVLNode* root);  // общее количество индексов во всех цепочках

#endif // CORE_AVL_TREE_H
