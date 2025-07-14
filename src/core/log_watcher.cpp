#include <QTextStream>
#include <QTextCodec>

#include "config_manager.h" 
#include "log_watcher.h"

LogWatcher::LogWatcher(QObject* parent)
    : QObject(parent)
    , timer_(new QTimer(this))
{
    ConfigManager& config = ConfigManager::instance();
    // 設定読み込み
    filePath_ = config.get("FilePath").toString();
    encoding_ = config.get("Encoding").toString();
    updateInterval_ = config.get("LogUpdateInterval").toInt();
}

LogWatcher::~LogWatcher() {
    if (file_.isOpen()) {
        file_.close();
    }
}

void LogWatcher::start() {
    connect(timer_, &QTimer::timeout, this, &LogWatcher::check);
    timer_->start(updateInterval_);
}

void LogWatcher::reopen(const QString& path) {
    if (!QFile::exists(path)) return;
    if (file_.isOpen()) file_.close();

    file_.setFileName(path);
    file_.open(QIODevice::ReadOnly | QIODevice::Text);
    
    if (!file_.isOpen()) return;

    size_ = file_.size();
}

void LogWatcher::check() {
    if (!file_.isOpen()) {
        reopen(filePath_);
        pos_ = size_;
    }
    if (file_.size() < size_) {
        // ローテーション検知
        reopen(filePath_);
        pos_ = 0;
        file_.seek(0);
    } else {
        file_.seek(pos_);
    }

    QByteArray rawData = file_.readAll();
    if (rawData.isEmpty()) return;

    size_ = file_.size();
    pos_ = file_.pos();

    QTextCodec* codec = QTextCodec::codecForName(encoding_.toUtf8());
    QString logText = codec ? codec->toUnicode(rawData) : QString::fromUtf8(rawData);

    // 複数行対応
    QStringList lines = logText.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
    for (const QString& line : lines) {
        emit newLogLine(line.trimmed());
    }
}