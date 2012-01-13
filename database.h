#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlResult>
#include <QSqlError>
#include <QFile>
#include <QSqlQuery>
class Database : public QObject
{
    Q_OBJECT
public:
    Database(QObject *parent = 0);
    ~Database();

signals:
    void queryComplete(QString id, QSqlQuery q);
public slots:
    void runQuery(QString id,QString query);
    QSqlQuery runQuery(QString query);
private:
    QSqlDatabase db;

};

#endif // DATABASE_H
