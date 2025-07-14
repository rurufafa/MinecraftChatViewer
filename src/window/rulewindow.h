#ifndef RULEWINDOW_H
#define RULEWINDOW_H

#include <QDialog>
#include <QTreeWidgetItem>

#include "rule_models.h"

namespace Ui {
class RuleWindow;
}

class RuleWindow : public QDialog
{
    Q_OBJECT

public:
    explicit RuleWindow(QWidget *parent = nullptr);
    ~RuleWindow();

private slots:
    void onTreeItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    void on_btnAddGroup_clicked();
    void on_btnEditGroup_clicked();
    void on_btnDeleteGroup_clicked();

    void on_btnAddRule_clicked();
    void on_btnEditRule_clicked();
    void on_btnEditRule_2_clicked();
    void on_btnDeleteRule_clicked();
    void on_btnDeleteRule_2_clicked();
    void on_btnOK_clicked();
    void on_btnImport_clicked();
    void on_btnExport_clicked();
private:
    void loadConfigToTreeWidget();
    void deleteRule();
    void editRule();

    Ui::RuleWindow *ui;
    RuleModels &ruleModels_;
};

#endif // RULEWINDOW_H
