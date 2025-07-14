#include <QMessageBox>
#include <QScrollArea>

#include "log_formatter.h"

#include "ruleeditdialog.h"
#include "ui_ruleeditdialog.h"
#include "keywordeditdialog.h"

namespace {
    void setComboBox(QComboBox *comboBox, const QString& data) {
        int index = comboBox->findData(data); 
        if (index >= 0)
            comboBox->setCurrentIndex(index);
    }

    QString getComboBox(QComboBox *comboBox) {
        return comboBox->currentData().toString();
    }
}

RuleEditDialog::RuleEditDialog(QWidget *parent, Rule *rule)
    : QDialog(parent)
    , tempRule_(*rule)
    , ui(new Ui::RuleEditDialog)
{
    ui->setupUi(this);

    ui->inputMatchMode->addItem("フィルター", "Filters");
    ui->inputMatchMode->addItem("正規表現", "Regex");

    ui->inputReplaceMode->addItem("全文", "full");
    ui->inputReplaceMode->addItem("合致した部分のみ", "partial");

    QHeaderView *header = ui->tableWidget->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::ResizeMode::Interactive);

    // 列ごとにサイズ設定
    header->setSectionResizeMode(0, QHeaderView::Stretch);     
    header->setSectionResizeMode(1, QHeaderView::Fixed); 
    header->setSectionResizeMode(2, QHeaderView::Stretch);    
    header->setSectionResizeMode(3, QHeaderView::Fixed); 

    ui->inputRuleName->setText(tempRule_.Name);

    connect(ui->inputMatchMode, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &RuleEditDialog::onMatchModeChanged);
    setComboBox(ui->inputMatchMode, tempRule_.MatchMode);
    
    if (tempRule_.MatchMode == "Filters") {
        tempFilters_ = tempRule_.Match.Filters;
        refreshFilterTable();
    } else if (tempRule_.MatchMode == "Regex") {
        ui->inputRegex->setText(tempRule_.Match.Regex);
        setComboBox(ui->inputReplaceMode, tempRule_.Match.ReplaceMode);
    }
    ui->inputReplaceText->setText(tempRule_.ReplaceText);
}

RuleEditDialog::~RuleEditDialog()
{
    delete ui;
}

void RuleEditDialog::refreshFilterTable()
{
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(tempFilters_.size());

    for (int i = 0; i < tempFilters_.size(); i++) {
        Filter filter = tempFilters_[i];
        addFilter(filter, i);
    }   
}

Rule RuleEditDialog::getRule()
{
    Rule editedRule = tempRule_;
    editedRule.Name = ui->inputRuleName->text(); 
    editedRule.MatchMode = getComboBox(ui->inputMatchMode);
    if (editedRule.MatchMode == "Filters") {
        editedRule.Match.Filters = tempFilters_;
    } else if (editedRule.MatchMode == "Regex") {
        editedRule.Match.Regex = ui->inputRegex->text();
        editedRule.Match.ReplaceMode = getComboBox(ui->inputReplaceMode);
    }
    editedRule.ReplaceText = ui->inputReplaceText->text();

    return editedRule;
}

