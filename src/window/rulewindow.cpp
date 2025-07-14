#include <QDateTime>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollArea>
#include <QTextStream>

#include "rulewindow.h"
#include "ui_rulewindow.h"
#include "groupeditdialog.h"
#include "ruleeditdialog.h"

namespace {
    void setComboBox(QComboBox *comboBox, const QString &data) {
        int index = comboBox->findData(data); 
        if (index >= 0)
            comboBox->setCurrentIndex(index);
    }

    QString getComboBox(QComboBox *comboBox) {
        return comboBox->currentData().toString();
    }
}

RuleWindow::RuleWindow(QWidget *parent)
    : QDialog(parent)
    , ruleModels_(RuleModels::instance())
    , ui(new Ui::RuleWindow)
{
    ui->setupUi(this);
    ui->Priority->setButtonSymbols(QAbstractSpinBox::NoButtons);
    QHeaderView *header = ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);  

    ui->MatchMode->addItem("フィルター", "Filters");
    ui->MatchMode->addItem("正規表現", "Regex");

    ui->MatchMode_2->addItem("フィルター", "Filters");
    ui->MatchMode_2->addItem("正規表現", "Regex");

    ui->ReplaceMode->addItem("全文", "full");
    ui->ReplaceMode->addItem("合致した部分のみ", "partial");

    loadConfigToTreeWidget();
    connect(ui->treeWidget, &QTreeWidget::currentItemChanged,
        this, &RuleWindow::onTreeItemChanged);
}

RuleWindow::~RuleWindow()
{
    delete ui;
}

void RuleWindow::loadConfigToTreeWidget() {
    QString selectedType;
    QString selectedGroupId; 
    QString selectedRuleId;
    if (QTreeWidgetItem *current = ui->treeWidget->currentItem()) {
        selectedType = current->data(0, Qt::UserRole).toString();
        selectedGroupId = current->data(0, Qt::UserRole + 1).toString();
        selectedRuleId = current->data(0, Qt::UserRole + 2).toString();
    }

    ui->treeWidget->clear(); // 初期化
    RuleConfig &ruleConfig_ = ruleModels_.get();

    for (const RuleGroup &group : ruleConfig_.Groups) {
        // 親アイテム（Group）
        QTreeWidgetItem *groupItem = new QTreeWidgetItem(ui->treeWidget);
        groupItem->setText(0, group.Name);
        groupItem->setCheckState(0, group.Enabled ? Qt::Checked : Qt::Unchecked);
        groupItem->setData(0, Qt::UserRole, "Group"); // 識別用
        groupItem->setData(0, Qt::UserRole + 1, group.Id); // IDまたはKeyなど

        // 子アイテム（Rules）
        for (const Rule &rule : group.Rules) {
            QTreeWidgetItem *ruleItem = new QTreeWidgetItem(groupItem);
            ruleItem->setText(0, rule.Name);
            ruleItem->setCheckState(0, rule.Enabled ? Qt::Checked : Qt::Unchecked);
            ruleItem->setData(0, Qt::UserRole, "Rule");
            ruleItem->setData(0, Qt::UserRole + 1, group.Id);
            ruleItem->setData(0, Qt::UserRole + 2, rule.Id);
        }

        ui->treeWidget->addTopLevelItem(groupItem);
    }

    ui->treeWidget->expandAll(); 

    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        QString type = (*it)->data(0, Qt::UserRole).toString();
        QString groupId = (*it)->data(0, Qt::UserRole + 1).toString();
        QString ruleId = (*it)->data(0, Qt::UserRole + 2).toString();
        if (type == selectedType &&
            groupId == selectedGroupId &&
            (type == "Group" || ruleId == selectedRuleId)) {
            ui->treeWidget->setCurrentItem(*it);
            break;
        }
        ++it;
    }
}

