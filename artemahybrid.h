#ifndef ARTEMAHYBRID_H
#define ARTEMAHYBRID_H

#include <QThread>
#include <QCryptographicHash>
#include "cute_credit.h"
#include <QFile>
#include <QTextStream>
// See bottom of file for testing methods
class ArtemaHybrid : public QThread
{
    Q_OBJECT
public:
    explicit ArtemaHybrid(QObject *parent = 0, QString path = "");
    void run();
    QString m_active_id;
    void test_suite();
    QString t_kauf;
    QString t_storno;
    QString t_gutschr;

    void log(char b);
    void log(QString s);
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
    int m_nummer;
    QMap<int, QString> * m_queue;
    bool m_ending_day;
    bool m_eoc_ending_day;
    ArtemaTimer * JStruct;
    ArtemaTimer * StartUpTimer;
    ArtemaTimer * EOCTimer;
    ArtemaTimer * KeinTimer;
    int u_structs_remaining;
    QSqlQuery m_insert_msg;
    QSettings s;


signals:
    void stateChange(int,QString);
    void pollingTimeOut();
    void receiveTimeOut();
    void repeatTimeOut();
    void sendTimeOut();
    void dataSent(QString);
    void bytesRead(QString,QString);
    void bufferComplete(QString);
    void saveMessage(QString,QString);
    void print(QString,QString);
    void display(QString,QString);
    void paid(QString, QString);
    void success(QString, QString);
    void fail(QString,QString);
    void wait_(QString,int); // so named to not conflict with the QThread method
    void error(QString, QString);
    void notify(QString, QString);
private slots:
    void eoctimerfinished() {
        this->m_eoc_ending_day = false;
    }
    void send(char b);
    void send(QString buffer);
    char read();
    void eod(QString id);
    void eoc(QString id, QString data);
    void handleBuffer(QString buffer);
    void assign_active_id(QString buffer) {
        this->m_nummer = -1;
        if (buffer.at(0) == 'P') {
            this->m_nummer = QChar::digitValue(buffer.at(60).unicode());
            this->m_active_id = this->m_queue->value(this->m_nummer);
        } else if (buffer.at(0) == 'M' || buffer.at(0) == 'T' || buffer.at(0) == 'U' || buffer.at(0) == 'H' || buffer.at(0) == 'L' || buffer.at(0) == 'S'){
            this->m_nummer = QChar::digitValue(buffer.at(4).unicode());
            this->m_active_id = this->m_queue->value(this->m_nummer);
        } else if (buffer.at(0) == 'N') {
            this->m_nummer = QChar::digitValue(buffer.at(20).unicode());
            this->m_active_id = this->m_queue->value(this->m_nummer);
            if (this->m_active_id == "") {
                qDebug() << "m_numer: " << QString::number(this->m_number) << " m_nummer: " << QString::number(this->m_nummer);
            }
        }
        if (this->m_active_id == "") {
            this->m_active_id = QCryptographicHash::hash(buffer.toAscii(),QCryptographicHash::Md5).toHex().constData();
        }
    }

    void newState(int s,QString ts) {
        if (s != this->m_state || ts != this->m_terminal_state) {
            this->m_state = s;
            this->m_terminal_state = ts;
            emit stateChange(s,ts);
            qDebug() << "State is: " << QString::number(s);
        }
    }

    /*
        TESTING METHODS
    */
    // M1121MASTERCARD          **** **** **** 6511  10/167300311358    20107970113000099000000B E Z A H L T       0000329500000000000000BEZAHLT!        055211EA0000000041010               C01
    void test_m_struct() {
        this->m_queue->insert(0,"TESTMSTRUCT");
        QString m_struct = "M1121MASTERCARD          **** **** **** 6511  10/167300311358    20107970113000091764528B E Z A H L T       0000329500000000000000BEZAHLT!        056011EA0000000041010               C01";
        m_struct.replace(4,1,"0");
        emit bufferComplete(m_struct);
    }
    // P11007970113 000100 000170 20120115 121953  B-K: EUR   32,95202PAN:***************0487 12/14 D01EA0000000043060
    void test_p_struct() {
        QString p_struct = "P11007970113 000100 000170 20120115 121953  B-K: EUR   32,95202PAN:***************0487 12/14 D01EA0000000043060";
        p_struct.replace(60,1,"0");
        emit bufferComplete(p_struct);
    }
    // N000KARTE ENTNEHMEN!1
    void test_n_struct() {
        this->m_queue->insert(0,"TESTNSTRUCT");
        QString s;
        //if (buffer.at(0) == 'P') {
                int low = 0;
                int high = 9;
                int r = qrand() % ((high + 1) - low) + low;
                s = "N00012345678901234560";
                s.replace(3,1,QString::number(r));
                if (r < 7) {
                    s.replace(4,1,"*");
                }
        //    }
        emit bufferComplete(s);
    }
    void test_t_struct() {
        this->m_queue->insert(0,"TESTTSTRUCT");
        QString t_struct = "T00100550100099KASSENABSCHLUSS!01B E Z A H L T       STORNO              GUTSCHRIFT          ";
        t_struct.replace(4,1,"0");
        emit bufferComplete(t_struct);
    }
    void test_u_struct() {

        // this should always be called with test_t_struct
        QString msg1 = "U013302VISA PLBE           300311358                           111115000000+0000036089000003608900200000000000000000000000000000-0000036089";
        msg1.replace(4,1,"0");
        emit bufferComplete(msg1);
        QString msg2 = "U013301MASTERCARD          300311358                           111115000000+0000135603000013560300300000000000000000000000000000-0000135603";
        msg1.replace(4,1,"0");
        emit bufferComplete(msg2);
    }

    /* J000001PPA 000100 000170 EUR 00003295
    002 000000300311358      07970113
    003 **** **** **** ***0 * E01  01
    */
    /*
        END OF TESTING METHODS
    */
public slots:
    void sendData(QString id,QString data);
    void buildMessageQuery(QString msg);
    void parsePStruct(QString data);
    void parseMStruct(QString data);
    void parseLStruct(QString data);
    void parseNStruct(QString data);
    void parseJStruct(QString data);
    void parseTStruct(QString data);
    void parseUStruct(QString data);
    void parseSStruct(QString data);
    qreal total2qreal(QString ttl) {
        qreal total = ttl.toFloat() / 100;
        return total;
    }
};


// >Testing methods

static QString testing_fail_struct(QString buffer) {
    QString s;
//if (buffer.at(0) == 'P') {
        int low = 0;
        int high = 9;
        int r = qrand() % ((high + 1) - low) + low;
        s = "N00012345678901234567";
        s.replace(3,1,QString::number(r));
        if (r < 7) {
            s.replace(4,1,"*");
        }
//    }
    return s;
}


#endif // ARTEMAHYBRID_H
