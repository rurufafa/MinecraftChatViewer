#ifndef KEYWORDEDITOR_H
#define KEYWORDEDITOR_H

#include <QDialog>

namespace Ui {
class KeywordEditDialog;
}

class KeywordEditDialog : public QDialog
{
    Q_OBJECT

public:
    explicit KeywordEditDialog(QWidget *parent = nullptr, QStringList *keywords = nullptr);
    ~KeywordEditDialog();
    QStringList getKeywords();

private slots:
    void on_btnAdd_clicked();
    void on_btnDelete_clicked();
    void on_btnApply_clicked();
    void on_btnCancel_clicked();

private:
    Ui::KeywordEditDialog *ui;
    QStringList &tempKeywords_;
};

#endif // KEYWORDEDITOR_H
