#ifndef FIFOCONTROLLER_H
#define FIFOCONTROLLER_H

#include <QThread>
#include "cute_credit.h"
#include "reader.h"
#include <QSqlQuery>
class Reader;
class FIFOController : public QThread
{
    Q_OBJECT
public:
    explicit FIFOController(QObject *parent = 0);
    void run();
    Reader * r;
    Reader * w;
    bool running;
    QStringList * m_ids;
signals:
    void dataRead();
    void send(QString,QString);
    void dataWritten();
    void runQuery(QString id, QString query);

public slots:
    void recvMsg(QString id, QString data);
    void writeData(QString data);
    void queryComplete(QString id, QSqlQuery q);
    void display(QString id,QString data);
    void print(QString id,QString data);
    void paid(QString id,QString data);
    void fail(QString id, QString data);
    void wait(QString id);
    void success(QString id, QString data);
    void error(QString id, QString data);
    void notify(QString id, QString data);
};

#endif // FIFOCONTROLLER_H
