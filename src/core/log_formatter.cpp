#include <QColor>
#include <QDateTime>
#include <QMap>
#include <QRegularExpression>
#include <QStringList>

#include "config_manager.h"
#include "log_formatter.h"
#include "record_manager.h"

LogFormatter::LogFormatter() {
    applyConfig();
    initialize(RuleModels::instance().get());
}

QString LogFormatter::formatLog(const QString& log) {
    // チャットプレフィックス検出
    int chatIndex = log.indexOf(chatPrefix_);
    if (chatIndex == -1) return QString(); 

    // チャット本文抽出
    QString chat = log.mid(chatIndex + chatPrefix_.length()).trimmed();

    // 追加機能 メッセージ
    QString extraMessage = RecordManager::instance().processLogLine(chat);
    
    // Block / Exclude フィルター
    bool filteredOut = false;
    if (block_) {
        bool found = std::any_of(blackSet_.begin(), blackSet_.end(), [&](const QString& w) {
            return chat.contains(w, Qt::CaseInsensitive);
        });
        filteredOut = found;
    }
    if (!filteredOut && exclude_) {
        bool found = std::any_of(whiteSet_.begin(), whiteSet_.end(), [&](const QString& w) {
            return chat.contains(w, Qt::CaseInsensitive);
        });
        filteredOut = !found;
    }

    for (const RuleEntry& entry : ruleEntries_) {
        if (entry.matcher(chat)) {
            chat = entry.replacer(chat);
        }
    }
    
    if (!filteredOut) {
        // タイムスタンプ抽出
        static const QRegularExpression timestampRegex(R"(\[(\d{2}):(\d{2}):(\d{2})\])");
        QRegularExpressionMatch timeMatch = timestampRegex.match(log);
        QString timeStampStr;
        if (timeMatch.hasMatch()) {
            QString hour = timeMatch.captured(1);
            QString min  = timeMatch.captured(2);
            QString sec  = timeMatch.captured(3);
            if (showTimestamp_ == "時：分") {
                timeStampStr = QString("[%1:%2] ").arg(hour).arg(min);
            } else if (showTimestamp_ == "時：分：秒") {
                timeStampStr = QString("[%1:%2:%3] ").arg(hour).arg(min).arg(sec);
            }
        }
        chat = timeStampStr + chat;
    }
    
    QString htmlChat = (filteredOut || chat.isEmpty()) ? QString() : mcTextToHtml(chat);
    QString htmlExtra = extraMessage.isEmpty() ? QString() : mcTextToHtml(extraMessage);

    if (!htmlChat.isEmpty() && !htmlExtra.isEmpty()) {
        return htmlChat + "<br>" + htmlExtra;
    } else if (!htmlChat.isEmpty()) {
        return htmlChat;
    } else if (!htmlExtra.isEmpty()) {
        return htmlExtra;
    } else {
        return QString();
    }
}

void LogFormatter::applyConfig() {
    ConfigManager &config = ConfigManager::instance();

    block_ = config.get("Block").toBool();
    exclude_ = config.get("Exclude").toBool();
    chatPrefix_ = config.get("ChatPrefix").toString(); 
    showTimestamp_ = config.get("ShowTimestamp").toString(); 

    QVariantList rawBlackList = config.get("BlackList").toList();
    QVariantList rawWhiteList = config.get("WhiteList").toList();

    for (const QVariant& item : rawBlackList) {
        blackSet_.insert(item.toString());
    }

    for (const QVariant& item : rawWhiteList) {
        whiteSet_.insert(item.toString());
    }
}

