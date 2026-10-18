// gui/character_window.cpp
#include "character_window.h"
#include "gui_common.h"

#include <QToolBar>
#include <QAction>
#include <QStyle>
#include <QTableWidget>
#include <QHeaderView>
#include <QAbstractItemView>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QSplitter>
#include <QLabel>
#include <QStatusBar>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QLineEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <sstream>

using gui::q;

CharacterWindow::CharacterWindow(AppState& state, QWidget* parent)
    : QMainWindow(parent), state_(state) {
    setWindowTitle(QStringLiteral("ХТ — Справочник «Персонажи»"));

    // ---------- панель инструментов (ToolStrip) ----------
    QToolBar* tb = addToolBar(QStringLiteral("Операции"));
    tb->setMovable(false);
    tb->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    auto add = [&](QStyle::StandardPixmap icon, const QString& text, void (CharacterWindow::*slot)()) {
        QAction* a = tb->addAction(style()->standardIcon(icon), text);
        connect(a, &QAction::triggered, this, slot);
    };
    add(QStyle::SP_DialogOpenButton, QStringLiteral("Загрузить"), &CharacterWindow::onLoad);
    add(QStyle::SP_DialogSaveButton, QStringLiteral("Сохранить"), &CharacterWindow::onSave);
    tb->addSeparator();
    add(QStyle::SP_FileIcon, QStringLiteral("Добавить"), &CharacterWindow::onAdd);
    add(QStyle::SP_TrashIcon, QStringLiteral("Удалить"), &CharacterWindow::onRemove);
    tb->addSeparator();
    add(QStyle::SP_FileDialogContentsView, QStringLiteral("Поиск"), &CharacterWindow::onSearch);
    add(QStyle::SP_FileDialogListView, QStringLiteral("Фильтр: Костюмер"), &CharacterWindow::onFilter);
    add(QStyle::SP_BrowserReload, QStringLiteral("Показать все"), &CharacterWindow::onShowAll);
    tb->addSeparator();
    add(QStyle::SP_FileDialogDetailedView, QStringLiteral("Размер ХТ"), &CharacterWindow::onSetCapacity);
    add(QStyle::SP_FileDialogInfoView, QStringLiteral("Отчёт"), &CharacterWindow::onReport);

    // ---------- справочник ----------
    table_ = new QTableWidget(this);
    table_->setColumnCount(4);
    table_->setHorizontalHeaderLabels({QStringLiteral("Индекс"), QStringLiteral("Имя"),
                                       QStringLiteral("Костюмер"), QStringLiteral("Франшиза")});
    table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_->verticalHeader()->setVisible(false);
    table_->horizontalHeader()->setStretchLastSection(true);

    QGroupBox* dataBox = new QGroupBox(QStringLiteral("Справочник «Персонажи» (массив)"), this);
    QVBoxLayout* dl = new QVBoxLayout(dataBox);
    dl->addWidget(table_);

    // ---------- окно отладки: хеш-таблица ----------
    hashView_ = new QTableWidget(this);
    hashView_->setColumnCount(6);
    hashView_->setHorizontalHeaderLabels({QStringLiteral("Слот"), QStringLiteral("Статус"),
                                          QStringLiteral("h1"), QStringLiteral("h2"),
                                          QStringLiteral("Ключ (Имя; Франшиза; Костюмер)"),
                                          QStringLiteral("Индекс в массиве")});
    hashView_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    hashView_->verticalHeader()->setVisible(false);
    hashView_->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch);

    QGroupBox* debugBox = new QGroupBox(QStringLiteral("Окно отладки: хеш-таблица"), this);
    QVBoxLayout* hl = new QVBoxLayout(debugBox);
    hl->addWidget(hashView_);

    QSplitter* split = new QSplitter(Qt::Vertical, this);
    split->addWidget(dataBox);
    split->addWidget(debugBox);
    setCentralWidget(split);

    status_ = new QLabel(this);
    statusBar()->addWidget(status_);

    refreshAll();
}

