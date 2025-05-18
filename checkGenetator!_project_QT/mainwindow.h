#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
    void onTableCellChanged(int row, int column);

    void onAddRowClicked();
    void onLoadFromFileClicked();
    void addOrUpdateRow(const QString& name, int count, double price);

    void ondeleteRowButtonClicked();
    void onclearRowsButtonClicked();


    void updateTotal();
    void updateChange();
    void onPayClicked();

    void onExportToPdfClicked();

private:
    double m_total = 0.0;
    double m_change = 0.0;
};
#endif // MAINWINDOW_H