QString LogFormatter::mcTextToHtml(const QString& text) {
    // 色コード定義（Minecraft仕様）
    static const QMap<QChar, QColor> colorMap = {
        { '0', QColor(0, 0, 0) },
        { '1', QColor(0, 0, 170) },
        { '2', QColor(0, 170, 0) },
        { '3', QColor(0, 170, 170) },
        { '4', QColor(170, 0, 0) },
        { '5', QColor(170, 0, 170) },
        { '6', QColor(255, 170, 0) },
        { '7', QColor(170, 170, 170) },
        { '8', QColor(85, 85, 85) },
        { '9', QColor(85, 85, 255) },
        { 'a', QColor(85, 255, 85) },
        { 'b', QColor(85, 255, 255) },
        { 'c', QColor(255, 85, 85) },
        { 'd', QColor(255, 85, 255) },
        { 'e', QColor(255, 255, 85) },
        { 'f', QColor(255, 255, 255) }
    };

    // 状態変数
    QColor currentColor = QColor(255, 255, 255);
    bool bold = false, italic = false, underline = false, strikethrough = false, obfuscated = false;

    QString html;
    QString buffer;

    // テキストのバッファをHTMLに変換してflush
    auto flushBuffer = [&]() {
        if (buffer.isEmpty()) return;

        QStringList styles;
        // color
        styles << QString("color: rgba(%1, %2, %3, 1.0);")
                      .arg(currentColor.red())
                      .arg(currentColor.green())
                      .arg(currentColor.blue());
        
        // text-decoration
        if (underline || strikethrough) {
            QStringList decorations;
            if (underline) decorations << "underline";
            if (strikethrough) decorations << "line-through";
            styles << QString("text-decoration: %1;").arg(decorations.join(' '));
        }

        QString tag = QString("<span style=\"%1\">").arg(styles.join(' '));
        html += tag;

        if (bold) html += "<b>";
        if (italic) html += "<i>";
        html += buffer;
        if (italic) html += "</i>";
        if (bold) html += "</b>";
        html += "</span>";

        buffer.clear();
    };

    // パース処理
    int i = 0;
    while (i < text.length()) {
        if (text[i] == QChar(0x00A7) && i + 1 < text.length()) {
            flushBuffer();
            QChar code = text[i + 1].toLower();
            i += 2;

            if (colorMap.contains(code)) {
                currentColor = colorMap[code];
                bold = italic = underline = strikethrough = obfuscated = false;
            } else if (code == 'l') {
                bold = true;
            } else if (code == 'o') {
                italic = true;
            } else if (code == 'n') {
                underline = true;
            } else if (code == 'm') {
                strikethrough = true;
            } else if (code == 'k') {
                obfuscated = true;
            } else if (code == 'r') {
                currentColor = QColor(255, 255, 255);
                bold = italic = underline = strikethrough = obfuscated = false;
            }
        } else {
            if (text[i] == '\\' && i + 1 < text.length() && text[i + 1] == 'n') {
                buffer += "<br>";
                    ++i; // 'n'スキップ
            } else if (text[i] == '\n') {
                buffer += "<br>";
            } else if (text[i] == ' ') {
                buffer += "&nbsp;";
            } else {
                buffer += obfuscated ? "*" : QString(text[i]).toHtmlEscaped();
            }
            ++i;
        }
    }

    flushBuffer();
    return html;
}

void LogFormatter::initialize(const RuleConfig& config) {
    ruleEntries_.clear();

    QVector<RuleGroup> groups;
    for (const auto& group : config.Groups) {
        if (group.Enabled) groups.append(group);
    }
    std::sort(groups.begin(), groups.end(), [](const RuleGroup& a, const RuleGroup& b) {
        return a.Priority > b.Priority;
    });

    for (const auto& group : groups) {
        for (const auto& rule : group.Rules) {
            if (!rule.Enabled) continue;

            RuleEntry entry;
            ParsedValue pv = parseValueTemplate(rule.ReplaceText);
            if (rule.MatchMode == "Regex") {
                initializeRegexRule(rule, entry, pv);
            }else if (rule.MatchMode == "Filters") {
                initializeFilterRule(rule, entry, pv);
            }

            ruleEntries_.append(entry);
        }
    }
}

void LogFormatter::initializeRegexRule(const Rule& rule, RuleEntry& entry, ParsedValue& pv)
{
    auto sharedRe = QSharedPointer<QRegularExpression>::create(rule.Match.Regex);

    if (rule.Match.ReplaceMode == "full") {
        entry.matcher = [sharedRe](const QString& message) {
            return sharedRe->match(message).hasMatch();
        };
        entry.replacer = [pv](const QString& message) {
            return LogFormatter::applyParsedValue(pv, message);
        };
    } else if (rule.Match.ReplaceMode == "partial") {
        entry.matcher = [sharedRe](const QString& message) {
            return sharedRe->match(message).hasMatch();
        };
        entry.replacer = [sharedRe, pv](const QString& message) {
            auto match = sharedRe->match(message);
            if (match.hasMatch()) {
                QString replaced = message;
                QString replacement = LogFormatter::applyParsedValue(pv, message);
                replaced.replace(match.capturedStart(), match.capturedLength(), replacement);
                return replaced;
            }
            return message;
        };
    }
}

