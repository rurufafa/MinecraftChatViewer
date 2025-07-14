#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <QObject>

class ChatWindow;
class TitleWindow;

class WindowManager : public QObject {
    Q_OBJECT
public:
    static WindowManager& instance();

    void openChatWindow();
    void openTitleWindow();

private:
    explicit WindowManager(QObject *parent = nullptr);
    ~WindowManager();

    ChatWindow *chatWindow_ = nullptr;
    TitleWindow *titleWindow_ = nullptr;
};

#endif // WINDOWMANAGER_H
