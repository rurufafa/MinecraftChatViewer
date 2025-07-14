#ifndef RECORDMANAGER_H
#define RECORDMANAGER_H

#include <QString>
#include <QJsonObject>

class RecordManager {
public:
    static RecordManager& instance();
    void load();
    void save();

    // ログ処理全体
    QString processLogLine(const QString& line);

    // MiningRecord
    void updateMiningRecord(int blocks);
    QString getMiningStatusMessage(int currentBlocks);

    // PosRecord
    void updatePosRecord(const QString& world, int x, int y, int z);
    QString getLastPosMessage() const;

private:
    RecordManager();
    QJsonObject record_;
    QString path_;
};

#endif // RECORDMANAGER_H