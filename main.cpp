#include <QApplication>

#include "config_manager.h"
#include "window_manager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    bool skip_title = ConfigManager::instance().get("SkipTitle").toBool();
    if (skip_title) {
        WindowManager::instance().openChatWindow();
    } else {
        WindowManager::instance().openTitleWindow();
    }

    return app.exec();
}