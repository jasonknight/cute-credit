#ifndef ARTEMAHYBRID_H
#define ARTEMAHYBRID_H

#include <QThread>
#include "cute_credit.h"
#include "reader.h"
class ArtemaHybrid : public QThread
{
    Q_OBJECT
public:
    explicit ArtemaHybrid(QObject *parent = 0, QString path = "");
    void run();
    QString m_active_id;
private:
    // member variables
    int m_state;
    QString m_terminal_state;
    bool m_ecr_ready;
    bool running;
    bool m_ready;
    QString m_data_to_send;
    QString m_path;
    Reader * m_Reader;
    int m_number;
    QMap<int, QString> * m_queue;


signals:
    void stateChange(int,QString);
    void pollingTimeOut();
    void receiveTimeOut();
    void repeatTimeOut();
    void sendTimeOut();
    void dataSent(QString);
    void bytesRead(QString,QString);
    void saveMessage(QString,QString);
    void print(QString,QString);
    void display(QString,QString);
    void paid(QString, QString);
    void success(QString, QString);
    void fail(QString,QString);
    void wait_(QString); // so named to not conflict with the QThread method
    void error(QString, QString);
    void notify(QString, QString);
private slots:
    void send(char b);
    void send(QString buffer);
    char read();
    void newState(int s,QString ts) {
        if (s != this->m_state || ts != this->m_terminal_state) {
            this->m_state = s;
            this->m_terminal_state = ts;
            emit stateChange(s,ts);
            qDebug() << "State is: " << QString::number(s);
        }
    }
public slots:
    void sendData(QString id,QString data);
    void buildMessageQuery(QString msg);
    void parsePStruct(QString data);
    void parseMStruct(QString data);
    void parseLStruct(QString data);
    void parseNStruct(QString data);
    qreal total2qreal(QString ttl) {
        qreal total = ttl.toFloat() / 100;
        return total;
    }
};

#endif // ARTEMAHYBRID_H
