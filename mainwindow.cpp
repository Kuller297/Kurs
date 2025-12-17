#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui_form.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent)  //Конструктор
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tablePurchase->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableSale->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableStock->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->actionSave, &QAction::triggered, this, &MainWindow::saveToFile);
    connect(ui->actionLoad, &QAction::triggered, this, &MainWindow::loadFromFile);
    connect(ui->actionReport, &QAction::triggered, this, &MainWindow::generateReport);
    connect(ui->actionBuy, &QAction::triggered, this, &MainWindow::buyProduct);
    connect(ui->actionSell, &QAction::triggered, this, &MainWindow::sellProduct);
    connect(ui->actionEdit, &QAction::triggered, this, &MainWindow::editProduct);
    connect(ui->actionDelete, &QAction::triggered, this, &MainWindow::deleteProduct);
    connect(ui->actionHelp, &QAction::triggered, this, &MainWindow::showHelp);
}

MainWindow::~MainWindow()       //Деструктор
{
    delete ui;
}

QTableWidget* MainWindow::getCurrentTable()
{
    int index = ui->tabWidget->currentIndex();
    switch(index) {
    case 0: return ui->tablePurchase;
    case 1: return ui->tableSale;
    case 2: return ui->tableStock;
    default: return ui->tablePurchase;
    }
}

void MainWindow::buyProduct()
{
    bool ok;
    QString product = QInputDialog::getText(this, "Купить товар", "Название товара:", QLineEdit::Normal, "", &ok);
    if (!ok || product.isEmpty()) return;

    QString manufacturer = QInputDialog::getText(this, "Купить товар", "Производитель:", QLineEdit::Normal, "", &ok);
    if (!ok || manufacturer.isEmpty()) return;

    int quantity = QInputDialog::getInt(this, "Купить товар", "Количество купленного:", 1, 1, 10000000, 1, &ok);
    if (!ok) return;

    int price = QInputDialog::getInt(this, "Купить товар", "Цена за единицу:", 1, 1, 2147483647, 1, &ok);
    if (!ok) return;

    QString purchaseDate = QInputDialog::getText(this, "Купить товар", "Дата покупки (дд.мм.гггг):",
                                                 QLineEdit::Normal, QDate::currentDate().toString("dd.MM.yyyy"), &ok);
    if (!ok || purchaseDate.isEmpty()) return;

    QDate date = QDate::fromString(purchaseDate, "dd.MM.yyyy");
    if (!date.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Некорректная дата! Используйте формат дд.мм.гггг");
        return;
    }
    purchaseDate = date.toString("dd.MM.yyyy");

    int spent = quantity * price;
    int row = ui->tablePurchase->rowCount();
    ui->tablePurchase->insertRow(row);

    ui->tablePurchase->setItem(row, 0, new QTableWidgetItem(product));
    ui->tablePurchase->setItem(row, 1, new QTableWidgetItem(manufacturer));
    ui->tablePurchase->setItem(row, 2, new QTableWidgetItem(QString::number(quantity)));
    ui->tablePurchase->setItem(row, 3, new QTableWidgetItem(QString::number(price)));
    ui->tablePurchase->setItem(row, 4, new QTableWidgetItem(QString::number(spent)));
    ui->tablePurchase->setItem(row, 5, new QTableWidgetItem(purchaseDate));

    updateStockTable();
    markDataChanged();
    QMessageBox::information(this, "Успех", "Покупка успешно добавлена!");
}

