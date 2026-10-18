// core/avl_tree.cpp
#include "avl_tree.h"

// ================= Цепочка индексов =================

static void freeIndexChain(IndexNode* head) {
    while (head != nullptr) {
        IndexNode* next = head->next;
        delete head;
        head = next;
    }
}

// Добавление индекса в конец цепочки, возвращает голову.
static IndexNode* appendIndex(IndexNode* head, int arrayIndex) {
    IndexNode* n = new IndexNode{arrayIndex, nullptr};
    if (head == nullptr) return n;
    IndexNode* cur = head;
    while (cur->next != nullptr) cur = cur->next;
    cur->next = n;
    return head;
}

// Удаление индекса из цепочки, возвращает новую голову; found — был ли найден.
static IndexNode* removeFromChain(IndexNode* head, int arrayIndex, bool& found) {
    found = false;
    IndexNode* prev = nullptr;
    IndexNode* cur = head;
    while (cur != nullptr) {
        if (cur->arrayIndex == arrayIndex) {
            if (prev == nullptr) head = cur->next;
            else prev->next = cur->next;
            delete cur;
            found = true;
            break;
        }
        prev = cur;
        cur = cur->next;
    }
    return head;
}


int balanceFactor(const AVLNode* node) {
    return node == nullptr ? 0 : node->bal;
}

// Высота дерева без рекурсии: от корня идём в более высокое поддерево
// (его указывает bal), считая уровни.
int nodeHeight(const AVLNode* node) {
    int h = 0;
    while (node != nullptr) {
        ++h;
        node = (node->bal < 0) ? node->left : node->right;
    }
    return h;
}

static AVLNode* makeNode(const CompositeKey& key, int arrayIndex) {
    AVLNode* n = new AVLNode();
    n->key = key;
    n->indexes = new IndexNode{arrayIndex, nullptr};
    n->indexCount = 1;
    n->left = nullptr;
    n->right = nullptr;
    n->bal = 0;
    return n;
}

// ================= 1. Инициализация =================

AVLNode* createEmptyTree() {
    return nullptr;
}

// ================= 2. Добавление =================
// h — выросла ли высота поддерева p.

static void insertRec(AVLNode*& p, const CompositeKey& key, int arrayIndex, bool& h) {
    if (p == nullptr) {                       // ключа нет — новый лист, высота выросла
        p = makeNode(key, arrayIndex);
        h = true;
        return;
    }
    if (key < p->key) {
        insertRec(p->left, key, arrayIndex, h);
        if (!h) return;                       // левая ветвь выросла
        if (p->bal == 1)       { p->bal = 0;  h = false; }
        else if (p->bal == 0)  { p->bal = -1; }
        else {                                // bal = −1: балансировка
            AVLNode* p1 = p->left;
            if (p1->bal == -1) {              // однократный LL-поворот
                p->left = p1->right;
                p1->right = p;
                p->bal = 0;
                p = p1;
            } else {                          // двукратный LR-поворот
                AVLNode* p2 = p1->right;
                p1->right = p2->left;
                p2->left = p1;
                p->left = p2->right;
                p2->right = p;
                if (p2->bal == -1) p->bal = 1; else p->bal = 0;
                if (p2->bal == 1) p1->bal = -1; else p1->bal = 0;
                p = p2;
            }
            p->bal = 0;
            h = false;
        }
    } else if (p->key < key) {
        insertRec(p->right, key, arrayIndex, h);
        if (!h) return;                       // правая ветвь выросла
        if (p->bal == -1)      { p->bal = 0; h = false; }
        else if (p->bal == 0)  { p->bal = 1; }
        else {                                // bal = +1: балансировка
            AVLNode* p1 = p->right;
            if (p1->bal == 1) {               // однократный RR-поворот
                p->right = p1->left;
                p1->left = p;
                p->bal = 0;
                p = p1;
            } else {                          // двукратный RL-поворот
                AVLNode* p2 = p1->left;
                p1->left = p2->right;
                p2->right = p1;
                p->right = p2->left;
                p2->left = p;
                if (p2->bal == 1) p->bal = -1; else p->bal = 0;
                if (p2->bal == -1) p1->bal = 1; else p1->bal = 0;
                p = p2;
            }
            p->bal = 0;
            h = false;
        }
    } else {
        // ключ уже есть — индекс в конец цепочки, структура дерева не меняется
        p->indexes = appendIndex(p->indexes, arrayIndex);
        p->indexCount++;
        h = false;
    }
}

AVLNode* insertIndex(AVLNode* root, const CompositeKey& key, int arrayIndex, bool& inserted) {
    bool h = false;
    insertRec(root, key, arrayIndex, h);
    inserted = true;
    return root;
}

// ================= 3. Удаление =================
// h — уменьшилась ли высота поддерева p.

// Уменьшилась левая ветвь узла p.
static void balanceL(AVLNode*& p, bool& h) {
    if (p->bal == -1)     { p->bal = 0; }
    else if (p->bal == 0) { p->bal = 1; h = false; }
    else {                                    // bal = +1: балансировка
        AVLNode* p1 = p->right;
        int b1 = p1->bal;
        if (b1 >= 0) {                        // однократный RR-поворот
            p->right = p1->left;
            p1->left = p;
            if (b1 == 0) { p->bal = 1; p1->bal = -1; h = false; }
            else         { p->bal = 0; p1->bal = 0; }
            p = p1;
        } else {                              // двукратный RL-поворот
            AVLNode* p2 = p1->left;
            int b2 = p2->bal;
            p1->left = p2->right;
            p2->right = p1;
            p->right = p2->left;
            p2->left = p;
            if (b2 == 1) p->bal = -1; else p->bal = 0;
            if (b2 == -1) p1->bal = 1; else p1->bal = 0;
            p = p2;
            p2->bal = 0;
        }
    }
}

