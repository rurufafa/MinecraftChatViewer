#ifndef KEYWORDWINDOW_H
#define KEYWORDWINDOW_H

#include <QDialog>

namespace Ui {
class KeywordWindow;
}

class QLineEdit;
class QListWidget;

class KeywordWindow : public QDialog
{
    Q_OBJECT

public:
    explicit KeywordWindow(QWidget *parent = nullptr);
    ~KeywordWindow();

private slots:
    void on_btnBlackAdd_clicked();
    void on_btnWhiteAdd_clicked();
    void on_btnBlackDelete_clicked();
    void on_btnWhiteDelete_clicked();
    void on_btnBlackClear_clicked();
    void on_btnWhiteClear_clicked();
    void on_btnImport_clicked();
    void on_btnExport_clicked();
    void on_btnApply_clicked();
    void on_btnCancel_clicked();

private:
    Ui::KeywordWindow *ui;
    void addItemToList(QLineEdit*, QListWidget*);
    void deleteSelectedItem(QListWidget*);
    void clearList(QListWidget*); 
};

#endif // KEYWORDWINDOW_H
