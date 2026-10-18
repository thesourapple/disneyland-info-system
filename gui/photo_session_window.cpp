// gui/photo_session_window.cpp
#include "photo_session_window.h"
#include "gui_common.h"

#include <QToolBar>
#include <QAction>
#include <QStyle>
#include <QTableWidget>
#include <QTreeWidget>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QLabel>
#include <QStatusBar>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QLineEdit>
#include <sstream>

using gui::q;

PhotoSessionWindow::PhotoSessionWindow(AppState& state, QWidget* parent)
    : QMainWindow(parent), state_(state) {
    setWindowTitle(QStringLiteral("Дерево — Справочник «Фотосессии»"));

    // ---------- панель инструментов (ToolStrip) ----------
    QToolBar* tb = addToolBar(QStringLiteral("Операции"));
    tb->setMovable(false);
    tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    auto add = [&](QStyle::StandardPixmap icon, const QString& text, void (PhotoSessionWindow::*slot)()) {
        QAction* a = tb->addAction(style()->standardIcon(icon), text);
        connect(a, &QAction::triggered, this, slot);
    };
    add(QStyle::SP_DialogOpenButton, QStringLiteral("Загрузить"), &PhotoSessionWindow::onLoad);
    add(QStyle::SP_DialogSaveButton, QStringLiteral("Сохранить"), &PhotoSessionWindow::onSave);
    tb->addSeparator();
    add(QStyle::SP_FileIcon, QStringLiteral("Добавить"), &PhotoSessionWindow::onAdd);
    add(QStyle::SP_TrashIcon, QStringLiteral("Удалить"), &PhotoSessionWindow::onRemove);
    tb->addSeparator();
    add(QStyle::SP_FileDialogContentsView, QStringLiteral("Поиск"), &PhotoSessionWindow::onSearch);
    add(QStyle::SP_FileDialogListView, QStringLiteral("Фильтр: Посетитель"), &PhotoSessionWindow::onFilterByVisitor);
    add(QStyle::SP_FileDialogListView, QStringLiteral("Фильтр: Дата"), &PhotoSessionWindow::onFilterByDate);
    add(QStyle::SP_BrowserReload, QStringLiteral("Показать все"), &PhotoSessionWindow::onShowAll);

    // ---------- справочник ----------
    table_ = new QTableWidget(this);
    table_->setColumnCount(7);
    table_->setHorizontalHeaderLabels({QStringLiteral("Индекс"), QStringLiteral("Имя персонажа"),
                                       QStringLiteral("Франшиза"), QStringLiteral("Костюмер"), QStringLiteral("Посетитель"),
                                       QStringLiteral("Дата"), QStringLiteral("Стоимость")});
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setStretchLastSection(true);

    QGroupBox* dataBox = new QGroupBox(QStringLiteral("Справочник «Фотосессии» (массив)"), this);
    QVBoxLayout* dl = new QVBoxLayout(dataBox);
    dl->addWidget(table_);

    // ---------- окно отладки: АВЛ-дерево ----------
    treeView_ = new QTreeWidget(this);
    treeView_->setColumnCount(3);
    treeView_->setHeaderLabels({QStringLiteral("Ключ (Имя; Франшиза; Костюмер)"), QStringLiteral("Цепочка индексов"),
                                QStringLiteral("Баланс (bal)")});
    QGroupBox* debugBox = new QGroupBox(QStringLiteral("Окно отладки: АВЛ-дерево (L — левый, R — правый потомок)"), this);
    QVBoxLayout* tl = new QVBoxLayout(debugBox);
    tl->addWidget(treeView_);

    QSplitter* split = new QSplitter(Qt::Vertical, this);
    split->addWidget(dataBox);
    split->addWidget(debugBox);
    setCentralWidget(split);

    status_ = new QLabel(this);
    statusBar()->addWidget(status_);

    refreshAll();
}

// ---------- отображение ----------

void PhotoSessionWindow::showIndexes(const std::vector<int>& indexes) {
    const PhotoSessionRepository& r = state_.sessions();
    table_->setRowCount(static_cast<int>(indexes.size()));
    for (int row = 0; row < static_cast<int>(indexes.size()); ++row) {
        const PhotoSession& p = r.at(indexes[row]);
        table_->setItem(row, 0, new QTableWidgetItem(QString::number(indexes[row])));
        table_->setItem(row, 1, new QTableWidgetItem(q(p.characterName)));
        table_->setItem(row, 2, new QTableWidgetItem(q(p.franchise)));
        table_->setItem(row, 3, new QTableWidgetItem(q(p.costumer)));
        table_->setItem(row, 4, new QTableWidgetItem(q(p.visitor)));
        table_->setItem(row, 5, new QTableWidgetItem(q(dateToString(p.date))));
        table_->setItem(row, 6, new QTableWidgetItem(QString::number(p.cost, 'f', 2)));
    }
    table_->resizeColumnsToContents();
}

