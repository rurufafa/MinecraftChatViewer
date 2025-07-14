#ifndef RULEMODELS_H
#define RULEMODELS_H

#include <QString>
#include <QList>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QUuid>
#include <QHash>

// --- Filter ---
struct Filter {
    QStringList Keywords = QStringList();
    QString Type = "StartsWithAny";

    static Filter fromJson(const QJsonObject& obj) {
        Filter f;
        for (const QJsonValue& kw : obj["Keywords"].toArray()) {
            f.Keywords << kw.toString();
        }
        f.Type = obj["Type"].toString();
        return f;
    }

    QJsonObject toJson() const {
        QJsonArray keywordsArray;
        for (const QString& k : Keywords)
            keywordsArray.append(k);

        return {
            {"Keywords", keywordsArray},
            {"Type", Type}
        };
    }
};

// --- Match ---
struct Match {
    QList<Filter> Filters = QList<Filter>();
    QString Regex = "";
    QString ReplaceMode = "full";

    static Match fromJson(const QJsonObject& obj) {
        Match c;
        if (obj.contains("Filters")) {
            for (const QJsonValue& f : obj["Filters"].toArray()) {
                c.Filters.append(Filter::fromJson(f.toObject()));
            }
        }
        if (obj.contains("Regex")) {
            c.Regex = obj["Regex"].toString();
            c.ReplaceMode = obj["ReplaceMode"].toString();
        }
        return c;
    }

    QJsonObject toJson() const {
        QJsonObject obj;
        if (!Filters.isEmpty()) {
            QJsonArray filterArray;
            for (const Filter& f : Filters)
                filterArray.append(f.toJson());
            obj["Filters"] = filterArray;
        }
        if (!Regex.isEmpty())
            obj["Regex"] = Regex;
            obj["ReplaceMode"] = ReplaceMode;
        return obj;
    }
};

// --- 個別Rule ---
struct Rule {
    QString Id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString Name = "";
    bool Enabled = true;
    QString MatchMode = "Filters";
    QString ReplaceText = "";
    Match Match;

    static Rule fromJson(const QJsonObject& obj) {
        Rule r;
        r.Id = obj["Id"].toString();
        r.Name = obj["Name"].toString();
        r.Enabled = obj["Enabled"].toBool();
        r.MatchMode = obj["MatchMode"].toString();
        r.ReplaceText = obj["ReplaceText"].toString();
        r.Match = Match::fromJson(obj["Match"].toObject());
        return r;
    }

    QJsonObject toJson() const {
        return {
            {"Id", Id},
            {"Name", Name},
            {"Enabled", Enabled},
            {"MatchMode", MatchMode},
            {"ReplaceText", ReplaceText},
            {"Match", Match.toJson()}
        };
    }

    bool operator==(const Rule& other) const {
        return Id == other.Id;
    }
};

// --- グループ ---
struct RuleGroup {
    QString Id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QString Name = "";
    bool Enabled = true;
    int Priority = 0;
    QString Description = "";
    QList<Rule> Rules = QList<Rule>();

    static RuleGroup fromJson(const QJsonObject& obj) {
        RuleGroup g;
        g.Id = obj["Id"].toString();
        g.Name = obj["Name"].toString();
        g.Enabled = obj["Enabled"].toBool();
        g.Priority = obj["Priority"].toInt();
        g.Description = obj["Description"].toString();

        for (const QJsonValue& r : obj["Rules"].toArray()) {
            g.Rules.append(Rule::fromJson(r.toObject()));
        }
        return g;
    }

    QJsonObject toJson() const {
        QJsonArray ruleArray;
        for (const Rule& r : Rules)
            ruleArray.append(r.toJson());

        return {
            {"Id", Id},
            {"Name", Name},
            {"Enabled", Enabled},
            {"Priority", Priority},
            {"Description", Description},
            {"Rules", ruleArray}
        };
    }

    bool operator==(const RuleGroup& other) const {
        return Id == other.Id;
    }
};

// --- 全体構造（ルート） ---
struct RuleConfig {
    QList<RuleGroup> Groups = QList<RuleGroup>();

    static RuleConfig fromJson(const QJsonObject& obj) {
        RuleConfig c;
        for (const QJsonValue& g : obj["Groups"].toArray()) {
            c.Groups.append(RuleGroup::fromJson(g.toObject()));
        }
        return c;
    }

    QJsonObject toJson() const {
        QJsonArray groupArray;
        for (const RuleGroup& g : Groups)
            groupArray.append(g.toJson());
        return {
            {"Groups", groupArray}
        };
    }
};

class RuleModels
{
public:
    static RuleModels& instance();
    RuleConfig& get() { return config_; }
    void set(RuleConfig& newConfig) { config_ = newConfig; }
    void load();
    void save();
    void rebuildIndex();

    std::optional<RuleGroup> getGroup(const QString &groupId) const;
    std::optional<Rule> getRule(const QString &groupId, const QString &ruleId) const;
    RuleConfig importRuleConfig(const QString &importFilePath);

    bool addGroup(const RuleGroup& group);
    bool editGroup(const QString& groupId, const RuleGroup& updatedGroup);
    bool removeGroup(const QString& groupId);

    bool addRule(const QString& parentGroupId, const Rule& rule);
    bool editRule(const QString& parentGroupId, const QString& ruleId, const Rule& updatedRule);
    bool removeRule(const QString& parentGroupId, const QString& ruleId);

private:
    RuleModels();
    QHash<QString, RuleGroup> groupMap_;
    QHash<QString, Rule> ruleMap_;
    RuleConfig config_;
    QString path_;

    RuleModels(const RuleModels&) = delete;
    RuleModels& operator=(const RuleModels&) = delete;
};

Q_DECLARE_METATYPE(RuleGroup)
Q_DECLARE_METATYPE(Rule)

#endif // RULEMODELS_H
