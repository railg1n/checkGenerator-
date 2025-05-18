#ifndef COMMONHELPERS_H
#define COMMONHELPERS_H

#include <QString>
#include <QTableWidget>

namespace commonHelpers
{
    double calculateTotal(QTableWidget* table);
    double calculateChange(double total, double clientCash);
}

#endif // COMMONHELPERS_H
