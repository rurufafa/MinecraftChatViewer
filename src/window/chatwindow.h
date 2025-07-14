#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QTimer>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QVariantMap>

#include "log_formatter.h"
#include "log_watcher.h"


class ChatWindow : public QWidget {
    Q_OBJECT

public:
    explicit ChatWindow(QWidget *parent = nullptr);

protected:
    void contextMenuEvent(QContextMenuEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void paintEvent(QPaintEvent *event) override;

private slots:
    void appendLog(const QString& rawLog);
    void fadeStep();

private:
    void applyConfig();
    int getTimeout(const QString &text);
    QString overrideAlpha(const QString& html, float alpha);
    void setViewMode(bool isViewMode);
    void updateDisplay();

    struct LogEntry {
        QString text;
        QDateTime timestamp;
        int timeoutSec;
        float alpha;
    };

    QList<LogEntry> logs_;   
    QList<LogEntry> historyLogs_;  
    QTimer fadeTimer_;
    QVBoxLayout *layout_;
    QTextEdit *logViewer_;

    LogWatcher* logWatcher_;
    LogFormatter* logFormatter_;

    bool alwaysOnTop_;
    QColor bgColor_;
    int bgAlpha_;
    int fadeInterval_;
    float fadeStep_;
    int fontSize_;
    int marginHeight_;
    int maxChatHeight_;
    int maxLogLines_;
    QVariantMap timeoutMap_;
    int timeoutSec_;
    bool scrollRestorePending_ = false;
    bool viewMode_ = false;
};

#endif // CHATWINDOW_H