void PhotoSessionWindow::addTreeNode(QTreeWidgetItem* parent, const AVLNode* node, const QString& side) {
    if (node == nullptr) return;
    QString chain;
    for (const IndexNode* p = node->indexes; p != nullptr; p = p->next) {
        if (!chain.isEmpty()) chain += QStringLiteral(" → ");
        chain += QString::number(p->arrayIndex);
    }
    QStringList cols{side + q(node->key.name + "; " + node->key.franchise + "; " + node->key.costumer), chain,
                     QString::number(node->bal)};
    QTreeWidgetItem* item = parent ? new QTreeWidgetItem(parent, cols) : new QTreeWidgetItem(treeView_, cols);
    addTreeNode(item, node->left, QStringLiteral("L: "));
    addTreeNode(item, node->right, QStringLiteral("R: "));
}

void PhotoSessionWindow::refreshDebugView() {
    treeView_->clear();
    addTreeNode(nullptr, state_.sessions().tree(), QStringLiteral("Корень: "));
    treeView_->expandAll();
    treeView_->resizeColumnToContents(0);
    status_->setText(QStringLiteral("Записей: %1   Узлов дерева: %2   Высота: %3")
                         .arg(state_.sessions().activeCount())
                         .arg(countNodes(state_.sessions().tree()))
                         .arg(nodeHeight(state_.sessions().tree())));
}

void PhotoSessionWindow::refreshAll() {
    std::vector<int> all;
    for (int i = 0; i < state_.sessions().size(); ++i)
        if (state_.sessions().isActive(i)) all.push_back(i);
    showIndexes(all);
    refreshDebugView();
}

void PhotoSessionWindow::onShowAll() { refreshAll(); }

// ---------- файлы ----------

void PhotoSessionWindow::onLoad() {
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("Загрузить фотосессии"), QString(),
                                                QStringLiteral("Текстовые файлы (*.txt);;Все файлы (*)"));
    if (path.isEmpty()) return;
    std::string text;
    if (!gui::readTextFile(path, text)) {
        QMessageBox::warning(this, QStringLiteral("Загрузка"), QStringLiteral("Не удалось открыть файл:\n") + path);
        return;
    }
    std::istringstream in(text);
    LoadStats s = state_.loadSessions(in);   // с проверкой, что персонаж есть в справочнике 1
    refreshAll();
    QMessageBox::information(this, QStringLiteral("Загрузка"), gui::loadStatsText(s));
}

void PhotoSessionWindow::onSave() {
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Сохранить фотосессии"), QString(),
                                                QStringLiteral("Текстовые файлы (*.txt)"));
    if (path.isEmpty()) return;
    std::ostringstream out;
    state_.sessions().saveToStream(out);
    bool ok = gui::writeTextFile(path, out.str());
    QMessageBox::information(this, QStringLiteral("Сохранение"),
        ok ? QStringLiteral("Справочник сохранён.") : QStringLiteral("Не удалось записать файл."));
}

// ---------- ввод записи ----------

static bool askText(QWidget* parent, const QString& title, const QString& label, QString& out) {
    bool ok = false;
    out = QInputDialog::getText(parent, title, label, QLineEdit::Normal, QString(), &ok).trimmed();
    return ok;
}

static bool askDate(QWidget* parent, const QString& title, Date& out) {
    QString text;
    if (!askText(parent, title, QStringLiteral("Дата (ДД мес ГГГГ, например 12 июн 2025):"), text)) return false;
    if (!parseDate(text.toStdString(), out) || !isValidDate(out)) {
        QMessageBox::warning(parent, title, QStringLiteral("Неверная дата. Формат: ДД мес ГГГГ "
                                                           "(мес — янв фев мар апр май июн июл авг сен окт ноя дек)."));
        return false;
    }
    return true;
}