// Уменьшилась правая ветвь узла p.
static void balanceR(AVLNode*& p, bool& h) {
    if (p->bal == 1)      { p->bal = 0; }
    else if (p->bal == 0) { p->bal = -1; h = false; }
    else {                                    // bal = −1: балансировка
        AVLNode* p1 = p->left;
        int b1 = p1->bal;
        if (b1 <= 0) {                        // однократный LL-поворот
            p->left = p1->right;
            p1->right = p;
            if (b1 == 0) { p->bal = -1; p1->bal = 1; h = false; }
            else         { p->bal = 0; p1->bal = 0; }
            p = p1;
        } else {                              // двукратный LR-поворот
            AVLNode* p2 = p1->right;
            int b2 = p2->bal;
            p1->right = p2->left;
            p2->left = p1;
            p->left = p2->right;
            p2->right = p;
            if (b2 == -1) p->bal = 1; else p->bal = 0;
            if (b2 == 1) p1->bal = -1; else p1->bal = 0;
            p = p2;
            p2->bal = 0;
        }
    }
}

// Минимальный узел поддерева r переносится в узел q (ключ и цепочка), сам узел удаляется.
static void delMin(AVLNode*& r, AVLNode* q, bool& h) {
    if (r->left != nullptr) {
        delMin(r->left, q, h);
        if (h) balanceL(r, h);
    } else {
        freeIndexChain(q->indexes);
        q->key = r->key;
        q->indexes = r->indexes;
        q->indexCount = r->indexCount;
        AVLNode* t = r;
        r = r->right;
        delete t;
        h = true;
    }
}

// Удаление узла с ключом key из поддерева p.
static void deleteRec(AVLNode*& p, const CompositeKey& key, bool& h) {
    if (p == nullptr) { h = false; return; }
    if (key < p->key) {
        deleteRec(p->left, key, h);
        if (h) balanceL(p, h);
    } else if (p->key < key) {
        deleteRec(p->right, key, h);
        if (h) balanceR(p, h);
    } else {
        AVLNode* q = p;
        if (q->right == nullptr)     { p = q->left;  h = true; freeIndexChain(q->indexes); delete q; }
        else if (q->left == nullptr) { p = q->right; h = true; freeIndexChain(q->indexes); delete q; }
        else {                       // два потомка: замена минимальным узлом правого поддерева
            delMin(q->right, q, h);
            if (h) balanceR(p, h);
        }
    }
}

AVLNode* removeIndex(AVLNode* root, const CompositeKey& key, int arrayIndex, bool& removed) {
    removed = false;
    AVLNode* node = root;                     // поиск узла без рекурсии
    while (node != nullptr && !(node->key == key))
        node = (key < node->key) ? node->left : node->right;
    if (node == nullptr) return root;

    node->indexes = removeFromChain(node->indexes, arrayIndex, removed);
    if (!removed) return root;
    node->indexCount--;
    if (node->indexCount > 0) return root;    // цепочка не пуста — узел остаётся
    bool h = false;
    deleteRec(root, key, h);
    return root;
}

// ================= 4. Поиск =================

const AVLNode* searchByKey(const AVLNode* root, const CompositeKey& key, int& steps) {
    steps = 0;
    const AVLNode* cur = root;
    while (cur != nullptr) {
        ++steps;
        if (key == cur->key) return cur;
        cur = (key < cur->key) ? cur->left : cur->right;
    }
    return nullptr;
}

const AVLNode* searchByKey(const AVLNode* root, const CompositeKey& key) {
    int steps = 0;
    return searchByKey(root, key, steps);
}

// ================= 5. Печать структуры =================

void printTreeStructure(const AVLNode* root, std::ostream& os, int depth, const std::string& prefix) {
    std::string indent(depth * 4, ' ');
    if (root == nullptr) {
        os << indent << prefix << "(пусто)\n";
        return;
    }
    os << indent << prefix << "(" << root->key.name << "; " << root->key.franchise << "; " << root->key.costumer << ")"
       << "  [bal=" << root->bal
       << "]  индексы:";
    for (IndexNode* p = root->indexes; p != nullptr; p = p->next) {
        os << " " << p->arrayIndex;
        if (p->next != nullptr) os << " ->";
    }
    os << "\n";
    if (root->left != nullptr || root->right != nullptr) {
        printTreeStructure(root->left, os, depth + 1, "L-- ");
        printTreeStructure(root->right, os, depth + 1, "R-- ");
    }
}

// ================= 6. Обход слева направо =================

void inorderTraversal(const AVLNode* root, const std::function<void(const CompositeKey&, int)>& visit) {
    if (root == nullptr) return;
    inorderTraversal(root->left, visit);
    for (IndexNode* p = root->indexes; p != nullptr; p = p->next)
        visit(root->key, p->arrayIndex);
    inorderTraversal(root->right, visit);
}

// ================= 7. Освобождение памяти =================

void freeTree(AVLNode* root) {
    if (root == nullptr) return;
    freeTree(root->left);
    freeTree(root->right);
    freeIndexChain(root->indexes);
    delete root;
}

// ================= Подсчёт =================

int countNodes(const AVLNode* root) {
    if (root == nullptr) return 0;
    return 1 + countNodes(root->left) + countNodes(root->right);
}

int countIndexes(const AVLNode* root) {
    if (root == nullptr) return 0;
    return root->indexCount + countIndexes(root->left) + countIndexes(root->right);
}