void LogFormatter::initializeFilterRule(const Rule& rule, RuleEntry& entry, ParsedValue& pv)
{
    QList<Filter> filters = rule.Match.Filters;
    QVector<std::function<bool(const QString&)>> filterFuncs;

    for (const auto& filter : filters) {
        const QStringList& keywords = filter.Keywords;
        const QString& type_ = filter.Type;

        if (type_ == "StartsWithAny") {
            filterFuncs.append([keywords](const QString& msg) {
                for (const auto& kw : keywords)
                    if (msg.startsWith(kw)) return true;
                return false;
            });
        } else if (type_ == "NotStartsWithAny") {
            filterFuncs.append([keywords](const QString& msg) {
                for (const auto& kw : keywords)
                    if (msg.startsWith(kw)) return false;
                return true;
            });
        } else if (type_ == "EndsWithAny") {
            filterFuncs.append([keywords](const QString& msg) {
                for (const auto& kw : keywords)
                    if (msg.endsWith(kw)) return true;
                return false;
            });
        } else if (type_ == "NotEndsWithAny") {
            filterFuncs.append([keywords](const QString& msg) {
                for (const auto& kw : keywords)
                    if (msg.endsWith(kw)) return false;
                return true;
            });
        } else if (type_ == "ContainsAny") {
            filterFuncs.append([keywords](const QString& msg) {
                for (const auto& kw : keywords)
                    if (msg.contains(kw)) return true;
                return false;
            });
        } else if (type_ == "NotContainsAny") {
            filterFuncs.append([keywords](const QString& msg) {
                for (const auto& kw : keywords)
                    if (!msg.contains(kw)) return true;
                return false;
            });
        } else if (type_ == "ContainsAll") {
            filterFuncs.append([keywords](const QString& msg) {
                for (const auto& kw : keywords)
                    if (!msg.contains(kw)) return false;
                return true;
            });
        } else if (type_ == "NotContainsAll") {
            filterFuncs.append([keywords](const QString& msg) {
                for (const auto& kw : keywords)
                    if (msg.contains(kw)) return false;
                return true;
            });
        } else {
            qWarning() << "Unknown ConditionType:" << type_;
            filterFuncs.append([](const QString&) { return true; });
        }
    }

    entry.matcher = [filterFuncs](const QString& msg) {
        for (const auto& f : filterFuncs)
            if (!f(msg)) return false;
        return true;
    };

    entry.replacer = [pv](const QString& message) {
        return LogFormatter::applyParsedValue(pv, message);
    };
}

ParsedValue LogFormatter::parseValueTemplate(const QString& value) {
    ParsedValue result;
    QRegularExpression re(R"(\{m\(([-\d]*):([-\d]*)\)\})"); // {m(...:...)} を検知
    QRegularExpressionMatchIterator it = re.globalMatch(value);

    int lastIndex = 0;
    int firstMatchStart = -1;
    bool first = true;

    while (it.hasNext()) {
        auto match = it.next();

        // 最初に出現したのが {m(...:...)} なら false
        if (first) {
            firstMatchStart = match.capturedStart();
            result.startsWithLiteral = (firstMatchStart != 0);
            first = false;
        }

        int startIndex = match.capturedStart();
        int endIndex = match.capturedEnd();

        QString literal = value.mid(lastIndex, startIndex - lastIndex);
        result.literals.append(literal);

        QString fromStr = match.captured(1);
        QString toStr = match.captured(2);

        int from = fromStr.isEmpty() ? 0 : fromStr.toInt();
        std::optional<int> to = toStr.isEmpty() ? std::nullopt : std::make_optional(toStr.toInt());
        result.ranges.append({ from, to });

        lastIndex = endIndex;
    }

    if (lastIndex < value.length()) {
        result.literals.append(value.mid(lastIndex));
    }
    
    return result;
}

QString LogFormatter::applyParsedValue(const ParsedValue& pv, const QString& m) {
    QString result;
    int li = 0;
    int ri = 0;
    int msgLen = static_cast<int>(m.length());

    // 先に文字列がくる場合
    if (pv.startsWithLiteral && li < pv.literals.size()) {
        result += pv.literals[li++];
    }

    while (li < pv.literals.size() || ri < pv.ranges.size()) {
        if (ri < pv.ranges.size()) {
            int from = pv.ranges[ri].first;
            std::optional<int> toOpt = pv.ranges[ri].second;
            int to = toOpt ? toOpt.value() : msgLen;

            // 負のインデックス補正
            if (from < 0) from += msgLen;
            if (to < 0) to += msgLen;
            // 範囲補正
            from = std::clamp(from, 0, msgLen);
            to = std::clamp(to, 0, msgLen);

            result += m.mid(from, to - from);

            ++ri;

        }
        if (li < pv.literals.size()) {
            result += pv.literals[li++];
        }
    }

    return result;
}
