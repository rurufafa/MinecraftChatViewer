#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>

#include "config_manager.h"
#include "widget_binding.h"

#include "keywordwindow.h"
#include "ui_keywordwindow.h"


KeywordWindow::KeywordWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::KeywordWindow)
{
    ui->setupUi(this);
    
    ListBinding::loadListWidgetFromConfig(ui->blackList, "BlackList");
    ListBinding::loadListWidgetFromConfig(ui->whiteList, "WhiteList");
}

KeywordWindow::~KeywordWindow()
{
    delete ui;
}

// 共通関数：リストに追加
void KeywordWindow::addItemToList(QLineEdit* lineEdit, QListWidget* listWidget) {
    QString text = lineEdit->text().trimmed();
    if (text.isEmpty()) return;

    // 重複チェック（完全一致）
    for (int i = 0; i < listWidget->count(); ++i) {
        if (listWidget->item(i)->text() == text) {
            QMessageBox::warning(this, tr("重複エラー"), tr("同じ単語がすでに存在します。"));
            return; 
        }
    }

    listWidget->addItem(text);
    lineEdit->clear();
}

// 共通関数：選択行を削除
void KeywordWindow::deleteSelectedItem(QListWidget* listWidget) {
    QList<QListWidgetItem*> selectedItems = listWidget->selectedItems();
    for (QListWidgetItem* item : selectedItems) {
        delete item;
    }
}

// 共通関数：リストをクリア
void KeywordWindow::clearList(QListWidget* listWidget) {
    // 確認メッセージを表示
    const QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("確認"),
        tr("すべての項目を削除しますか？"),
        QMessageBox::Yes | QMessageBox::No
    );

    if (reply != QMessageBox::Yes) {
        return; 
    }

    listWidget->clear();
}


void KeywordWindow::on_btnBlackAdd_clicked(){
    addItemToList(ui->leBlackInput, ui->blackList);
}

void KeywordWindow::on_btnWhiteAdd_clicked(){
    addItemToList(ui->leWhiteInput, ui->whiteList);
}

void KeywordWindow::on_btnBlackDelete_clicked(){
    deleteSelectedItem(ui->blackList);
}

void KeywordWindow::on_btnWhiteDelete_clicked(){
    deleteSelectedItem(ui->whiteList);
}

void KeywordWindow::on_btnBlackClear_clicked(){
    clearList(ui->blackList);
}

void KeywordWindow::on_btnWhiteClear_clicked(){
    clearList(ui->whiteList);
}

void KeywordWindow::on_btnImport_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("インポート"), "", tr("テキストファイル (*.txt)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("エラー"), tr("ファイルを開けませんでした。"));
        return;
    }

    ui->blackList->clear();
    ui->whiteList->clear();

    QTextStream in(&file);
    QListWidget* currentList = nullptr;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line == "---ブラックリスト---") {
            currentList = ui->blackList;
        } else if (line == "---ホワイトリスト---") {
            currentList = ui->whiteList;
        } else if (!line.isEmpty() && currentList) {
            currentList->addItem(line);
        }
    }

    file.close();
}

void KeywordWindow::on_btnExport_clicked()
{
    QString defaultName = "Keyword_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".txt";
    QString fileName = QFileDialog::getSaveFileName(this, tr("エクスポート"), defaultName, tr("テキストファイル (*.txt)"));
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("エラー"), tr("ファイルに書き込めませんでした。"));
        return;
    }

    QTextStream out(&file);

    // ブラックリスト
    out << "---ブラックリスト---\n";
    for (int i = 0; i < ui->blackList->count(); ++i) {
        out << ui->blackList->item(i)->text() << "\n";
    }

    // ホワイトリスト
    out << "---ホワイトリスト---\n";
    for (int i = 0; i < ui->whiteList->count(); ++i) {
        out << ui->whiteList->item(i)->text() << "\n";
    }

    file.close();
}

void KeywordWindow::on_btnApply_clicked()
{
    // 適用処理
    ListBinding::saveListWidgetToConfig(ui->blackList,"BlackList");
    ListBinding::saveListWidgetToConfig(ui->whiteList,"WhiteList");
    ConfigManager::instance().save();
    this->close();
}

void KeywordWindow::on_btnCancel_clicked()
{
    this->close();
}