// ---------- отображение ----------

void CharacterWindow::showRows(const std::vector<Character>& items, const std::vector<int>& indexes) {
    table_->setRowCount(static_cast<int>(items.size()));
    for (int r = 0; r < static_cast<int>(items.size()); ++r) {
        const Character& c = items[r];
        QString idx = r < static_cast<int>(indexes.size()) ? QString::number(indexes[r]) : QString();
        table_->setItem(r, 0, new QTableWidgetItem(idx));
        table_->setItem(r, 1, new QTableWidgetItem(q(c.name)));
        table_->setItem(r, 2, new QTableWidgetItem(q(c.costumer)));
        table_->setItem(r, 3, new QTableWidgetItem(q(c.franchise)));
    }
    table_->resizeColumnsToContents();
}

void CharacterWindow::refreshDebugView() {
    const HashTable& t = state_.characters().table();
    hashView_->setRowCount(t.getCapacity());
    static const char* names[] = {"0 (свободен)", "1 (занят)", "2 (удалён)"};
    for (int i = 0; i < t.getCapacity(); ++i) {
        const HashSlot& s = t.slotAt(i);
        bool busy = s.status == SLOT_OCCUPIED;
        hashView_->setItem(i, 0, new QTableWidgetItem(QString::number(i)));
        hashView_->setItem(i, 1, new QTableWidgetItem(QString::fromUtf8(names[s.status])));
        hashView_->setItem(i, 2, new QTableWidgetItem(busy ? QString::number(t.primaryHash(s.key)) : QString()));
        hashView_->setItem(i, 3, new QTableWidgetItem(busy ? QString::number(t.secondaryHash(s.key)) : QString()));
        hashView_->setItem(i, 4, new QTableWidgetItem(busy ? q(s.key.name + "; " + s.key.franchise + "; " + s.key.costumer) : QString()));
        hashView_->setItem(i, 5, new QTableWidgetItem(busy ? QString::number(s.arrayIndex) : QString()));
    }
    hashView_->resizeColumnsToContents();
    status_->setText(QStringLiteral("Записей: %1   Размер ХТ: %2   Заполнение: %3 %")
                         .arg(state_.characters().activeCount()).arg(t.getCapacity())
                         .arg(100.0 * t.getCount() / t.getCapacity(), 0, 'f', 0));
}

void CharacterWindow::refreshAll() {
    const CharacterRepository& r = state_.characters();
    std::vector<Character> items;
    std::vector<int> indexes;
    for (int i = 0; i < r.size(); ++i) {
        if (!r.isActive(i)) continue;
        items.push_back(r.at(i));
        indexes.push_back(i);
    }
    showRows(items, indexes);
    refreshDebugView();
}

void CharacterWindow::onShowAll() { refreshAll(); }

// ---------- файлы ----------

void CharacterWindow::onLoad() {
    QString path = QFileDialog::getOpenFileName(this, QStringLiteral("Загрузить персонажей"), QString(),
                                                QStringLiteral("Текстовые файлы (*.txt);;Все файлы (*)"));
    if (path.isEmpty()) return;
    std::string text;
    if (!gui::readTextFile(path, text)) {
        QMessageBox::warning(this, QStringLiteral("Загрузка"), QStringLiteral("Не удалось открыть файл:\n") + path);
        return;
    }
    std::istringstream in(text);
    LoadStats s = state_.characters().loadFromStream(in);
    refreshAll();
    emit dataChanged();
    QMessageBox::information(this, QStringLiteral("Загрузка"), gui::loadStatsText(s));
}