static bool askSession(QWidget* parent, const QString& title, PhotoSession& p) {
    QString name, franchise, costumer, visitor;
    if (!askText(parent, title, QStringLiteral("Имя персонажа:"), name)) return false;
    if (!askText(parent, title, QStringLiteral("Франшиза:"), franchise)) return false;
    if (!askText(parent, title, QStringLiteral("Костюмер:"), costumer)) return false;
    if (!askText(parent, title, QStringLiteral("Посетитель:"), visitor)) return false;
    Date date{};
    if (!askDate(parent, title, date)) return false;
    bool ok = false;
    double cost = QInputDialog::getDouble(parent, title, QStringLiteral("Стоимость (от 0,01 до 1 000 000):"), 1000.0, 0.01, 1000000.0, 2, &ok);
    if (!ok) return false;
    p = PhotoSession{name.toStdString(), franchise.toStdString(), costumer.toStdString(), visitor.toStdString(), date, cost};
    return true;
}

void PhotoSessionWindow::onAdd() {
    const QString title = QStringLiteral("Добавление фотосессии");
    PhotoSession p;
    if (!askSession(this, title, p)) return;
    bool validData, unknownCharacter;
    if (state_.addSession(p, validData, unknownCharacter)) {
        refreshAll();
        QMessageBox::information(this, title, QStringLiteral("Фотосессия добавлена."));
    } else if (unknownCharacter) {
        QMessageBox::warning(this, title, QStringLiteral("Персонажа с такими Именем, Франшизой и Костюмером нет в справочнике «Персонажи»."));
    } else {
        QMessageBox::warning(this, title, QStringLiteral("Некорректные данные: текстовые поля — непустые, без символа «;», не длиннее 50 символов; "
                                                   "стоимость — больше 0 и не больше 1 000 000."));
    }
}

void PhotoSessionWindow::onRemove() {
    const QString title = QStringLiteral("Удаление фотосессии");
    PhotoSession p;
    if (!askSession(this, title, p)) return;
    if (state_.sessions().remove(p)) {
        refreshAll();
        QMessageBox::information(this, title, QStringLiteral("Фотосессия удалена."));
    } else {
        QMessageBox::warning(this, title, QStringLiteral("Запись с такими значениями всех шести полей не найдена."));
    }
}

// ---------- поиск и фильтры ----------

void PhotoSessionWindow::onSearch() {
    const QString title = QStringLiteral("Поиск фотосессий");
    QString name, franchise, costumer;
    if (!askText(this, title, QStringLiteral("Имя персонажа:"), name)) return;
    if (!askText(this, title, QStringLiteral("Франшиза:"), franchise)) return;
    if (!askText(this, title, QStringLiteral("Костюмер:"), costumer)) return;
    int steps = 0;
    const AVLNode* node = searchByKey(state_.sessions().tree(),
                                      CompositeKey{name.toStdString(), franchise.toStdString(), costumer.toStdString()},
                                      steps);
    std::vector<int> found;
    for (const IndexNode* p = node ? node->indexes : nullptr; p != nullptr; p = p->next)
        found.push_back(p->arrayIndex);
    showIndexes(found);
    QMessageBox::information(this, title, QStringLiteral("Найдено фотосессий: %1\nШагов поиска в АВЛ-дереве: %2")
                                              .arg(static_cast<int>(found.size())).arg(steps));
}

void PhotoSessionWindow::onFilterByVisitor() {
    QString visitor;
    if (!askText(this, QStringLiteral("Фильтр по посетителю"), QStringLiteral("Посетитель:"), visitor) ||
        visitor.isEmpty())
        return;
    std::vector<int> found;
    for (int i = 0; i < state_.sessions().size(); ++i)
        if (state_.sessions().isActive(i) && state_.sessions().at(i).visitor == visitor.toStdString())
            found.push_back(i);
    showIndexes(found);
    status_->setText(QStringLiteral("Фильтр «%1»: найдено %2").arg(visitor).arg(static_cast<int>(found.size())));
}

void PhotoSessionWindow::onFilterByDate() {
    Date d{};
    if (!askDate(this, QStringLiteral("Фильтр по дате"), d)) return;
    std::vector<int> found;
    for (int i = 0; i < state_.sessions().size(); ++i) {
        if (!state_.sessions().isActive(i)) continue;
        const Date& x = state_.sessions().at(i).date;
        if (x.day == d.day && x.month == d.month && x.year == d.year) found.push_back(i);
    }
    showIndexes(found);
    status_->setText(QStringLiteral("Фильтр «%1»: найдено %2").arg(q(dateToString(d))).arg(static_cast<int>(found.size())));
}
