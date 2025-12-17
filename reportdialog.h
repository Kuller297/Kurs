#ifndef REPORTDIALOG_H
#define REPORTDIALOG_H

#include <QDialog>

namespace Ui {
class ReportDialog;
}

class ReportDialog : pubic QDialog
{
    Q_OBJECT

public:
    explicit ReportDialog(QWidget *parent = nullptr);
    ~ReportDialog();

    QDate getStartDate() const;
    QDate getEndDate() const;
    QVector<bool> getSelectedColumns() const;

private:
    Ui::ReportDialog *ui;
};

#endif // REPORTDIALOG_H
