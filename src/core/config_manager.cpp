#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QJsonDocument>

#include "config_manager.h"

ConfigManager::ConfigManager()
{
    QDir dir(QCoreApplication::applicationDirPath());
    path_ = dir.filePath("config/config.json");

    default_config_ = QJsonObject{
        // [Basic]
        {"Block", false},
        {"Exclude", false},
        {"FilePath", ""},
        
        // [Drawing]
        {"BgColorCode", "#323232"},
        {"BgAlpha", 50},
        {"FontSize", 20},
        {"MaxChatHeight", 160},
        {"MarginHeight", 10},
        {"Encoding", "cp932"},
        {"AlwaysOnTop", true},
        
        // [Log]
        {"TimeoutSec", 9},
        {"MaxLogLines", 80},
        {"LogUpdateInterval", 100},
        {"FadeInterval", 300},
        {"FadeStep", 0.34},
        {"ChatPrefix", "[System] [CHAT] "},
        
        // [Control]
        {"ToggleViewKey", "R"},
        {"TitleScreenKey", "T"},
        {"SkipTitle", false},
        
        // [Addon]
        {"ShowTimestamp", "非表示"},
        {"PosRecord", true},
        {"MiningRecord", true},
        {"MiningTarget", 0},
        {"MiningPaceInterval", 20},

        // [Keyword]
        {"BlackList", QJsonArray{}},
        {"WhiteList", QJsonArray{}},

        // [Timeout]
        {"TimeoutMap", QJsonObject{}}
    };

    load();  // 起動時にロード
}

ConfigManager& ConfigManager::instance()
{
    static ConfigManager instance;
    return instance;
}

void ConfigManager::load()
{
    QFile file(path_);
    if (!file.open(QIODevice::ReadOnly)) {
        config_ = default_config_;
        save();
        return;
    }

    QByteArray jsonData = file.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        config_ = default_config_;
        save();
        return;
    }

    config_ = doc.object();

    // 欠損キーをデフォルトで補完
    bool modified = false;
    for (auto it = default_config_.begin(); it != default_config_.end(); ++it) {
        if (!config_.contains(it.key())) {
            config_[it.key()] = it.value();
            modified = true;
        }
    }

    if (modified) {
        save();  // 補完があった場合は保存
    }

    file.close();
}

void ConfigManager::save()
{
    QJsonDocument doc(config_);
    QFile file(path_);

    // 親ディレクトリを作成（必要なら）
    QFileInfo fileInfo(file);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        if (!dir.mkpath(".")) return;
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
}

QVariant ConfigManager::get(const QString &key) const
{
    if(!config_.contains(key)) return {};
    return config_.value(key).toVariant();
}

void ConfigManager::set(const QString &key, const QVariant &value)
{
    if(!config_.contains(key)) return;
    config_[key] = QJsonValue::fromVariant(value);
}

const QJsonObject& ConfigManager::defaultConfig() const
{
    return default_config_;
}
