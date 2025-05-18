#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "commonhelpers.h"
#include "generatePDF.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("checkGenerator!");

    ui->tableWidget->setColumnCount(3);
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    connect(ui->addRowButton, &QPushButton::clicked, this, &MainWindow::onAddRowClicked);
    connect(ui->tableWidget, &QTableWidget::cellChanged, this, &MainWindow::onTableCellChanged);

    ui->tableWidget->setHorizontalHeaderLabels({"Name", "Quantity", "Price"});
    connect(ui->loadFromFileButton, &QPushButton::clicked, this, &MainWindow::onLoadFromFileClicked);
    connect(ui->deleteRowButton, &QPushButton::clicked, this, &MainWindow::ondeleteRowButtonClicked);
    connect(ui->clearRowsButton, &QPushButton::clicked, this, &MainWindow::onclearRowsButtonClicked);

    connect(ui->lineEditClientCash, &QLineEdit::editingFinished, this, &MainWindow::updateChange);

    connect(ui->ExportToPdf, &QPushButton::clicked, this, &MainWindow::onExportToPdfClicked);

    connect(ui->payButton, &QPushButton::clicked, this, &MainWindow::onPayClicked);

    ui->ExportToPdf->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::onTableCellChanged(int row, int column)
{
    Q_UNUSED(column);

    QTableWidgetItem* nameItem = ui->tableWidget->item(row, 0);
    QTableWidgetItem* countItem = ui->tableWidget->item(row, 1);
    QTableWidgetItem* priceItem = ui->tableWidget->item(row, 2);


    if (!nameItem || !countItem || !priceItem)
        return;

    QString name = nameItem->text().trimmed();
    QString countStr = countItem->text().trimmed();
    QString priceStr = priceItem->text().trimmed();

    if (name.isEmpty() || countStr.isEmpty() || priceStr.isEmpty())
        return;

    bool okCount = false;
    bool okPrice = false;
    int count = countStr.toInt(&okCount);
    double price = priceStr.toDouble(&okPrice);

    if (!okCount || !okPrice)
        return;

    updateTotal();
}

void MainWindow::onAddRowClicked()
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);

    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(""));  // Name
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(""));  // Count
    ui->tableWidget->setItem(row, 2, new QTableWidgetItem(""));  // Price

    ui->tableWidget->setCurrentCell(row, 0);
}

void MainWindow::addOrUpdateRow(const QString& name, int count, double price)
{
    for (int row = 0; row < ui->tableWidget->rowCount(); ++row)
    {
        QString existingName = ui->tableWidget->item(row, 0)->text();
        if (existingName == name)
        {
            // if present -- increase
            int existingCount = ui->tableWidget->item(row, 1)->text().toInt();
            ui->tableWidget->item(row, 1)->setText(QString::number(existingCount + count));

            // update price if needed
            ui->tableWidget->item(row, 2)->setText(QString::number(price));
            updateTotal();
            return;
        }
    }

    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);
    ui->tableWidget->setItem(row, 0, new QTableWidgetItem(name));
    ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(count)));
    ui->tableWidget->setItem(row, 2, new QTableWidgetItem(QString::number(price)));
    updateTotal();
}

void MainWindow::ondeleteRowButtonClicked()
{
    int selectedRow = ui->tableWidget->currentRow();
    if (selectedRow >= 0)
    {
        ui->tableWidget->removeRow(selectedRow);
    }
    else
    {
        ui->ExportToPdf->setEnabled(false);
    }
    updateTotal();
}

void MainWindow::onclearRowsButtonClicked()
{
    ui->tableWidget->setRowCount(0);
    updateTotal();
    ui->ExportToPdf->setEnabled(false);
}

void MainWindow::onLoadFromFileClicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open product file", "", "Text Files (*.txt *.csv);;All Files (*)");
    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, "Error", "Can't open file");
        return;
    }

    QTextStream in(&file);

    while (!in.atEnd())
    {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList fields = line.split(',');

        if (fields.size() < 3)
        {
            QMessageBox::warning(this, "Error", "Incorrect format: not enough columns");
            continue;
        }

        QString name = fields[0].trimmed();
        QString countStr = fields[1].trimmed();
        QString priceStr = fields[2].trimmed();

        bool okCount = false;
        bool okPrice = false;
        int count = countStr.toInt(&okCount);
        double price = priceStr.toDouble(&okPrice);

        if (!okCount || !okPrice)
        {
            QMessageBox::warning(this, "Error", QString("Invalid number format in line: %1").arg(line));
            continue;
        }

        addOrUpdateRow(name, count, price);
    }

    file.close();
}

void MainWindow::updateTotal()
{
    m_total = commonHelpers::calculateTotal(ui->tableWidget);
    ui->labelTotal->setText(QString("Sum: %1 UAH").arg(m_total, 0, 'f', 2));
}

void MainWindow::updateChange()
{
    bool ok;
    double clientCash = ui->lineEditClientCash->text().toDouble(&ok);
    if (!ok)
    {
        ui->labelChange->setText("Incorrect sum");
        return;
    }

    m_change = commonHelpers::calculateChange(m_total, clientCash);

    if (m_change < 0)
    {
        ui->labelChange->setText("Not enough amount");
    }
    else
    {
        ui->labelChange->setText(QString("Change: %1 UAH").arg(m_change, 0, 'f', 2));
    }
}

void MainWindow::onExportToPdfClicked()
{
    QString clientStr = ui->lineEditClientCash->text();
    double paid = clientStr.toDouble();


    QList<Product> products;
    int rowCount = ui->tableWidget->rowCount();

    for (int row = 0; row < rowCount; ++row)
    {
        QString name = ui->tableWidget->item(row, 0)->text();
        int count = ui->tableWidget->item(row, 1)->text().toInt();
        double price = ui->tableWidget->item(row, 2)->text().toDouble();

        products.append(Product{name, count, price});
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save check in PDF", "chek.pdf", "PDF files (*.pdf)");
    if (fileName.isEmpty())
        return;

     generatePDF::generatePDF(fileName, products, m_total, paid, m_change);
}


void MainWindow::onPayClicked()
{
    bool ok;
    double clientCash = ui->lineEditClientCash->text().toDouble(&ok);
    if (!ok)
    {
        QMessageBox::warning(this, "Error", "Incorrect sum");
        return;
    }

    double total = commonHelpers::calculateTotal(ui->tableWidget);
    double change = clientCash - total;

    if (change < 0)
    {
        QMessageBox::critical(this, "Not enough", "Not enough money on the balance");
        ui->labelChange->setStyleSheet("color: white; background-color: red; font-weight: bold;");
        ui->ExportToPdf->setEnabled(false);
        return;
    }

    // success payment
    QMessageBox::information(this, "Pay", "Pay successful!");


    // save rest
    m_total = total;
    m_change = change;

    ui->labelChange->setText(QString("Change: %1 UAH").arg(change, 0, 'f', 2));

    //color
    ui->labelChange->setStyleSheet("color: white; background-color: green; font-weight: bold;");

    // enable print
    ui->ExportToPdf->setEnabled(true);
}
