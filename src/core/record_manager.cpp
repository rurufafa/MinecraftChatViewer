#include <QCoreApplication>
#include <QDate> 
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTime>
#include <QJsonDocument>
#include <QJsonArray>
#include <QCoreApplication>
#include <QClipboard>
#include <QGuiApplication>
#include <QRegularExpression>

#include "config_manager.h"
#include "record_manager.h"

RecordManager::RecordManager() {
    QDir dir(QCoreApplication::applicationDirPath());
    path_ = dir.filePath("config/record.json");

    load();
}

RecordManager& RecordManager::instance() {
    static RecordManager instance;
    return instance;
}

void RecordManager::load() {
    QFile file(path_);
    if (!file.open(QIODevice::ReadOnly)) {
        record_ = QJsonObject();
        save();
        return;
    }
    QByteArray jsonData = file.readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);
    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        record_ = QJsonObject();
        save();
        return;
    }

    record_ = doc.object();
    file.close();
}

void RecordManager::save() {
    QJsonDocument doc(record_);
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

QString RecordManager::processLogLine(const QString& line) {
    QStringList resultMessages;
    
    // 採掘数ログ検出
    static QRegularExpression mineExp(R"(Broke §2(\d{1,10})§3 blocks\.)");
    QRegularExpressionMatch m = mineExp.match(line);
    if (m.hasMatch()) {
        if (ConfigManager::instance().get("MiningRecord").toBool()) {
            int blocks = m.captured(1).toInt();
            updateMiningRecord(blocks);
            resultMessages << getMiningStatusMessage(blocks);
        }
        if (ConfigManager::instance().get("PosRecord").toBool()) {
            QString posMsg = getLastPosMessage();
            if (!posMsg.isEmpty()) resultMessages << posMsg;
        }
    }

    // 座標ログ検出
    if (line == "［デバッグ］： クリップボードに座標をコピーしました" &&
        ConfigManager::instance().get("PosRecord").toBool()) {
        QClipboard* cb = QGuiApplication::clipboard();
        QString text = cb->text();
        static QRegularExpression coordExp(R"(/execute in minecraft:(\w+)\s+run tp @s\s+([\-\d.]+)\s+([\-\d.]+)\s+([\-\d.]+))");
        QRegularExpressionMatch m = coordExp.match(text);
        if (m.hasMatch()) {
            QString world = m.captured(1);
            int x = static_cast<int>(m.captured(2).toDouble());
            int y = static_cast<int>(m.captured(3).toDouble());
            int z = static_cast<int>(m.captured(4).toDouble());
            updatePosRecord(world, x, y, z);
            resultMessages << QString("[§ePosRecord§f] 記録しました ワールド: %1 座標: (%2, %3, %4)")
                .arg(world).arg(x).arg(y).arg(z);
        }
    }

    return resultMessages.join("\n");
}

void RecordManager::updateMiningRecord(int blocks) {
    QString date = QDate::currentDate().toString("yyyy-MM-dd");
    if (!record_.contains("mining") || !record_.value("mining").isObject()) record_["mining"] = QJsonObject();
    QJsonObject mining = record_.value("mining").toObject(); 
    if (!mining.contains(date) || !mining.value(date).isObject()) mining[date] = QJsonObject();

    QJsonObject todayRecord = mining.value(date).toObject();

    if (!todayRecord.contains("initial")) {
        todayRecord["initial"] = blocks;
        todayRecord["history"] = QJsonArray{ QJsonArray{ QTime::currentTime().toString("HH:mm:ss"), blocks } };
    } else {
        QJsonArray history = todayRecord["history"].toArray();
        QJsonArray entry = QJsonArray{ QTime::currentTime().toString("HH:mm:ss"), blocks };
        history.append(entry);
        todayRecord["history"] = history;
    }

    mining[date] = todayRecord;
    record_["mining"] = mining;
    save();
}

QString RecordManager::getMiningStatusMessage(int current) {
    QString date = QDate::currentDate().toString("yyyy-MM-dd");

    if (!record_.contains("mining") || !record_["mining"].isObject()) {
        return "[§dMiningRecord§f] 採掘データが存在しません。";
    }

    QJsonObject mining = record_["mining"].toObject();
    if (!mining.contains(date) || !mining[date].isObject()) {
        return "[§dMiningRecord§f] 今日の採掘データが存在しません。";
    }

    QJsonObject todayRecord = mining[date].toObject();
    int initial = todayRecord["initial"].toInt();
    int delta = current - initial;
    int target = ConfigManager::instance().get("MiningTarget").toInt();

    QStringList resultLines;

    // 今日の採掘数（これは常に表示）
    resultLines << QString("[§dMiningRecord§f] 今日の採掘数 : %1").arg(delta);

    // 目標と残り採掘数（target が正の場合のみ）
    if (target > 0) {
        int remain = qMax(0, target - delta);
        int rate = static_cast<int>((static_cast<double>(delta) / target) * 100);

        QString color;
        if (rate >= 100) color = "§a";
        else if (rate >= 75) color = "§9";
        else if (rate >= 50) color = "§e";
        else if (rate >= 25) color = "§6";
        else color = "§c";

        resultLines.prepend(QString("[§dMiningRecord§f] 目標採掘数 : %1").arg(target));
        resultLines << QString("[§dMiningRecord§f] %1§l残りの採掘数 : %2 (%3%)§r")
            .arg(color)
            .arg(remain)
            .arg(rate);
    }

    // 採掘ペース計算
    if (todayRecord.contains("history") && todayRecord["history"].isArray()) {
        QJsonArray history = todayRecord["history"].toArray();

        int intervalMinutes = ConfigManager::instance().get("MiningPaceInterval").toInt();
        QTime now = QTime::currentTime();
        QTime cutoff = now.addSecs(-intervalMinutes * 60);

        // 後ろから interval 分以上前の最新記録を探す
        for (int i = history.size() - 1; i >= 0; --i) {
            QJsonArray entry = history[i].toArray();
            if (entry.size() != 2) continue;

            QTime time = QTime::fromString(entry[0].toString(), "HH:mm:ss");
            if (!time.isValid()) continue;

            if (time <= cutoff) {
                int prev = entry[1].toInt();
                int diff = current - prev;
                int elapsedSecs = time.secsTo(now);
                if (elapsedSecs > 0) {
                    double pacePerHour = diff * 3600.0 / elapsedSecs;
                    QString paceText, paceColor;
                    if (pacePerHour >= 10000.0) {
                        paceText = QString::asprintf("%.1f万", pacePerHour / 10000.0);
                        paceColor = "§a§l";
                    } else {
                        paceText = QString("%1").arg(static_cast<int>(pacePerHour), 4, 10, QLatin1Char(' '));
                        paceColor = "§7§l";
                    }

                    resultLines << QString("[§dMiningRecord§f] %1%2 block/h (直近%3分の記録)§r")
                        .arg(paceColor)
                        .arg(paceText)
                        .arg(intervalMinutes);
                }
                break;
            }
        }
    }

    return resultLines.join("\n");
}

void RecordManager::updatePosRecord(const QString& world, int x, int y, int z) {
    QJsonObject pos;
    pos["world"] = world;
    pos["x"] = x;
    pos["y"] = y;
    pos["z"] = z;

    record_["position"] = pos; 
    save();
}

QString RecordManager::getLastPosMessage() const {
    QJsonObject pos = record_.value("position").toObject();
    if (pos.isEmpty()) return "";

    return QString("[§ePosRecord§f] §lワールド: %1 座標: (%2, %3, %4)")
        .arg(pos["world"].toString())
        .arg(pos["x"].toInt())
        .arg(pos["y"].toInt())
        .arg(pos["z"].toInt());
}