void RuleEditDialog::addFilter(const Filter& filter, int row)
{   
    QTableWidget *tableWidget = ui->tableWidget;

    // キーワード
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QWidget *container = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(container);
    layout->setContentsMargins(0, 0, 0, 0);
    for (const QString& kw : filter.Keywords) {
        QLabel *label = new QLabel(kw);
        label->setFrameShape(QFrame::Shape::Box);
        layout->addWidget(label);
    }
    layout->addStretch();
    container->setLayout(layout);
    scrollArea->setWidget(container);
    tableWidget->setCellWidget(row, 0, scrollArea);

    // キーワード編集ボタン
    QPushButton *btnEditKeyword = new QPushButton("...");
    btnEditKeyword->setProperty("row", row);
    connect(btnEditKeyword, &QPushButton::clicked, this, &RuleEditDialog::onEditKeyword);
    tableWidget->setCellWidget(row, 1, btnEditKeyword);

    // 条件タイプ
    QComboBox *inputType = new QComboBox();
    inputType->addItem("文頭がキーワードのいずれかである", "StartsWithAny");
    inputType->addItem("文頭がキーワードのいずれでもない", "NotStartsWithAny");
    inputType->addItem("文末がキーワードのいずれかである", "EndsWithAny");
    inputType->addItem("文末がキーワードのいずれでもない", "NotEndsWithAny");
    inputType->addItem("キーワードのいずれかを含む", "ContainsAny");
    inputType->addItem("キーワードのいずれかを含まない", "NotContainsAny");
    inputType->addItem("キーワードをすべて含む", "ContainsAll");
    inputType->addItem("キーワードのすべてを含まない", "NotContainsAll");

    inputType->setProperty("row", row); 
    setComboBox(inputType, filter.Type);
    connect(inputType, QOverload<int>::of(&QComboBox::currentIndexChanged), 
        this, &RuleEditDialog::onTypeChanged);
    tableWidget->setCellWidget(row, 2, inputType);

    // 削除ボタン
    QPushButton *btnRemoveFilter = new QPushButton("削除");
    btnRemoveFilter->setProperty("row", row);
    connect(btnRemoveFilter, &QPushButton::clicked, this, &RuleEditDialog::onDeleteFilter);
    tableWidget->setCellWidget(row, 3, btnRemoveFilter);
}

void RuleEditDialog::on_btnAddFilter_clicked()
{
    // ダイアログを開く
    Filter newFilter;
    tempFilters_.append(newFilter);
    refreshFilterTable();
}

void RuleEditDialog::onEditKeyword() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    int row = btn->property("row").toInt();
    Filter f = tempFilters_[row];

    // キーワード編集ダイアログ
    KeywordEditDialog dlg(this, &f.Keywords);
    if (dlg.exec() == QDialog::Accepted) {
        tempFilters_[row].Keywords = dlg.getKeywords();
        refreshFilterTable();
    }
}

void RuleEditDialog::onDeleteFilter() {
    // 確認ダイアログ
    auto ret = QMessageBox::warning(this, tr("削除の確認"),
        tr("このフィルターを本当に削除してもよろしいですか？"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (ret != QMessageBox::Yes) {
        return;  
    }

    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    int row = btn->property("row").toInt();
    tempFilters_.removeAt(row);
    refreshFilterTable();  
}

void RuleEditDialog::on_btnApply_clicked()
{
    accept();
}

void RuleEditDialog::on_btnCancel_clicked()
{
    reject();
}

void RuleEditDialog::onMatchModeChanged(int index)
{
    QString mode = ui->inputMatchMode->itemData(index).toString();
    if (mode == "Filters") {
        ui->stackedWidget->setCurrentWidget(ui->pageFilters);
        ui->pageFilters->setEnabled(true);
        ui->pageRegex->setEnabled(false);
    } else if (mode == "Regex") {
        ui->stackedWidget->setCurrentWidget(ui->pageRegex);
        ui->pageRegex->setEnabled(true);
        ui->pageFilters->setEnabled(false);
    }
}

void RuleEditDialog::onTypeChanged(int)
{
    QComboBox *combo = qobject_cast<QComboBox*>(sender());
    if (!combo) return;

    int row = combo->property("row").toInt();
    if (row >= 0 && row < tempFilters_.size()) {
        qDebug() << tempFilters_[row].Keywords
                 << tempFilters_[row].Type;
        tempFilters_[row].Type = getComboBox(combo);
    }
}

void RuleEditDialog::on_btnPreview_clicked()
{
    QString targetText = ui->inputPreviewText->text();
    Rule currentRule = getRule();
    RuleEntry entry;
    ParsedValue pv = LogFormatter::parseValueTemplate(currentRule.ReplaceText);
    if (currentRule.MatchMode == "Regex") {
        LogFormatter::initializeRegexRule(currentRule, entry, pv);
    }else if (currentRule.MatchMode == "Filters") {
        LogFormatter::initializeFilterRule(currentRule, entry, pv);
    }

    if (entry.matcher(targetText)) {
        targetText = entry.replacer(targetText);
    }

    ui->outputPreviewText->setText(targetText);
}