void MainWindow::sellProduct()
{
    bool ok;
    QString product = QInputDialog::getText(this, "Продать товар", "Название товара:", QLineEdit::Normal, "", &ok);
    if (!ok || product.isEmpty()) return;

    QString manufacturer = QInputDialog::getText(this, "Продать товар", "Производитель:", QLineEdit::Normal, "", &ok);
    if (!ok || manufacturer.isEmpty()) return;

    int availableQuantity = 0;
    for (int row = 0; row < ui->tableStock->rowCount(); ++row) {
        if (ui->tableStock->item(row, 0) &&
            ui->tableStock->item(row, 0)->text() == product &&
            ui->tableStock->item(row, 1) &&
            ui->tableStock->item(row, 1)->text() == manufacturer) {
            availableQuantity = ui->tableStock->item(row, 2)->text().toInt();
            break;
        }
    }

    if (availableQuantity <= 0) {
        QMessageBox::warning(this, "Ошибка", "Товар отсутствует на складе!");
        return;
    }

    int quantity = QInputDialog::getInt(this, "Продать товар","Количество для продажи (доступно: " + QString::number(availableQuantity) + "):",
                                        1, 1, availableQuantity, 1, &ok);
    if (!ok) return;

    int price = QInputDialog::getInt(this, "Продать товар", "Цена продажи за единицу:", 1, 1, 2147483647, 1, &ok);
    if (!ok) return;

    QString saleDate = QInputDialog::getText(this, "Продать товар", "Дата продажи (дд.мм.гггг):",
                                             QLineEdit::Normal, QDate::currentDate().toString("dd.MM.yyyy"), &ok);
    if (!ok || saleDate.isEmpty()) return;

    QDate date = QDate::fromString(saleDate, "dd.MM.yyyy");
    if (!date.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Некорректная дата! Используйте формат дд.мм.гггг");
        return;
    }
    saleDate = date.toString("dd.MM.yyyy");

    int row = ui->tableSale->rowCount();
    ui->tableSale->insertRow(row);
    ui->tableSale->setItem(row, 0, new QTableWidgetItem(product));
    ui->tableSale->setItem(row, 1, new QTableWidgetItem(manufacturer));
    ui->tableSale->setItem(row, 2, new QTableWidgetItem(QString::number(quantity)));
    ui->tableSale->setItem(row, 3, new QTableWidgetItem(QString::number(price)));
    ui->tableSale->setItem(row, 4, new QTableWidgetItem(QString::number(quantity * price)));
    ui->tableSale->setItem(row, 5, new QTableWidgetItem(saleDate));

    updateStockTable();
    markDataChanged();
    QMessageBox::information(this, "Успех", "Продажа успешно добавлена!");
}

void MainWindow::updateStockTable()
{
    ui->tableStock->setRowCount(0);
    QMap<QString, int> stockMap;

    for (int row = 0; row < ui->tablePurchase->rowCount(); ++row) {
        if (ui->tablePurchase->item(row, 0) && ui->tablePurchase->item(row, 1)) {
            QString key = ui->tablePurchase->item(row, 0)->text() + "|" + ui->tablePurchase->item(row, 1)->text();
            stockMap[key] += ui->tablePurchase->item(row, 2)->text().toInt();
        }
    }

    for (int row = 0; row < ui->tableSale->rowCount(); ++row) {
        if (ui->tableSale->item(row, 0) && ui->tableSale->item(row, 1)) {
            QString key = ui->tableSale->item(row, 0)->text() + "|" + ui->tableSale->item(row, 1)->text();
            if (stockMap.contains(key)) {
                stockMap[key] -= ui->tableSale->item(row, 2)->text().toInt();
                if (stockMap[key] < 0) stockMap[key] = 0;
            }
        }
    }

    for (auto it = stockMap.begin(); it != stockMap.end(); ++it) {
        QStringList parts = it.key().split("|");
        if (parts.size() == 2 && it.value() > 0) {
            int row = ui->tableStock->rowCount();
            ui->tableStock->insertRow(row);
            ui->tableStock->setItem(row, 0, new QTableWidgetItem(parts[0]));
            ui->tableStock->setItem(row, 1, new QTableWidgetItem(parts[1]));
            ui->tableStock->setItem(row, 2, new QTableWidgetItem(QString::number(it.value())));
        }
    }
}