void RuleWindow::onTreeItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous) {
    
    if (!current) return;
    if (current == previous) return;

    QString dataType = current->data(0, Qt::UserRole).toString();
    QString groupId = current->data(0, Qt::UserRole + 1).toString();
    auto optGroup = ruleModels_.getGroup(groupId);
    if (!optGroup) return;
    RuleGroup group = *optGroup;

    if (dataType == "Group") {
        ui->stackedWidget->setCurrentWidget(ui->groupDetail);
        ui->GroupName->setText(group.Name);
        ui->Priority->setValue(group.Priority);
        ui->Description->setText(group.Description);

    } else if (dataType == "Rule") { 
        QString ruleId = current->data(0, Qt::UserRole + 2).toString();
        auto optRule = ruleModels_.getRule(groupId, ruleId);
        if (!optRule) return;
        Rule rule = *optRule;

        if (rule.MatchMode == "Filters") {
            ui->stackedWidget->setCurrentWidget(ui->ruleFilterDetail);
            ui->RuleName->setText(rule.Name);
            setComboBox(ui->MatchMode, rule.MatchMode);
            ui->ReplaceText->setText(rule.ReplaceText);
            
            QTableWidget *tableWidget = ui->tableWidget;
            tableWidget->setRowCount(0); 

            QList<Filter> filters = rule.Match.Filters;

            for (int i = 0; i < filters.size(); i++) {
                Filter filter = filters[i];
                QString type_ = filter.Type;

                tableWidget->insertRow(i);

                QScrollArea *scrollArea = new QScrollArea();
                scrollArea->setWidgetResizable(true);
                scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
                scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

                QWidget *container = new QWidget();
                QHBoxLayout *layout = new QHBoxLayout(container);
                layout->setContentsMargins(0, 0, 0, 0);

                for (const QString &kw : filter.Keywords) {
                    QLabel *label = new QLabel(kw);
                    label->setFrameShape(QFrame::Shape::Box);
                    label->setEnabled(false);
                    layout->addWidget(label);
                }

                layout->addStretch();
                container->setLayout(layout);
                scrollArea->setWidget(container);
                tableWidget->setCellWidget(i, 0, scrollArea);
 
                QComboBox *combo = new QComboBox();
                combo->addItem("文頭がキーワードのいずれかである", "StartsWithAny");
                combo->addItem("文頭がキーワードのいずれでもない", "NotStartsWithAny");
                combo->addItem("文末がキーワードのいずれかである", "EndsWithAny");
                combo->addItem("文末がキーワードのいずれでもない", "NotEndsWithAny");
                combo->addItem("キーワードのいずれかを含む", "ContainsAny");
                combo->addItem("キーワードのいずれかを含まない", "NotContainsAny");
                combo->addItem("キーワードをすべて含む", "ContainsAll");
                combo->addItem("キーワードのすべてを含まない", "NotContainsAll");
                setComboBox(combo, filter.Type);
                combo->setEnabled(false);
                tableWidget->setCellWidget(i, 1, combo);
            }

            // 他の項目も更新
        } else if (rule.MatchMode == "Regex") {
            ui->stackedWidget->setCurrentWidget(ui->ruleRegexDetail);
            ui->RuleName_2->setText(rule.Name);
            setComboBox(ui->MatchMode_2, rule.MatchMode);
            ui->Regex->setText(rule.Match.Regex);
            setComboBox(ui->ReplaceMode, rule.Match.ReplaceMode);
            ui->ReplaceText_2->setText(rule.ReplaceText);
        }       
    }
}

