// gui/main.cpp
// Вёрстка 3: два отдельных окна — «ХТ» (Персонажи) слева и «Дерево» (Фотосессии)
// справа, у каждого своя панель инструментов. Оба окна работают с одним AppState,
// поэтому отчёт видит оба справочника без третьего окна.

#include <QApplication>
#include <QScreen>
#include <QInputDialog>
#include "app_state.h"
#include "character_window.h"
#include "photo_session_window.h"

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    // Начальный размер ХТ задаётся при запуске (позже — кнопкой «Размер ХТ»)
    bool ok = false;
    int capacity = QInputDialog::getInt(nullptr, QStringLiteral("Диснейленд"),
                                        QStringLiteral("Начальный размер хеш-таблицы:"), 20, 1, 100000, 1, &ok);
    if (!ok) return 0;

    AppState state(capacity);
    CharacterWindow characterWindow(state);
    PhotoSessionWindow sessionWindow(state);

    // изменения в «Персонажах» могут затронуть «Фотосессии» — обновляем второе окно
    QObject::connect(&characterWindow, &CharacterWindow::dataChanged,
                     &sessionWindow, &PhotoSessionWindow::refreshAll);

    // два окна рядом: левая и правая половины экрана
    QRect screen = app.primaryScreen()->availableGeometry();
    int w = screen.width() / 2 - 30;
    int h = screen.height() - 120;
    characterWindow.setGeometry(screen.x() + 15, screen.y() + 40, w, h);
    sessionWindow.setGeometry(screen.x() + screen.width() / 2 + 15, screen.y() + 40, w, h);

    characterWindow.show();
    sessionWindow.show();
    return app.exec();
}