void MainWindow::editProduct()
{
    QTableWidget* currentTable = getCurrentTable();

    int tabIndex = ui->tabWidget->currentIndex();
    if (tabIndex == 2) {
        QMessageBox::warning(this, "Ошибка",
                             "Редактирование записей в таблице склада запрещено!\n" "Остатки автоматически рассчитываются на основе покупок и продаж.");
        return;
    }

    int currentRow = currentTable->currentRow();
    if (currentRow == -1) {
        QMessageBox::warning(this, "Ошибка", "Выберите запись для редактирования!");
        return;
    }

    bool ok;
    QString product = QInputDialog::getText(this, "Редактировать", "Название товара:",
                                            QLineEdit::Normal, currentTable->item(currentRow, 0)->text(), &ok);
    if (!ok || product.isEmpty()) return;

    QString manufacturer = QInputDialog::getText(this, "Редактировать", "Производитель:",
                                                 QLineEdit::Normal, currentTable->item(currentRow, 1)->text(), &ok);
    if (!ok || manufacturer.isEmpty()) return;

    int quantity = QInputDialog::getInt(this, "Редактировать", "Количество:",
                                        currentTable->item(currentRow, 2)->text().toInt(), 1, 10000000, 1, &ok);
    if (!ok) return;

    int price = QInputDialog::getInt(this, "Редактировать", "Цена:",
                                     currentTable->item(currentRow, 3)->text().toInt(), 1, 2147483647, 1, &ok);
    if (!ok) return;

    QString dateStr = QInputDialog::getText(this, "Редактировать", "Дата (дд.мм.гггг):",
                                            QLineEdit::Normal, currentTable->item(currentRow, 5)->text(), &ok);
    if (!ok || dateStr.isEmpty()) return;

    currentTable->item(currentRow, 0)->setText(product);
    currentTable->item(currentRow, 1)->setText(manufacturer);
    currentTable->item(currentRow, 2)->setText(QString::number(quantity));
    currentTable->item(currentRow, 3)->setText(QString::number(price));
    currentTable->item(currentRow, 4)->setText(QString::number(quantity * price));
    currentTable->item(currentRow, 5)->setText(dateStr);

    updateStockTable();
    markDataChanged();
    QMessageBox::information(this, "Успех", "Запись отредактирована!");
}

void MainWindow::deleteProduct()
{
    QTableWidget* currentTable = getCurrentTable();

    int tabIndex = ui->tabWidget->currentIndex();
    if (tabIndex == 2) {
        QMessageBox::warning(this, "Ошибка",
                             "Удаление записей в таблице склада запрещено!\n" "Остатки автоматически рассчитываются на основе покупок и продаж.");
        return;
    }

    int currentRow = currentTable->currentRow();
    if (currentRow != -1 && QMessageBox::question(this, "Удаление", "Удалить запись?") == QMessageBox::Yes) {
        currentTable->removeRow(currentRow);
        updateStockTable();
        markDataChanged();
    }
}

void MainWindow::saveToFile()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить данные", "", "Текстовые файлы (*.txt)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return;

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out.setGenerateByteOrderMark(true);

    out << "[PURCHASES]\n";
    for (int r = 0; r < ui->tablePurchase->rowCount(); ++r) {
        QStringList rowData;
        for (int c = 0; c < ui->tablePurchase->columnCount(); ++c)
            rowData << (ui->tablePurchase->item(r, c) ? ui->tablePurchase->item(r, c)->text() : "");
        out << rowData.join(";") << "\n";
    }

    out << "[SALES]\n";
    for (int r = 0; r < ui->tableSale->rowCount(); ++r) {
        QStringList rowData;
        for (int c = 0; c < ui->tableSale->columnCount(); ++c)
            rowData << (ui->tableSale->item(r, c) ? ui->tableSale->item(r, c)->text() : "");
        out << rowData.join(";") << "\n";
    }
    file.close();
    markDataSaved();
}

void MainWindow::loadFromFile()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл", "", "Текстовые файлы (*.txt)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) return;

    ui->tablePurchase->setRowCount(0);
    ui->tableSale->setRowCount(0);

    QTextStream in(&file);
    in.setCodec("UTF-8");

    QTableWidget* currentTable = nullptr;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line == "[PURCHASES]") { currentTable = ui->tablePurchase; continue; }
        if (line == "[SALES]") { currentTable = ui->tableSale; continue; }
        if (currentTable && !line.isEmpty()) {
            QStringList fields = line.split(";");
            if (fields.size() >= currentTable->columnCount()) {
                int r = currentTable->rowCount();
                currentTable->insertRow(r);
                for (int c = 0; c < currentTable->columnCount(); ++c)
                    currentTable->setItem(r, c, new QTableWidgetItem(fields[c]));
            }
        }
    }
    file.close();
    updateStockTable();
    markDataSaved();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (dataChanged) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Несохраненные изменения","У вас есть несохраненные изменения. Сохранить перед выходом?",
                                      QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

        if (reply == QMessageBox::Save) {
            saveToFile();
            if (!dataChanged) {
                event->accept();
            } else {
                event->ignore();
            }
        } else if (reply == QMessageBox::Discard) {
            event->accept();
        } else {
            event->ignore();
        }
    } else {
        event->accept();
    }
}