void RuleWindow::deleteRule() {
    // 確認ダイアログ
    auto ret = QMessageBox::warning(this, tr("削除の確認"),
        tr("このルールを本当に削除してもよろしいですか？"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret != QMessageBox::Yes) {
        return;  
    }

    QTreeWidgetItem *current = ui->treeWidget->currentItem();
    if (!current) return;

    QString type = current->data(0, Qt::UserRole).toString();
    if (type != "Rule") return;

    QString parentGroupId = current->data(0, Qt::UserRole + 1).toString();
    QString ruleId = current->data(0, Qt::UserRole + 2).toString();

    if (!ruleModels_.removeRule(parentGroupId, ruleId)) {
        return;
    }

    loadConfigToTreeWidget(); // 再読み込み
}

void RuleWindow::editRule() {
    QTreeWidgetItem *current = ui->treeWidget->currentItem();
    if (!current) return;

    QString type = current->data(0, Qt::UserRole).toString();
    if (type != "Rule") return;

    QString parentGroupId = current->data(0, Qt::UserRole + 1).toString();
    QString ruleId = current->data(0, Qt::UserRole + 2).toString();

    auto optRule = ruleModels_.getRule(parentGroupId, ruleId);
    if (!optRule) return;
    Rule rule = *optRule;
    
    // ダイアログを開く
    RuleEditDialog dialog(this, &rule);

    if (dialog.exec() == QDialog::Accepted) {
        Rule editedRule = dialog.getRule();

        if(!ruleModels_.editRule(parentGroupId, ruleId, editedRule)) {
            QMessageBox::warning(this, "重複", "同じ名前のルールが既に存在します。");
            return;
        }
        
        loadConfigToTreeWidget();
    }
}

void RuleWindow::on_btnAddGroup_clicked() {
    // ダイアログを開く
    RuleGroup newGroup;
    GroupEditDialog dialog(this, &newGroup);

    if (dialog.exec() == QDialog::Accepted) {
        RuleGroup editedGroup = dialog.getGroup();

        if (!ruleModels_.addGroup(editedGroup)) {
            QMessageBox::warning(this, "重複", "同じ名前のグループが既に存在します。");
            return;
        }

        loadConfigToTreeWidget();
    }
}

void RuleWindow::on_btnEditGroup_clicked() {
    QTreeWidgetItem *current = ui->treeWidget->currentItem();
    if (!current) return;

    QString type = current->data(0, Qt::UserRole).toString();
    if (type != "Group") return;

    QString groupId = current->data(0, Qt::UserRole + 1).toString();
    auto optGroup = ruleModels_.getGroup(groupId);
    if (!optGroup) return;
    RuleGroup group = *optGroup;

    // ダイアログを開く
    GroupEditDialog dialog(this, &group);

    if (dialog.exec() == QDialog::Accepted) {
        RuleGroup editedGroup = dialog.getGroup();

        if (!ruleModels_.editGroup(groupId, editedGroup)) {
            QMessageBox::warning(this, "重複", "同じ名前のグループが既に存在します。");
            return;
        }

        loadConfigToTreeWidget();
    }
}

void RuleWindow::on_btnDeleteGroup_clicked() {
    // 確認ダイアログ
    auto ret = QMessageBox::warning(this, tr("削除の確認"),
        tr("このグループを本当に削除してもよろしいですか？"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret != QMessageBox::Yes) {
        return;  
    }

    QTreeWidgetItem *current = ui->treeWidget->currentItem();
    if (!current) return;

    QString type = current->data(0, Qt::UserRole).toString();
    if (type != "Group") return;

    QString groupId = current->data(0, Qt::UserRole + 1).toString();

    if (!ruleModels_.removeGroup(groupId)) {
        return;
    }

    loadConfigToTreeWidget(); 
}

void RuleWindow::on_btnAddRule_clicked() {
    QTreeWidgetItem *current = ui->treeWidget->currentItem();
    if (!current) return;

    QString type = current->data(0, Qt::UserRole).toString();
    if (type != "Group") return;

    QString parentGroupId = current->data(0, Qt::UserRole + 1).toString();

    // ダイアログを開く
    Rule newRule;
    RuleEditDialog dialog(this, &newRule);

    if (dialog.exec() == QDialog::Accepted) {
        Rule editedRule = dialog.getRule();

        if (!ruleModels_.addRule(parentGroupId, editedRule)) {
            QMessageBox::warning(this, "重複", "同じ名前のルールがこのグループに既に存在します。");
            return;
        } 

        loadConfigToTreeWidget();
    }
}

void RuleWindow::on_btnOK_clicked()
{
    // ツリー全体を走査
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        QTreeWidgetItem *item = *it;
        QString type = item->data(0, Qt::UserRole).toString();
        QString groupId = item->data(0, Qt::UserRole + 1).toString();
        QString ruleId = item->data(0, Qt::UserRole + 2).toString();

        if (type == "Group") {
            auto optGroup = ruleModels_.getGroup(groupId);
            if (optGroup) {
                RuleGroup group = *optGroup;
                group.Enabled = (item->checkState(0) == Qt::Checked);
                ruleModels_.editGroup(groupId, group); // Enabledだけ更新
            }
        } else if (type == "Rule") {
            auto optRule = ruleModels_.getRule(groupId, ruleId);
            if (optRule) {
                Rule rule = *optRule;
                rule.Enabled = (item->checkState(0) == Qt::Checked);
                ruleModels_.editRule(groupId, ruleId, rule); // Enabledだけ更新
            }
        }

        ++it;
    }

    // 保存
    ruleModels_.save();

    accept(); // ダイアログを閉じる
}

void RuleWindow::on_btnEditRule_clicked() {editRule();}
void RuleWindow::on_btnEditRule_2_clicked() {editRule();}
void RuleWindow::on_btnDeleteRule_clicked() {deleteRule();}
void RuleWindow::on_btnDeleteRule_2_clicked() {deleteRule();}

void RuleWindow::on_btnImport_clicked()
{
    // 確認ダイアログ
    auto ret = QMessageBox::warning(this, tr("インポートの確認"),
        tr("ファイルをインポートすると現在のルール設定は失われます。本当にインポートしますか？"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret != QMessageBox::Yes) {
        return;  
    }

    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("インポート"),
        "",
        tr("テキストファイル (*.txt);;すべてのファイル (*)")
    );

    if (filePath.isEmpty())
        return;

    RuleConfig importedConfig = ruleModels_.importRuleConfig(filePath);

    if (importedConfig.Groups.isEmpty()) {
        QMessageBox::warning(this, tr("インポート失敗"), tr("有効なルールが見つかりませんでした。"));
        return;
    }

    ruleModels_.set(importedConfig);
    ruleModels_.save();
    ruleModels_.rebuildIndex();

    QMessageBox::information(this, tr("インポート完了"), tr("ルールを正常にインポートしました。"));

    loadConfigToTreeWidget();
}

void RuleWindow::on_btnExport_clicked()
{
    QString defaultName = "Rule_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".txt";
    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("エクスポート"),
        defaultName,
        tr("テキストファイル (*.txt);;すべてのファイル (*)")
    );
    if (filePath.isEmpty())
        return;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, tr("エクスポート失敗"), tr("ファイルを書き込めませんでした。"));
        return;
    }

    QTextStream out(&file);
    const RuleConfig& config = ruleModels_.get();

    for (const RuleGroup& group : config.Groups) {
        out << "# Group: " << group.Name << "\n";
        out << "Description: " << group.Description << "\n";
        out << "Priority: " << group.Priority << "\n\n";

        for (const Rule& rule : group.Rules) {
            out << "# Rule: " << rule.Name << "\n";
            out << "MatchMode: " << rule.MatchMode << "\n";
            out << "ReplaceText: " << rule.ReplaceText << "\n";

            out << "<Match>\n";

            if (rule.MatchMode == "Regex") {
                out << "Regex: " << rule.Match.Regex << "\n";
                out << "ReplaceMode: " << rule.Match.ReplaceMode << "\n";
            } else if (rule.MatchMode == "Filters") {
                for (const Filter& filter : rule.Match.Filters) {
                    out << "Type: " << filter.Type << "\n";
                    out << "Keywords:\n";
                    for (const QString& keyword : filter.Keywords) {
                        out << keyword << "\n";
                    }
                }
            }

            out << "\n";
        }
    }

    file.close();

    QMessageBox::information(this, tr("エクスポート完了"), tr("ルールを正常にエクスポートしました。"));
}
