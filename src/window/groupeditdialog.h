#ifndef GROUPEDITDIALOG_H
#define GROUPEDITDIALOG_H

#include <QDialog>

#include "rule_models.h"

namespace Ui {
class GroupEditDialog;
}

class GroupEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GroupEditDialog(QWidget *parent = nullptr, RuleGroup *group = nullptr);
    ~GroupEditDialog();
    RuleGroup getGroup();

private slots: 
    void on_btnApply_clicked();
    void on_btnCancel_clicked();

private:
    Ui::GroupEditDialog *ui;
    RuleGroup &tempGroup_;
};

#endif // GROUPEDITDIALOG_H
