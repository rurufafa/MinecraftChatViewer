#ifndef SETTINGWINDOW_H
#define SETTINGWINDOW_H

#include <QDialog>
#include <QLabel>

namespace Ui {
    class SettingWindow;
}

class SettingWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SettingWindow(QWidget *parent = nullptr);
    ~SettingWindow();

private slots:
    void on_btnBgColorSetting_clicked();
    void on_btnTimeoutSetting_clicked();
    void on_btnRuleSetting_clicked();
    void on_btnDefault_clicked();
    void on_btnApply_clicked();
    void on_btnCancel_clicked();

private:
    void updateColorSample(QLabel *label, const QString &colorCode, int alpha);
    Ui::SettingWindow *ui;
};

#endif // SETTINGWINDOW_H
