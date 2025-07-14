#include <QColorDialog>
#include <QMessageBox>

#include "config_manager.h"
#include "widget_binding.h"

#include "settingwindow.h"
#include "ui_settingwindow.h"
#include "rulewindow.h"
#include "timeoutwindow.h"

SettingWindow::SettingWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SettingWindow)
{
    ui->setupUi(this);
    ui->inputBgAlpha->setButtonSymbols(QAbstractSpinBox::NoButtons);
    ui->inputBgAlpha->setReadOnly(true);   
    ui->inputBgColorCode->setReadOnly(true); 

    ValueBinding::applyConfigToWidgets(this->findChildren<QWidget*>());
    updateColorSample(
        ui->lblBgColorSample, 
        ui->inputBgColorCode->text(), 
        ui->inputBgAlpha->value()
    );
}

SettingWindow::~SettingWindow()
{
    delete ui;
}

void SettingWindow::updateColorSample(QLabel *label, const QString &colorCode, int alpha)
{
    QColor color(colorCode);
    color.setAlpha(alpha);
    QString style = QString("background-color: rgba(%1, %2, %3, %4);")
                        .arg(color.red())
                        .arg(color.green())
                        .arg(color.blue())
                        .arg(alpha);
    label->setStyleSheet(style);
}

void SettingWindow::on_btnBgColorSetting_clicked()
{
    // モーダルなカラー選択ウィンドウを開く 
    // 現在のカラーコードと透明度を取得
    QString currentColorCode = ui->inputBgColorCode->text();
    int currentAlpha = ui->inputBgAlpha->value();

    // カラーコードから QColor を生成（失敗時はデフォルト色）
    QColor initialColor(currentColorCode.isEmpty() ? "#ffffff" : currentColorCode);
    initialColor.setAlpha(currentAlpha);

    // カラーダイアログの表示（透明度も含める）
    QColorDialog dialog(initialColor, this);
    dialog.setOption(QColorDialog::ShowAlphaChannel, true);
    dialog.setWindowTitle("背景色を選択");

    if (dialog.exec() == QDialog::Accepted) {
        QColor selectedColor = dialog.selectedColor();
        QString colorCode = selectedColor.name(QColor::HexRgb);  // "#rrggbb"
        int alpha = selectedColor.alpha();

        // 選択結果をウィジェットに反映
        ui->inputBgColorCode->setText(colorCode);
        ui->inputBgAlpha->setValue(alpha);

        // 背景サンプルラベルに反映
        updateColorSample(ui->lblBgColorSample, colorCode, alpha);
    }
}

void SettingWindow::on_btnTimeoutSetting_clicked()
{
    // モーダルな個別タイムアウト設定ウィンドウを開く
    TimeoutWindow timeoutWin(this); 
    timeoutWin.setWindowModality(Qt::WindowModal); 
    timeoutWin.exec(); 
}

void SettingWindow::on_btnRuleSetting_clicked()
{
    // モーダルなランクスタイル設定ウィンドウを開く
    RuleWindow ruleWin(this); 
    ruleWin.setWindowModality(Qt::WindowModal); 
    ruleWin.exec(); 
}

void SettingWindow::on_btnDefault_clicked()
{
    // 確認ダイアログの表示
    const QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("確認"),
        tr("このタブの設定をデフォルトに戻しますか？"),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return;
    }

    ConfigManager &config = ConfigManager::instance();
    const QJsonObject &defaults = config.defaultConfig();

    QWidget *currentTab = ui->tabWidget->currentWidget();
    if (!currentTab) return;

    ValueBinding::applyDefaultsToWidgets(currentTab->findChildren<QWidget*>());
    updateColorSample(
        ui->lblBgColorSample, 
        ui->inputBgColorCode->text(), 
        ui->inputBgAlpha->value()
    );
}

void SettingWindow::on_btnApply_clicked()
{
    ValueBinding::applyWidgetSettingsToConfig(this->findChildren<QWidget*>());
    ConfigManager::instance().save();
    this->close();   
}

void SettingWindow::on_btnCancel_clicked()
{
    this->close();
}
