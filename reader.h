#ifndef READER_H
#define READER_H

#include <QObject>
#include "cute_credit.h"
class Reader : public QObject
{
    Q_OBJECT
public:
    explicit Reader(QObject *parent = 0, QString path = "", int mode = O_RDWR, bool keepalive = true);
    QString m_path;
    bool m_keepalive;
    int m_mode;
#ifdef LINUX
    int m_descriptor;
#endif
    char read_char();
    QString read_string(int len);
    bool write_char(char b);
    bool write_string(QString data);
    bool write_string(const char * data);
signals:
    void charRead(QChar b);
    void charWritten(QChar b);
public slots:
    void configure(QString type);


};

#endif // READER_H
