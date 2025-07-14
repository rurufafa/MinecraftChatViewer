#ifndef RULEEDITDIALOG_H
#define RULEEDITDIALOG_H

#include <QDialog>

#include "rule_models.h"

namespace Ui {
class RuleEditDialog;
}

class RuleEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RuleEditDialog(QWidget *parent = nullptr, Rule *rule = nullptr);
    ~RuleEditDialog();
    Rule getRule();

private slots:
    void on_btnAddFilter_clicked();
    void onEditKeyword();
    void onDeleteFilter();
    void on_btnApply_clicked();
    void on_btnCancel_clicked();
    void onMatchModeChanged(int index);
    void onTypeChanged(int);
    void on_btnPreview_clicked();

private:
    void refreshFilterTable();
    void addFilter(const Filter &filter, int row);
    Ui::RuleEditDialog *ui;
    Rule tempRule_;
    QList<Filter> tempFilters_;
};

#endif // RULEEDITDIALOG_H
