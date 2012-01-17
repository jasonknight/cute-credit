#ifndef RECEIPTTEMPLATE_H
#define RECEIPTTEMPLATE_H

#include <QObject>
#include <QHash>

class ReceiptTemplate : public QObject
{
    Q_OBJECT
public:
    explicit ReceiptTemplate(QObject *parent = 0, QString name = "");
    QString m_name;
    QString m_path;
private:
    QString p_template;
signals:

public slots:
    QString parse(QHash<QString,QString>& key_values);
    QString parseSpecialCodes(QString tmp_string);
    QString parseSpecialCodesInLine(QString txt);
};

#endif // RECEIPTTEMPLATE_H
