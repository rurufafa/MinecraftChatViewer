#ifndef TITLEWINDOW_H
#define TITLEWINDOW_H

#include <QDialog>

namespace Ui {
class TitleWindow;
}

class TitleWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TitleWindow(QWidget *parent = nullptr);
    ~TitleWindow();

private slots:
    void on_btnFileSetting_clicked();
    void on_btnKeywordSetting_clicked();
    void on_btnStart_clicked();
    void on_btnSetting_clicked();
    void on_btnExit_clicked();

private:
    Ui::TitleWindow *ui;
};

#endif // TITLEWINDOW_H
