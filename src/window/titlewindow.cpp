#include <QApplication>
#include <QFileDialog>

#include "config_manager.h"
#include "window_manager.h"
#include "widget_binding.h"

#include "titlewindow.h"
#include "ui_titlewindow.h"
#include "keywordwindow.h"
#include "settingwindow.h"

TitleWindow::TitleWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TitleWindow)
{
    ui->setupUi(this);  

    ValueBinding::applyConfigToWidgets(this->findChildren<QWidget*>());
}

TitleWindow::~TitleWindow()
{
    delete ui;
}

void TitleWindow::on_btnFileSetting_clicked()
{
    // モーダルなファイル選択を開く
    QString fileName = QFileDialog::getOpenFileName(
        this,
        tr("ログファイルを選択"),
        QString(),  
        tr("ログファイル (*.log);;テキストファイル (*.txt);;すべてのファイル (*)")
    );

    if (!fileName.isEmpty()) {
        ui->inputFilePath->setText(fileName);
    }
}

void TitleWindow::on_btnKeywordSetting_clicked()
{
    // モーダルなキーワード設定ウィンドウを開く
    KeywordWindow keywordWin(this); 
    keywordWin.setWindowModality(Qt::WindowModal); 
    keywordWin.exec(); 
}

void TitleWindow::on_btnStart_clicked()
{
    // 適用処理を行う
    ValueBinding::applyWidgetSettingsToConfig(this->findChildren<QWidget*>());
    ConfigManager::instance().save();

    // チャットウィンドウを開いて、タイトル画面を閉じる
    WindowManager::instance().openChatWindow();
    this->close(); 
}

void TitleWindow::on_btnSetting_clicked()
{
    // モーダルな設定ウィンドウを開く
    SettingWindow settingWin(this); 
    settingWin.setWindowModality(Qt::WindowModal); 
    settingWin.exec(); 
}

void TitleWindow::on_btnExit_clicked()
{
    QApplication::quit(); 
}