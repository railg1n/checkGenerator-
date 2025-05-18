#include "commonhelpers.h"

double commonHelpers::calculateTotal(QTableWidget* table)
{
    double total = 0.0;

    for (int i = 0; i < table->rowCount(); ++i)
    {
        bool ok1, ok2;
        int count = table->item(i, 1)->text().toInt(&ok1);
        double price = table->item(i, 2)->text().toDouble(&ok2);

        if (ok1 && ok2)
            total += count * price;
    }

    return total;
}

double commonHelpers::calculateChange(double total, double clientCash)
{
    return clientCash - total;
}

