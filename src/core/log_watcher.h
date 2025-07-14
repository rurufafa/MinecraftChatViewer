#ifndef LOGWATCHER_H
#define LOGWATCHER_H

#include <QObject>
#include <QFile>
#include <QTimer>
#include <QString>

class LogWatcher : public QObject {
    Q_OBJECT

public:
    explicit LogWatcher(QObject* parent = nullptr);
    ~LogWatcher();
    void start();

signals:
    void newLogLine(const QString& line);

private slots:
void check();

private:
    void reopen(const QString& path);
    QFile file_;
    QTimer* timer_;
    qint64 pos_ = 0;
    qint64 size_ = 0;
    
    int updateInterval_;
    QString filePath_;
    QString encoding_;
};

#endif // LOGWATCHER_H