void CharacterWindow::onSave() {
    QString path = QFileDialog::getSaveFileName(this, QStringLiteral("Сохранить персонажей"), QString(),
                                                QStringLiteral("Текстовые файлы (*.txt)"));
    if (path.isEmpty()) return;
    std::ostringstream out;
    state_.characters().saveToStream(out);
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

static bool askCharacter(QWidget* parent, const QString& title, Character& c) {
    QString name, costumer, franchise;
    if (!askText(parent, title, QStringLiteral("Имя:"), name)) return false;
    if (!askText(parent, title, QStringLiteral("Костюмер:"), costumer)) return false;
    if (!askText(parent, title, QStringLiteral("Франшиза:"), franchise)) return false;
    c = Character{name.toStdString(), costumer.toStdString(), franchise.toStdString()};
    return true;
}

void CharacterWindow::onAdd() {
    const QString title = QStringLiteral("Добавление персонажа");
    Character c;
    if (!askCharacter(this, title, c)) return;
    bool validData, duplicateKey, overflow;
    if (state_.characters().add(c, validData, duplicateKey, overflow)) {
        refreshAll();
        emit dataChanged();
        QMessageBox::information(this, title, QStringLiteral("Персонаж добавлен."));
    } else if (!validData) {
        QMessageBox::warning(this, title, QStringLiteral("Некорректные данные: каждое поле должно быть непустым, без символа «;» и не длиннее 50 символов."));
    } else if (duplicateKey) {
        QMessageBox::warning(this, title, QStringLiteral("Персонаж с такими Именем, Франшизой и Костюмером уже есть."));
    } else if (overflow) {
        QMessageBox::warning(this, title, QStringLiteral("Хеш-таблица заполнена — увеличьте размер (кнопка «Размер ХТ»)."));
    }
}

void CharacterWindow::onRemove() {
    const QString title = QStringLiteral("Удаление персонажа");
    Character c;
    if (!askCharacter(this, title, c)) return;
    bool hasSessions = false;
    if (state_.removeCharacter(c, hasSessions)) {
        refreshAll();
        emit dataChanged();
        QMessageBox::information(this, title, QStringLiteral("Персонаж удалён."));
    } else if (hasSessions) {
        QMessageBox::warning(this, title,
            QStringLiteral("У персонажа есть фотосессии в справочнике «Фотосессии» — сначала удалите их."));
    } else {
        QMessageBox::warning(this, title, QStringLiteral("Запись с такими значениями всех трёх полей не найдена."));
    }
}

// ---------- поиск и фильтр ----------

void CharacterWindow::onSearch() {
    const QString title = QStringLiteral("Поиск персонажа");
    QString name, franchise, costumer;
    if (!askText(this, title, QStringLiteral("Имя:"), name)) return;
    if (!askText(this, title, QStringLiteral("Франшиза:"), franchise)) return;
    if (!askText(this, title, QStringLiteral("Костюмер:"), costumer)) return;
    CompositeKey key{name.toStdString(), franchise.toStdString(), costumer.toStdString()};
    int steps = 0;
    int idx = state_.characters().table().search(key, steps);   // поиск в ХТ
    if (idx == -1) {
        QMessageBox::information(this, title, QStringLiteral("Не найдено.\nШагов поиска в ХТ: %1").arg(steps));
        return;
    }
    showRows({state_.characters().at(idx)}, {idx});
    QMessageBox::information(this, title, QStringLiteral("Найдено (индекс в массиве %1).\nШагов поиска в ХТ: %2")
                                              .arg(idx).arg(steps));
}

void CharacterWindow::onFilter() {
    const QString title = QStringLiteral("Фильтр по костюмеру");
    QString costumer;
    if (!askText(this, title, QStringLiteral("Костюмер:"), costumer) || costumer.isEmpty()) return;
    const CharacterRepository& r = state_.characters();
    std::vector<Character> items;
    std::vector<int> indexes;
    for (int i = 0; i < r.size(); ++i) {
        if (r.isActive(i) && r.at(i).costumer == costumer.toStdString()) {
            items.push_back(r.at(i));
            indexes.push_back(i);
        }
    }
    showRows(items, indexes);
    status_->setText(QStringLiteral("Фильтр «%1»: найдено %2").arg(costumer).arg(static_cast<int>(items.size())));
}

// ---------- размер ХТ ----------

void CharacterWindow::onSetCapacity() {
    const QString title = QStringLiteral("Размер хеш-таблицы");
    bool ok = false;
    int cap = QInputDialog::getInt(this, title, QStringLiteral("Новый размер ХТ:"),
                                   state_.characters().table().getCapacity(), 1, 100000, 1, &ok);
    if (!ok) return;
    if (!state_.characters().setCapacity(cap)) {
        QMessageBox::warning(this, title, QStringLiteral("Размер меньше числа записей (%1) — таблица не изменена.")
                                              .arg(state_.characters().activeCount()));
        return;
    }
    refreshAll();
}

// ---------- отчёт ----------

void CharacterWindow::onReport() {
    const QString title = QStringLiteral("Отчёт");
    const QString hint = QStringLiteral("\n(пусто — любое значение)");
    QString costumer, visitor, dateText;
    if (!askText(this, title, QStringLiteral("Костюмер (справочник «Персонажи»):") + hint, costumer)) return;
    if (!askText(this, title, QStringLiteral("Посетитель (справочник «Фотосессии»):") + hint, visitor)) return;
    if (!askText(this, title, QStringLiteral("Дата (справочник «Фотосессии»), например 12 июн 2025:") + hint, dateText))
        return;

    ReportFilter filter;
    filter.costumer = costumer.toStdString();
    filter.visitor = visitor.toStdString();
    if (!dateText.isEmpty()) {
        if (!parseDate(dateText.toStdString(), filter.date) || !isValidDate(filter.date)) {
            QMessageBox::warning(this, title, QStringLiteral("Неверная дата. Формат: ДД мес ГГГГ, например 12 июн 2025."));
            return;
        }
        filter.useDate = true;
    }

    Report report = state_.buildReport(filter);
    std::ostringstream oss;
    printReport(report, oss);

    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.resize(800, 420);
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QTableWidget* view = new QTableWidget(static_cast<int>(report.rows.size()), 6, &dialog);
    view->setHorizontalHeaderLabels({QStringLiteral("Имя"), QStringLiteral("Костюмер"), QStringLiteral("Франшиза"),
                                     QStringLiteral("Посетитель"), QStringLiteral("Дата"), QStringLiteral("Стоимость")});
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->verticalHeader()->setVisible(false);
    for (int r = 0; r < static_cast<int>(report.rows.size()); ++r) {
        const ReportRow& row = report.rows[r];
        view->setItem(r, 0, new QTableWidgetItem(q(row.character.name)));
        view->setItem(r, 1, new QTableWidgetItem(q(row.character.costumer)));
        view->setItem(r, 2, new QTableWidgetItem(q(row.character.franchise)));
        view->setItem(r, 3, new QTableWidgetItem(q(row.session.visitor)));
        view->setItem(r, 4, new QTableWidgetItem(q(dateToString(row.session.date))));
        view->setItem(r, 5, new QTableWidgetItem(QString::number(row.session.cost, 'f', 2)));
    }
    view->resizeColumnsToContents();
    view->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(view);
    layout->addWidget(new QLabel(QStringLiteral("Строк: %1. Персонажей по фильтру: %2. Шагов поиска в АВЛ-дереве: %3.")
                                     .arg(static_cast<int>(report.rows.size())).arg(report.charactersMatched).arg(report.treeSteps),
                                 &dialog));
    QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close, &dialog);
    layout->addWidget(buttons);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        QString path = QFileDialog::getSaveFileName(&dialog, QStringLiteral("Сохранить отчёт"), QString(),
                                                    QStringLiteral("Текстовые файлы (*.txt)"));
        if (path.isEmpty()) return;
        bool ok = gui::writeTextFile(path, oss.str());
        QMessageBox::information(&dialog, title, ok ? QStringLiteral("Отчёт сохранён.")
                                                    : QStringLiteral("Не удалось записать файл."));
    });
    dialog.exec();
}
