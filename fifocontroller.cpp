#include "fifocontroller.h"

#include "reader.h"
#include "cute_credit.h"
#include <QSettings>
FIFOController::FIFOController(QObject *parent) :
    QThread(parent)
{
    this->running = false;
    this->m_ids = new QStringList();
}
void FIFOController::run() {
    this->running = true;

    QString str;
    QSettings settings("JolieRouge", "CuteCredit");
    FILE *fp;
    char readbuf[512];
    this->w = new Reader(this,"/tmp/CUTE_CREDIT_OUT", O_WRONLY,false);
    //configureFIFO(this->r->m_descriptor);
    //configureFIFO(this->w->m_descriptor);
    while(this->running) {
        fp = fopen("/tmp/CUTE_CREDIT_IN","r");
        fgets(readbuf,512,fp);
        fclose(fp);
        str = QString(readbuf);
        qDebug() << "FIFO Read: " << str;
        if (str.length() < 4) {
            continue;
        }
        QStringList cmds = str.split(" ");
        if (cmds.length() < 3)
            continue;
        QString command = cmds.at(0);
        QString id = cmds.at(1);
        QString data = cmds.at(2);
        if (command.length() > 1) {
            if (command == "TEST") {
                qDebug() << "Read: " << str;
                this->w->write_string("RECV " + id + "Hello World!\x04");
            } else if (command == "PAY") {
                qDebug() << "Sending Data";
                emit send(id,str);
            } else if (command == "QUERY") {
                qDebug() << "Sending Query";
                this->m_ids->append(id);
                // now we need to parse the query
                int index = str.indexOf(id) + id.length() + 1;
                QString query;
                qDebug() << "Index is: " << QString::number(index);
                while (index < str.length()) {
                    qDebug() << str.at(index);
                    query += str.at(index);
                    index++;
                }
                qDebug() << "Query is: " << query;
                emit runQuery(id,query);
            } else if (command == "SET") {
                int index = str.indexOf(id) + id.length() + 1;
                QString value;
                while (index < str.length()) {
                    value += str.at(index);
                    index++;
                }
                settings.setValue(id,value);
                this->success(id," set to " + value);
            } else if (command == "GET") {
                this->success(id,"value=" + settings.value(id).toString());
            }
        } else {
            qDebug() << "No match for " << str;
        }

    }
}

void FIFOController::writeData(QString data) {
    this->w->write_string(data);
}
void FIFOController::recvMsg(QString id, QString data) {
    qDebug() << "Error: recvMSG is defunct...";
}
void FIFOController::queryComplete(QString id, QSqlQuery q) {
    qDebug() << "Received query result...";
    if (q.isSelect() && this->m_ids->indexOf(id) != -1) {
        int id_field = q.record().indexOf("id");
        int msg = q.record().indexOf("msg");
        int sent_at = q.record().indexOf("sent_at");
        while(q.next()) {
            QString res = "RECV " + id + " " + q.value(id_field).toString();
            res += " " + q.value(sent_at).toString();
            res += " " + q.value(msg).toString() + "\n";
            this->w->write_string(res);
        }
        int index = this->m_ids->indexOf(id);
        this->m_ids->removeAt(index);
    }
}
void FIFOController::display(QString id, QString data) {
    this->writeData("DISP " + id + " " + data + 0x04);
}
void FIFOController::print(QString id, QString data) {
    this->writeData("PRINT " + id + " " + data + 0x04);
}
void FIFOController::paid(QString id, QString amount) {
    this->writeData("PAID " + id + " " + amount + 0x04);
}
void FIFOController::success(QString id, QString data) {
    this->writeData("SUCCESS " + id + " " + data + 0x04);
}
void FIFOController::fail(QString id, QString data) {
    this->writeData("FAIL " + id + " " + data + 0x04);
}
void FIFOController::wait(QString id) {
    this->writeData("WAIT " + id + 0x04);
}
void FIFOController::error(QString id, QString data) {
    this->writeData("ERROR " + id + " " + data + 0x04);
}
void FIFOController::notify(QString id, QString data) {
    this->writeData("NOTIFY " + id + " " + data + 0x04);
}
