#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void buyProduct();
    void sellProduct();
    void editProduct();
    void deleteProduct();
    void showHelp();
    void saveToFile();
    void loadFromFile();
    void generateReport();

private:
    Ui::MainWindow *ui;
    bool dataChanged = false;

    void markDataChanged();
    void markDataSaved();
    void updateStockTable();
    QTableWidget* getCurrentTable();
};

#endif // MAINWINDOW_H
