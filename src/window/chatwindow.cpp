#include <QApplication>
#include <QDateTime>
#include <QContextMenuEvent>
#include <QMenu>
#include <QPainter>
#include <QScrollBar>

#include "config_manager.h"
#include "frameless_helper.h"
#include "log_watcher.h"
#include "log_formatter.h"
#include "window_manager.h"

#include "chatwindow.h"

ChatWindow::ChatWindow(QWidget *parent)
    : QWidget(parent),
      logViewer_(new QTextEdit(this))
{
    this->resize(640, 480);

    // configの設定を適用する
    applyConfig();

    // フレームレスウィンドウのイベント処理
    FramelessHelper *frameless = new FramelessHelper(this); 

    if (alwaysOnTop_) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    }

    // 背景をbgColor_に変更
    bgColor_.setAlpha(bgAlpha_);

    logViewer_->setStyleSheet(QString(
        "background: transparent;"
        "border: none;"
        "font-size: %1pt;"
        "color: white;" 
    ).arg(fontSize_));

    logViewer_->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    logViewer_->setReadOnly(true);
    // logViewer_->setFocusPolicy(Qt::NoFocus);
    logViewer_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    logViewer_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // logViewer_にチャットを配置する
    layout_ = new QVBoxLayout(this);
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->addWidget(logViewer_);

    connect(&fadeTimer_, &QTimer::timeout, this, &ChatWindow::fadeStep); 

    logWatcher_ = new LogWatcher(this);
    connect(logWatcher_, &LogWatcher::newLogLine, this, &ChatWindow::appendLog);
    logWatcher_->start();

    logFormatter_ = new LogFormatter();

    updateDisplay();
}

void ChatWindow::applyConfig() {
    ConfigManager &config = ConfigManager::instance();

    alwaysOnTop_ = config.get("AlwaysOnTop").toBool();
    bgColor_ = QColor(config.get("BgColorCode").toString());
    bgAlpha_ = config.get("BgAlpha").toInt();
    timeoutSec_ = config.get("TimeoutSec").toInt();
    fadeStep_ = config.get("FadeStep").toDouble();
    fadeInterval_ = config.get("FadeInterval").toInt();
    maxLogLines_ = config.get("MaxLogLines").toInt();
    fontSize_ = config.get("FontSize").toInt();
    marginHeight_ = config.get("MarginHeight").toInt();
    maxChatHeight_ = config.get("MaxChatHeight").toInt();
    timeoutMap_ = config.get("TimeoutMap").toMap();
}

int ChatWindow::getTimeout(const QString &text) {
    for (auto it = timeoutMap_.begin(); it != timeoutMap_.end(); ++it) {
        if (text.contains(it.key(), Qt::CaseInsensitive)) {
            return it.value().toInt();
        }
    }
    return timeoutSec_;
}

QString ChatWindow::overrideAlpha(const QString& html, float alpha)
{
    QString result = html;

    QRegularExpression rgbaRegex(R"(rgba\(\s*(\d+),\s*(\d+),\s*(\d+),\s*[\d.]+\s*\))");
    QRegularExpressionMatchIterator it = rgbaRegex.globalMatch(html);

    int offset = 0;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();

        int r = match.captured(1).toInt();
        int g = match.captured(2).toInt();
        int b = match.captured(3).toInt();

        QString newRgba = QString("rgba(%1, %2, %3, %4)")
            .arg(r).arg(g).arg(b).arg(QString::number(alpha, 'f', 2));

        int start = match.capturedStart(0) + offset;
        int length = match.capturedLength(0);

        result.replace(start, length, newRgba);
        offset += newRgba.length() - length;
    }

    return result;
}

void ChatWindow::appendLog(const QString &rawLog) {
    QString formatted = logFormatter_->formatLog(rawLog);
    int timeout = getTimeout(rawLog);

    if (formatted.isEmpty()) return;

    LogEntry log = {formatted, QDateTime::currentDateTime(), timeout, 1.0f};

    // logs_: 表示対象, historyLogs_: 全ログ履歴(viewModeに使う)
    logs_.append(log);
    historyLogs_.append(log);  

    // フェードアウト処理開始
    if (!viewMode_ && !fadeTimer_.isActive()) {
        fadeTimer_.start(fadeInterval_);
    }

    // 古いログは消去
    while (logs_.size() > maxLogLines_) {
        logs_.removeFirst();
    }

    while (historyLogs_.size() > maxLogLines_) {
        historyLogs_.removeFirst();
    }
 
    updateDisplay();
}

