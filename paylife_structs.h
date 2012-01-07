#ifndef PAYLIFE_STRUCTS_H
#define PAYLIFE_STRUCTS_H
#include <QString>
#include <QSettings>
static QString format_price(QString price) {
    price = price.replace(",","");
    price = price.replace(".","");
    while (price.length() < 8) {
        price = "0" + price;
    }
    return price;
}
static void cat(char * dest, char * src, int strt, int len) {
    int i;
    int j = 0;
    for (i  = strt; i < len + 1; i++) {
        dest[i] = src[j];
        j++;
    }
}
// Here we define the different structs for Paylife/Artema Hybrid
static QString artema_a_struct(QString price,QString ind,int nummer, QString personal,QString konto) {
    price = format_price(price);
    // Eventually this should have some if's to decide what to concat onto the string based on the rules,
    // but right now, we aren't supporting anything but straight up payments.
    qDebug() << "About to create struct";
    QString s = "A11" + ind + price + QString::number(nummer) + personal + konto + "099000000";
    return s;
}
static QString artema_e_struct(QString price,QString ind,int nummer, QString personal,QString konto) {
    price = format_price(price);
    // Eventually this should have some if's to decide what to concat onto the string based on the rules,
    // but right now, we aren't supporting anything but straight up payments.
    qDebug() << "About to create struct";
    QString s = "E11" + ind + price + QString::number(nummer) + personal + konto + "099000000";
    return s;
}

#endif // PAYLIFE_STRUCTS_H
