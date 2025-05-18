#include <QTextDocument>
#include <QTextCursor>
#include <QTextTable>
#include <QPrinter>
#include <QDateTime>
#include <QPdfWriter>
#include <QMessageBox>
#include <QPainter>
#include <QRandomGenerator>
#include "generatepdf.h"

#include "qrcode/qrcodegen.hpp"

using qrcodegen::QrCode;

QImage generateQRCode(const QString &text, int scale = 4)
{
    auto qr = QrCode::encodeText(text.toUtf8().data(), QrCode::Ecc::LOW);
    int size = qr.getSize();
    QImage img(size * scale, size * scale, QImage::Format_RGB32);
    img.fill(Qt::white);
    QPainter painter(&img);
    painter.setBrush(Qt::black);
    for (int y = 0; y < size; y++)
    {
        for (int x = 0; x < size; x++)
        {
            if (qr.getModule(x, y))
                painter.drawRect(x * scale, y * scale, scale, scale);
        }
    }
    return img;
}

namespace generatePDF
{

void generatePDF(const QString &fileName, const QList<Product> &products, double total, double paid, double change)
{
    QPdfWriter writer(fileName);
    writer.setPageSize(QPageSize(QPageSize::A4)); // page format
    writer.setResolution(300);

    QPainter painter(&writer);
    if (!painter.isActive()) {
        QMessageBox::warning(nullptr, "Error", "failed to create PDF-file");
        return;
    }

    // fonts settings
    QFont font("Courier", 12);
    QFont boldFont("Courier", 12, QFont::Bold);
    painter.setFont(font);

    int y = 50;               // current vertical Y
    const int lineSpacing = 50;

    // === Заголовок ===
    QFont fancyFont("Garamond", 14, QFont::Bold);
    painter.setFont(fancyFont);
    painter.drawText(200, y, "Fruto Kitty Store");
    y+=50;

    // little kitty
    QFont catFont("Courier", 10);
    painter.setFont(catFont);

    QStringList catLines = {
        " /\\_/\\  (",
        "( ^.^ ) _)",
        "   \\\"/  (",
        " ( | | )",
        "(__d b__)"
    };

    // draw cat
    for (const QString &line : catLines) {
        painter.drawText(350, y, line);
        y += 30;
    }


    QPen solidPen(Qt::black);
    solidPen.setWidthF(4.0);
    painter.setPen(solidPen);
    painter.drawLine(50, y, 1000, y);
    y += lineSpacing;

    int orderId = QRandomGenerator::global()->bounded(100000, 999999);
    painter.setFont(font);
    painter.drawText(100, y, QString("Check № %1").arg(orderId));
    y += lineSpacing;

    // Date and time
    painter.setFont(font);
    painter.drawText(100, y, QDateTime::currentDateTime().toString("dd.MM.yyyy hh:mm"));
    y += lineSpacing;


    QPen dashedPen(Qt::black);
    dashedPen.setWidthF(1.5);
    dashedPen.setStyle(Qt::CustomDashLine);
    dashedPen.setDashPattern({4, 4});
    painter.setPen(dashedPen);
    painter.drawLine(50, y, 1000, y);
    y += lineSpacing;


    // Table columns
    painter.setPen(Qt::black);
    painter.setFont(boldFont);
    painter.drawText(60, y, "Name");
    painter.drawText(600, y, "Q-ty");
    painter.drawText(850, y, "Price");
    y += lineSpacing;

    //  list of the purchases
    QFont smallFont("Courier", 10);
    painter.setFont(smallFont);
    for (const Product &product : products)
    {
        // do not print empty lines
        ensureSpace(writer, y, lineSpacing);
        if (product.name.trimmed().isEmpty()) continue;
        painter.drawText(60, y, product.name);
        painter.drawText(630, y, QString::number(product.count));
        painter.drawText(850, y, QString::number(product.price, 'f', 2));
        y += lineSpacing;
    }


    painter.setPen(dashedPen);
    painter.drawLine(50, y, 1000, y);
    y += lineSpacing;

    // Sum, paid, change
    painter.setPen(Qt::black);
    painter.setFont(font);


    ensureSpace(writer, y, lineSpacing);

    painter.drawText(QRect(60, y, 300, lineSpacing), Qt::AlignLeft | Qt::AlignVCenter, "Sum:");
    painter.drawText(QRect(450, y, 510, lineSpacing), Qt::AlignRight | Qt::AlignVCenter,
                     QString("%1 UAH").arg(total, 0, 'f', 2));
    y += lineSpacing;

    painter.drawText(QRect(60, y, 300, lineSpacing), Qt::AlignLeft | Qt::AlignVCenter, "Paid:");
    painter.drawText(QRect(450, y, 510, lineSpacing), Qt::AlignRight | Qt::AlignVCenter,
                     QString("%1 UAH").arg(paid, 0, 'f', 2));
    y += lineSpacing;

    painter.drawText(QRect(60, y, 300, lineSpacing), Qt::AlignLeft | Qt::AlignVCenter, "Change:");
    painter.drawText(QRect(450, y, 510, lineSpacing), Qt::AlignRight | Qt::AlignVCenter,
                     QString("%1 UAH").arg(change, 0, 'f', 2));
    y += lineSpacing+25;

    painter.setPen(solidPen);
    painter.drawLine(50, y, 1000, y);
    y += lineSpacing;


    ensureSpace(writer, y, lineSpacing);

    painter.setPen(Qt::black);
    painter.setFont(boldFont);
    painter.drawText(100, y, "Thank you for your purchase!");
    y += lineSpacing;

    QFont centerFont("Courier", 10, QFont::Bold);
    painter.setFont(centerFont);
    painter.drawText(QRect(50, y, 800, lineSpacing), Qt::AlignCenter, "Scan for bonuses!");
    y += lineSpacing+10;


    ensureSpace(writer, y, 200);
    QImage qr = generateQRCode("https://github.com/railgin");
    int scaleFactor = 3;
    QImage scaledQR = qr.scaled(qr.width() * scaleFactor, qr.height() * scaleFactor, Qt::KeepAspectRatio, Qt::FastTransformation);
    int x = 300;
    painter.drawImage(x, y, scaledQR);
    y += lineSpacing;

    painter.end();

    // final
    QMessageBox::information(nullptr, "Success", "PDF file successully saved.");
}

//call it before every large fragment to add new page when it needed
void ensureSpace(QPdfWriter &writer, /*QPainter &painter,*/ int &y, int neededHeight)
{
    if (y + neededHeight > writer.height() - 50) {
        writer.newPage();
        y = 50;
    }
}

}