void ChatWindow::fadeStep() {
    auto now = QDateTime::currentDateTime();
    QList<LogEntry> updated;
    bool stillFading = false;
    bool changed = false;

    for (const auto &log : logs_) {
        LogEntry updatedLog = log;
        int elapsed = log.timestamp.secsTo(now);

        // タイムアウト時間が過ぎたログはフェードアウトさせる
        if (elapsed > log.timeoutSec) {
            changed = true;
            updatedLog.alpha = log.alpha - fadeStep_;
        }

        // 表示対象のログのみ追加
        if (updatedLog.alpha > 0.0f) {
            stillFading = true;
            updated.append(updatedLog);
        }
    }

    // 表示対象のログが存在しなければフェードアウト処理を停止
    if (!stillFading){
        fadeTimer_.stop();
    }

    // ログに変更があれば再描画
    if (changed) {
        logs_ = std::move(updated);
        updateDisplay();
    }
}

void ChatWindow::updateDisplay() {
    QString html;

    // viewMode_にしたがって表示するログを変更
    const QList<LogEntry> &source = viewMode_ ? historyLogs_ : logs_;

    for (int i = 0; i < source.size(); ++i) {
        const LogEntry &log = source[i];
        QString line = viewMode_ ? log.text : overrideAlpha(log.text, log.alpha);
        html += line;

        if (i < source.size() - 1) {
            html += "<br>"; 
        }
    }

    if (html.isEmpty()) {
        logViewer_->hide();
        bgColor_.setAlpha(0);  // 完全に透明
    } else {
        logViewer_->show();

        QScrollBar* vbar = logViewer_->verticalScrollBar();
        int prevScrollValue = vbar->value();
        bool atBottom = (prevScrollValue == vbar->maximum());

        logViewer_->setHtml(html);

        if (!scrollRestorePending_) {
            scrollRestorePending_ = true;

            QTimer::singleShot(0, this, [=] {
                QScrollBar* vb = logViewer_->verticalScrollBar();
                if (!viewMode_) {
                    // 通常モードは常に最新チャットを追尾
                    vb->setValue(vb->maximum());
                } else {
                    if (atBottom) {
                        // 閲覧モードかつ最下部にいる場合は追尾
                        vb->setValue(vb->maximum());
                    } else {
                        // 閲覧モードかつスクロール途中なら位置を保持
                        vb->setValue(prevScrollValue);
                    }
                }
                scrollRestorePending_ = false;
            });
        }

        bgColor_.setAlpha(bgAlpha_); // 通常透明度に戻す

        if (viewMode_) return;

        // 文書の高さ取得
        auto docHeight = logViewer_->document()->size().height();

        // 最大高さを超えないように制限
        int newHeight = std::min(static_cast<int>(docHeight)+marginHeight_, maxChatHeight_);

        if (newHeight != height()) {
            int bottomY = y() + height();  // 現在の下端位置

            resize(width(), newHeight);
             // 高さ変更後、下端を元の位置に合わせるように y を再設定
            move(x(), bottomY - height());
        }
    }
}

void ChatWindow::setViewMode(bool isViewMode) {
    // viewMode_: フェードアウト処理を停止+スクロール可能
    // !viewMode_: フェードアウト処理を開始+スクロール不可

    viewMode_ = isViewMode;
    if (viewMode_) {
        fadeTimer_.stop();
        logViewer_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        resize(width(), maxChatHeight_);
    } else {
        if (!fadeTimer_.isActive()) {
            fadeTimer_.start(fadeInterval_);
        }
        logViewer_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }

    updateDisplay();
}

void ChatWindow::contextMenuEvent(QContextMenuEvent *event) {
    QMenu menu(this);
    QString viewModeText = viewMode_ ? "閲覧モードを解除する" : "閲覧モードにする";
    menu.addAction(viewModeText, [this] { setViewMode(!viewMode_); });
    menu.addAction("タイトル画面を開く", [this] {
        WindowManager::instance().openTitleWindow();
    });
    menu.addAction("アプリケーションを終了", [] {QApplication::quit(); }); 
    menu.exec(event->globalPos());
}

void ChatWindow::keyPressEvent(QKeyEvent *event) {
    auto toggleKey = ConfigManager::instance().get("ToggleViewKey").toString().toUpper();
    auto titleKey = ConfigManager::instance().get("TitleScreenKey").toString().toUpper();
    if (event->text().toUpper() == toggleKey) {
        setViewMode(!viewMode_);
    } else if (event->text().toUpper() == titleKey) {
        WindowManager::instance().openTitleWindow();
    }
}

void ChatWindow::wheelEvent(QWheelEvent *event) {
    if (viewMode_) {
        QCoreApplication::sendEvent(logViewer_->viewport(), event);
    }
}

void ChatWindow::paintEvent(QPaintEvent *event) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), bgColor_);  // alpha込み
}
