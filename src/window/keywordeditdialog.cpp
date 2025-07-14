#include "keywordeditdialog.h"
#include "ui_keywordeditdialog.h"

KeywordEditDialog::KeywordEditDialog(QWidget *parent, QStringList *keywords)
    : QDialog(parent)
    , tempKeywords_(*keywords)
    , ui(new Ui::KeywordEditDialog)
{
    ui->setupUi(this);

    ui->listWidget->clear();
    for (const QString& kw : tempKeywords_) {
        ui->listWidget->addItem(kw);
    }
}

KeywordEditDialog::~KeywordEditDialog()
{
    delete ui;
}

QStringList KeywordEditDialog::getKeywords()
{
    QStringList editedKeywords;

    QListWidget *widget = ui->listWidget;
    for (int i = 0; i < widget->count(); ++i) {
        editedKeywords << widget->item(i)->text().trimmed();
    }

    return editedKeywords;
}

void KeywordEditDialog::on_btnAdd_clicked() 
{
    QString text = ui->inputKeyword->text();
    if (!text.isEmpty()) {
        ui->listWidget->addItem(text);  
    }
    ui->inputKeyword->clear();
}

void KeywordEditDialog::on_btnDelete_clicked()
{
    QListWidgetItem* currentItem = ui->listWidget->currentItem();
    delete currentItem;
}

void KeywordEditDialog::on_btnApply_clicked()
{
    accept();
}

void KeywordEditDialog::on_btnCancel_clicked()
{
    reject();
}




