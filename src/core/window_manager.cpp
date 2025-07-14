#include "window_manager.h"

#include "chatwindow.h"
#include "titlewindow.h"

WindowManager::WindowManager(QObject *parent) : QObject(parent) {}

WindowManager::~WindowManager() {}

WindowManager& WindowManager::instance() {
    static WindowManager instance;
    return instance;
}

void WindowManager::openChatWindow() {
    if (chatWindow_) {
        // 既存ならclose
        chatWindow_->close();
    }

    chatWindow_ = new ChatWindow();
    chatWindow_->setAttribute(Qt::WA_DeleteOnClose);
    connect(chatWindow_, &QObject::destroyed, this, [this](QObject* obj) {
        // 上書き後の破棄対策
        if (obj == chatWindow_)
            chatWindow_ = nullptr;
    });
    chatWindow_->show();
}

void WindowManager::openTitleWindow() {
    if (!titleWindow_) {
        titleWindow_ = new TitleWindow();
        titleWindow_->setAttribute(Qt::WA_DeleteOnClose);
        connect(titleWindow_, &QObject::destroyed, this, [this] {
            titleWindow_ = nullptr;
        });
        titleWindow_->show();
    } else {
        // 既存なら最上面に
        titleWindow_->raise();
        titleWindow_->activateWindow();
    }
}
