#include <QFileDialog>
#include <QMessageBox>
#include <QTableWidgetItem>
#include <QTextStream>

#include "config_manager.h"
#include "widget_binding.h"

#include "timeoutwindow.h"
#include "ui_timeoutwindow.h"

TimeoutWindow::TimeoutWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TimeoutWindow)
{
    ui->setupUi(this);
    // ヘッダーの幅をウィンドウサイズにフィットさせ、等分配
    QHeaderView *header = ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);  

    TableBinding::loadTableWidgetFromConfig(ui->tableWidget, "TimeoutMap");
}

TimeoutWindow::~TimeoutWindow()
{
    delete ui;
}

void TimeoutWindow::on_btnAdd_clicked() {
    QString key = ui->leKeyInput->text().trimmed();
    QString value = ui->leValueInput->text().trimmed();

    if (key.isEmpty() && value.isEmpty()) return;

    if (key.isEmpty() ^ value.isEmpty()) {
        QMessageBox::warning(this, tr("入力エラー"), tr("キーワードとタイムアウトの両方を入力してください。"));
        return;
    }

    // 整数チェック
    bool ok;
    int timeoutValue = value.toInt(&ok);
    if (!ok || timeoutValue < 0) {
        QMessageBox::warning(this, tr("入力エラー"), tr("タイムアウトには0以上の整数を入力してください。"));
        return;
    }

    // 重複チェック（1列目に同じキーがあるか）
    int rowCount = ui->tableWidget->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        QTableWidgetItem *item = ui->tableWidget->item(i, 0);
        if (item && item->text() == key) {
            QMessageBox::warning(this, tr("重複エラー"), tr("同じキーワードがすでに存在します。"));
            return;
        }
    }

    // 行の追加
    int newRow = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(newRow);
    ui->tableWidget->setItem(newRow, 0, new QTableWidgetItem(key));
    ui->tableWidget->setItem(newRow, 1, new QTableWidgetItem(value));

    // 入力欄をクリア
    ui->leKeyInput->clear();
    ui->leValueInput->clear();
}

void TimeoutWindow::on_btnImport_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(
        this, 
        tr("インポート"), 
        "", 
        tr("テキストファイル (*.txt)")
    );
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("エラー"), tr("ファイルを開けませんでした。"));
        return;
    }

    QTextStream in(&file);
    QStringList lines;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (!line.isEmpty())
            lines << line;
    }

    file.close();

    if (lines.size() % 2 != 0) {
        QMessageBox::warning(this, tr("フォーマットエラー"), tr("行数が奇数のため、正しく読み込めません。"));
        return;
    }

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    for (int i = 0; i < lines.size(); i += 2) {
        QString key = lines[i];
        QString value = lines[i + 1];

        bool ok;
        int timeoutValue = value.toInt(&ok);
        if (!ok || timeoutValue < 0) {
            QMessageBox::warning(this, tr("データエラー"), tr("タイムアウト値は0以上の整数である必要があります（'%1'）。").arg(timeoutValue));
            return;
        }
    }
}

void TimeoutWindow::on_btnExport_clicked()
{
    QString defaultName = "TimeoutMap_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".txt";
    QString fileName = QFileDialog::getSaveFileName(
        this, 
        tr("エクスポート"), 
        defaultName, 
        tr("テキストファイル (*.txt)")
    );
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("エラー"), tr("ファイルに書き込めませんでした。"));
        return;
    }

    QTextStream out(&file);
    const int rowCount = ui->tableWidget->rowCount();

    for (int i = 0; i < rowCount; ++i) {
        QTableWidgetItem *keyItem = ui->tableWidget->item(i, 0);
        QTableWidgetItem *valueItem = ui->tableWidget->item(i, 1);

        if (!keyItem || !valueItem) continue;

        QString key = keyItem->text().trimmed();
        QString value = valueItem->text().trimmed();

        if (key.isEmpty() || value.isEmpty()) continue;

        out << key << "\n";
        out << value << "\n";
    }

    file.close();
}

void TimeoutWindow::on_btnDelete_clicked() {
    QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();

    QSet<int> selectedRows;
    for (QTableWidgetItem *item : selectedItems) {
        selectedRows.insert(item->row());
    }

    // 高い行番号から削除しないと、削除中にrow番号がズレる
    QList<int> rowsToDelete = selectedRows.values();
    std::sort(rowsToDelete.begin(), rowsToDelete.end(), std::greater<int>());

    for (int row : rowsToDelete) {
        ui->tableWidget->removeRow(row);
    }
}

void TimeoutWindow::on_btnClear_clicked() {
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

    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);
}

void TimeoutWindow::on_btnApply_clicked()
{
    TableBinding::saveTableWidgetToConfig(ui->tableWidget, "TimeoutMap");
    ConfigManager::instance().save();
    this->close();  
}

void TimeoutWindow::on_btnCancel_clicked()
{
    this->close();
}