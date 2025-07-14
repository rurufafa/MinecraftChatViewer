#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#include "rule_models.h"

namespace {
    template <typename T>
    bool existsSameName(const QList<T>& roots, const T& target) {
        return std::any_of(roots.begin(), roots.end(), [&](const T& r) {
            return r.Name == target.Name && !(r == target);
        });
    }
}

RuleModels::RuleModels() {
    QDir dir(QCoreApplication::applicationDirPath());
    path_ = dir.filePath("config/rule.json");

    load();
    rebuildIndex();
}

RuleModels& RuleModels::instance() {
    static RuleModels instance;
    return instance;
}

void RuleModels::load() {
    QFile file(path_);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open file:" << path_;
        return;
    }

    QByteArray jsonData = file.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "Invalid JSON format.";
        return;
    }

    config_ = RuleConfig::fromJson(doc.object());
    file.close();
}

void RuleModels::save() {
    QJsonDocument doc(config_.toJson());
    QFile file(path_);

    QFileInfo fileInfo(file);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) return;
    }
    
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}

void RuleModels::rebuildIndex() {
    groupMap_.clear();
    ruleMap_.clear();

    for (const RuleGroup &group : config_.Groups) {
        groupMap_[group.Id] = group;
        for (const Rule &rule : group.Rules) {
            QString key = group.Id + "::" + rule.Id;
            ruleMap_[key] = rule;
        }
    }
}

// groupIDからグループ取得
std::optional<RuleGroup> RuleModels::getGroup(const QString &groupId) const {
    if (groupMap_.contains(groupId)) {
        return groupMap_.value(groupId);
    }
    return std::nullopt;
}

// groupID + ruleID からルール取得
std::optional<Rule> RuleModels::getRule(const QString &groupId, const QString &ruleId) const {
    QString key = groupId + "::" + ruleId;
    if (ruleMap_.contains(key)) {
        return ruleMap_.value(key);
    }
    return std::nullopt;
}

bool RuleModels::addGroup(const RuleGroup& group) {
    auto& groups = config_.Groups;

    // 名前重複チェック
    if (existsSameName(groups, group)) return false;

    groups.append(group);

    rebuildIndex();
    save();
    return true;
}

bool RuleModels::editGroup(const QString& groupId, const RuleGroup& updatedGroup) {
    auto& groups = config_.Groups;

    // 名前重複チェック
    if (existsSameName(groups, updatedGroup)) return false;

    // 対象groupのイテレータを取得
    auto it = std::find_if(groups.begin(), groups.end(),
                             [&](const RuleGroup& g) { return g.Id == groupId; });
    if (it == groups.end()) return false;

    *it = updatedGroup;

    rebuildIndex();
    save();
    return true;
}

bool RuleModels::removeGroup(const QString& groupId) {
    auto& groups = config_.Groups;

    // 対象groupのイテレータを取得
    auto it = std::remove_if(groups.begin(), groups.end(),
                             [&](const RuleGroup& g) { return g.Id == groupId; });
    if (it == groups.end()) return false;
    
    groups.erase(it, groups.end());

    rebuildIndex();
    save();
    return true;
}

bool RuleModels::addRule(const QString& parentGroupId, const Rule& rule) {
    auto& groups = config_.Groups;

    // parentGroupのイテレータを取得
    auto it = std::find_if(groups.begin(), groups.end(),
                           [&](const RuleGroup& g) { return g.Id == parentGroupId; });
    if (it == groups.end()) return false;
    auto& rules = it->Rules;

    // 名前重複チェック
    if (existsSameName(rules, rule)) return false;

    rules.append(rule);

    rebuildIndex();
    save();
    return true;
}

bool RuleModels::editRule(const QString& parentGroupId, const QString& ruleId, const Rule& updatedRule) {
    auto& groups = config_.Groups;

    // parentGroupのイテレータを取得
    auto it = std::find_if(groups.begin(), groups.end(),
                           [&](const RuleGroup& g) { return g.Id == parentGroupId; });
    if (it == groups.end()) return false;
    auto& rules = it->Rules;

    // 名前重複チェック
    if (existsSameName(rules, updatedRule)) return false;

    // 対象ruleのイテレータを取得
    auto rit = std::find_if(rules.begin(), rules.end(),
                            [&](const Rule& r) { return r.Id == ruleId; });
    if (rit == rules.end()) return false;
    *rit = updatedRule;

    rebuildIndex();
    save();
    return true;
}

