#include "database.h"
#include <QDir>
#include <QFile>
#include <QSqlQuery>
#include <QDebug>
Database::Database(QObject *parent) :
    QObject(parent)
{
    this->db = QSqlDatabase::addDatabase("QSQLITE");
    QString path(QDir::home().path());
    path.append(QDir::separator()).append(".cute_credit.sqlite");
    path = QDir::toNativeSeparators(path);
    db.setDatabaseName(path);
    if (!this->db.open()) {
      printf("Could not create database!");
      exit(-1);
  } else {
    QSqlQuery q;
    QString sql = "";
    sql +=       "CREATE TABLE messages (ref_id VARCHAR(255), ";
    sql +=       "msg TEXT, ";
    sql +=       "sent_at INTEGER, ";
    sql +=       "ind VARCHAR(255),";
    sql +=       "name VARCHAR(255), ";
    sql +=       "card VARCHAR(255), " ;
    sql +=       "expiry_date VARCHAR(255), ";
    sql +=       "uid VARCHAR(255), ";
    sql +=       "tid VARCHAR(255), " ;
    sql +=       "receipt_id VARCHAR(255), ";
    sql +=       "auth VARCHAR(255), ";
    sql +=       "total VARCHAR(255), ";
    sql +=       "result TEXT,";
    sql +=       "x_text TEXT, ";
    sql +=       "transactions_remaining TEXT, ";
    sql +=       "customer_display_text TEXT, ";
    sql +=       "user_display_text TEXT, ";
    sql +=       "signature_required VARCHAR(5), ";
    sql +=       "receipt_text TEXT );";
    if (!q.exec(sql)) {
        qDebug() << "Creat Table Statement invalid!" << q.lastError().text();
    }

  }

}
Database::~Database() {
    this->db.close();
}
void Database::runQuery(QString id, QString query) {
    qDebug() << "Database received: " << query;
    QSqlQuery q;
    if (q.exec(query)) {
        emit queryComplete(id,q);
    } else {
        qDebug() << "There was a query Error" << q.lastError().text();
        emit queryComplete(id,q.lastError().text());
    }
}
QSqlQuery Database::runQuery(QString query) {
    qDebug() << "Database received Direct: " << query;
    QSqlQuery q;
    q.exec(query);
    return q;
}
