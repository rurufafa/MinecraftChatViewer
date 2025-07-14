#ifndef LOGFORMATTER_H 
#define LOGFORMATTER_H

#include <QPair>
#include <QSet>
#include <QString>
#include <QVector>

#include "record_manager.h"
#include "rule_models.h"

struct RuleEntry {
    std::function<bool(const QString&)> matcher; // 条件合致を判定する関数
    std::function<QString(const QString&)> replacer; // メッセージを置き換える関数
};

struct ParsedValue {
    QStringList literals; // 文字列部分   
    QVector<QPair<int, std::optional<int>>> ranges; // メッセージ範囲指定部分
    bool startsWithLiteral; // 最初が文字列部分か
};

class LogFormatter {
public:
    explicit LogFormatter();
    QString formatLog(const QString& rawLog);
    static void initializeRegexRule(const Rule& rule, RuleEntry& entry, ParsedValue& pv);
    static void initializeFilterRule(const Rule& rule, RuleEntry& entry, ParsedValue& pv);
    static ParsedValue parseValueTemplate(const QString& value);
    static QString applyParsedValue(const ParsedValue& pv, const QString& m);
    
private:
    void applyConfig();
    QString mcTextToHtml(const QString& text);
    void initialize(const RuleConfig& config);

    bool block_, exclude_;
    QString chatPrefix_, showTimestamp_; 
    QSet<QString> blackSet_, whiteSet_;
    QVector<RuleEntry> ruleEntries_;
};

#endif // LOGFORMATTER_H