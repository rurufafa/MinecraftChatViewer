#ifndef TIMEOUTWINDOW_H
#define TIMEOUTWINDOW_H

#include <QDialog>

namespace Ui {
class TimeoutWindow;
}

class TimeoutWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TimeoutWindow(QWidget *parent = nullptr);
    ~TimeoutWindow();

private slots:
    void on_btnAdd_clicked();
    void on_btnImport_clicked();
    void on_btnExport_clicked();
    void on_btnDelete_clicked();
    void on_btnClear_clicked();
    void on_btnApply_clicked();
    void on_btnCancel_clicked();

private:
    Ui::TimeoutWindow *ui;
};

#endif // TIMEOUTWINDOW_H
