#ifndef GENERATEPDF_H
#define GENERATEPDF_H

#include <QString>
#include <QTableWidget>
#include <QPdfWriter>

struct Product
{
    QString name;
    int count;
    double price;
};

namespace generatePDF
{
    void generatePDF(const QString &fileName, const QList<Product> &products, double total, double paid, double change);
    void ensureSpace(QPdfWriter &writer, /*QPainter &painter,*/ int &y, int neededHeight);
}
#endif // GENERATEPDF_H
