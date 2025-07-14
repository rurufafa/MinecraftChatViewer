#include "groupeditdialog.h"
#include "ui_groupeditdialog.h"

GroupEditDialog::GroupEditDialog(QWidget *parent, RuleGroup *group)
    : QDialog(parent)
    , tempGroup_(*group)
    , ui(new Ui::GroupEditDialog)
{
    ui->setupUi(this);
    ui->inputGroupName->setText(tempGroup_.Name);
    ui->inputPriority->setValue(tempGroup_.Priority);
    ui->inputDescription->setText(tempGroup_.Description);
}

GroupEditDialog::~GroupEditDialog()
{
    delete ui;
}

RuleGroup GroupEditDialog::getGroup()
{
    RuleGroup editedGroup = tempGroup_;
    editedGroup.Name = ui->inputGroupName->text(); 
    editedGroup.Priority = ui->inputPriority->value();
    editedGroup.Description = ui->inputDescription->toPlainText();
    return editedGroup;
}

void GroupEditDialog::on_btnApply_clicked() 
{
    accept();
}

void GroupEditDialog::on_btnCancel_clicked()
{
    reject();
}