void MainWindow::showHelp()
{
    QMessageBox::about(this, "О программе","Данная программа представляет собой приложение для складского учета\n"
                                            "С помощью нее вы можете:\n" "1) Добавить товар и всю информацию о нем\n" "2) Редактирование данных о товаре\n"
                                            "3) Удалить товар из списка\n" "4) Создать отчет за определенный период\n");
}

void MainWindow::markDataChanged() {
    dataChanged = true;
}

void MainWindow::markDataSaved() {
    dataChanged = false;
}

void MainWindow::generateReport()
{
    QDialog dialog(this);
    Ui::Form formUi;
    formUi.setupUi(&dialog);
    dialog.setWindowTitle(QStringLiteral("Создание отчета CSV"));

    formUi.dateEditStart->setDate(QDate::currentDate().addMonths(-1));
    formUi.dateEditEnd->setDate(QDate::currentDate());

    connect(formUi.btnGenerate, &QPushButton::clicked, &dialog, [&]() {
        QDate startDate = formUi.dateEditStart->date();
        QDate endDate = formUi.dateEditEnd->date();

        if (startDate > endDate) {
            QMessageBox::warning(&dialog, "Ошибка", "Дата начала позже даты окончания!");
            return;
        }

        QString fileName = QFileDialog::getSaveFileName(&dialog, QStringLiteral("Сохранить отчет"), "", "CSV файлы (*.csv)");
        if (fileName.isEmpty()) return;

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::warning(&dialog, QStringLiteral("Ошибка"), QStringLiteral("Не удалось создать файл!"));
            return;
        }

        QTextStream out(&file);

        out.setCodec("UTF-8");
        out.setGenerateByteOrderMark(true);

        QString sep = ";";

        out << QStringLiteral("ОТЧЕТ ПО СКЛАДУ") << "\n";
        out << QStringLiteral("Период") << sep << startDate.toString("dd.MM.yyyy") << sep << endDate.toString("dd.MM.yyyy") << "\n\n";

        out << QStringLiteral("КУПЛЕНО") << "\n";
        out << QStringLiteral("Товар") << sep << QStringLiteral("Производитель") << sep << QStringLiteral("Количество") << sep
            << QStringLiteral("Цена") << sep << QStringLiteral("Сумма") << sep << QStringLiteral("Дата") << "\n";
        for (int row = 0; row < ui->tablePurchase->rowCount(); ++row) {
            QDate d = QDate::fromString(ui->tablePurchase->item(row, 5)->text(), "dd.MM.yyyy");
            if (d >= startDate && d <= endDate) {
                QStringList rowData;
                for (int col = 0; col < 6; ++col) rowData << ui->tablePurchase->item(row, col)->text();
                out << rowData.join(sep) << "\n";
            }
        }

        out << "\n" << QStringLiteral("ПРОДАНО") << "\n";
        out << QStringLiteral("Товар") << sep << QStringLiteral("Производитель") << sep << QStringLiteral("Количество") << sep
            << QStringLiteral("Цена") << sep << QStringLiteral("Выручка") << sep << QStringLiteral("Дата") << "\n";
        for (int row = 0; row < ui->tableSale->rowCount(); ++row) {
            QDate d = QDate::fromString(ui->tableSale->item(row, 5)->text(), "dd.MM.yyyy");
            if (d >= startDate && d <= endDate) {
                QStringList rowData;
                for (int col = 0; col < 6; ++col) rowData << ui->tableSale->item(row, col)->text();
                out << rowData.join(sep) << "\n";
            }
        }

        out << "\n" << QStringLiteral("ОСТАТКИ НА СКЛАДЕ") << "\n";
        out << QStringLiteral("Товар") << sep << QStringLiteral("Производитель") << sep << QStringLiteral("Количество") << "\n";
        for (int row = 0; row < ui->tableStock->rowCount(); ++row) {
            QStringList rowData;
            for (int col = 0; col < 3; ++col) rowData << ui->tableStock->item(row, col)->text();
            out << rowData.join(sep) << "\n";
        }

        file.close();
        QMessageBox::information(&dialog, QStringLiteral("Успех"), QStringLiteral("Отчет успешно создан"));
        dialog.accept();
    });

    connect(formUi.btnCancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    dialog.exec();
}