bool RuleModels::removeRule(const QString& parentGroupId, const QString& ruleId) {
    // parentGroupのイテレータを取得
    auto it = std::find_if(config_.Groups.begin(), config_.Groups.end(),
                           [&](const RuleGroup& g) { return g.Id == parentGroupId; });
    if (it == config_.Groups.end()) return false;
    auto& rules = it->Rules;

    // 対象ruleのイテレータを取得
    auto rit = std::remove_if(rules.begin(), rules.end(),
                              [&](const Rule& r) { return r.Id == ruleId; });
    if (rit == rules.end()) return false;
    rules.erase(rit, rules.end());

    rebuildIndex();    
    save();
    return true;
}

RuleConfig RuleModels::importRuleConfig(const QString &importFilePath)
{
    QFile file(importFilePath);
    RuleConfig config;

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "ファイルを開けません:" << importFilePath;
        return config;
    }

    QTextStream in(&file);
    RuleGroup currentGroup;
    Rule currentRule;
    bool inMatchBlock = false;
    Filter currentFilter;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.startsWith("# Group:")) {
            if (inMatchBlock && !currentFilter.Type.isEmpty() && !currentFilter.Keywords.isEmpty()) {
                currentRule.Match.Filters.append(currentFilter);
                currentFilter = Filter();
                inMatchBlock = false;
            }

            if (!currentRule.Name.isEmpty())
                currentGroup.Rules.append(currentRule);

            if (!currentGroup.Name.isEmpty())
                    config.Groups.append(currentGroup);

            currentGroup = RuleGroup();
            currentGroup.Name = line.mid(QString("# Group:").length()).trimmed();
            currentGroup.Enabled = true;
            currentGroup.Id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            currentGroup.Rules.clear();
            currentRule = Rule(); 
        }
        else if (line.startsWith("Description:")) {
            currentGroup.Description = line.mid(QString("Description:").length()).trimmed();
        }
        else if (line.startsWith("Priority:")) {
            currentGroup.Priority = line.mid(QString("Priority:").length()).trimmed().toInt();
        }
        else if (line.startsWith("# Rule:")) {
            if (inMatchBlock && !currentFilter.Type.isEmpty() && !currentFilter.Keywords.isEmpty()) {
                currentRule.Match.Filters.append(currentFilter);
                currentFilter = Filter();
                inMatchBlock = false;
            }

            if (!currentRule.Name.isEmpty()) {
                currentGroup.Rules.append(currentRule);
            }

            currentRule = Rule();
            currentRule.Enabled = true;
            currentRule.Id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            currentRule.Name = line.mid(QString("# Rule:").length()).trimmed();
            currentRule.Match = Match();
            currentRule.MatchMode = "Filters";
            currentRule.ReplaceText = "";
        }
        else if (line.startsWith("MatchMode:")) {
            currentRule.MatchMode = line.mid(QString("MatchMode:").length()).trimmed();
        }
        else if (line.startsWith("ReplaceText:")) {
            currentRule.ReplaceText = line.mid(QString("ReplaceText:").length()).trimmed();
        }
        else if (line == "<Match>") {
            inMatchBlock = true;
            currentRule.Match = Match();
            currentRule.Match.Filters.clear();
            currentFilter = Filter();
        }
        else if (inMatchBlock) {
            if (line.startsWith("Regex:")) {
                currentRule.Match.Regex = line.mid(QString("Regex:").length()).trimmed();
            }
            else if (line.startsWith("ReplaceMode:")) {
                currentRule.Match.ReplaceMode = line.mid(QString("ReplaceMode:").length()).trimmed();
            }
            else if (line.startsWith("Type:")) {
                if (!currentFilter.Keywords.isEmpty()) {
                    currentRule.Match.Filters.append(currentFilter);
                    currentFilter = Filter();
                }
                currentFilter.Type = line.mid(QString("Type:").length()).trimmed();
            }
            else if (line.startsWith("Keywords:")) {
                // 無視（次行以降に複数行キーワード）
            }
            else if (!line.isEmpty() && !line.startsWith("#")) {
                currentFilter.Keywords.append(line);
            }
        }
    }

    // 残っている最後のフィルター・ルール・グループを追加
    if (inMatchBlock && !currentFilter.Type.isEmpty() && !currentFilter.Keywords.isEmpty()) {
        currentRule.Match.Filters.append(currentFilter);
        currentFilter = Filter();
        inMatchBlock = false;
    }

    if (!currentRule.Name.isEmpty())
        currentGroup.Rules.append(currentRule);

    if (!currentGroup.Name.isEmpty())
        config.Groups.append(currentGroup);


    config_ = config;
    return config;
}
