#include "artemahybrid.h"
#include "artematimer.h"
#include "cute_credit.h"
#include "helpers.h"
#include "paylife_structs.h"
#include <QDateTime>
#include <QSettings>
#include <QCryptographicHash>

//#define FAIL_TRANSACTIONS // i.e. put this is error testing mode.
ArtemaHybrid::ArtemaHybrid(QObject *parent, QString path) :
    QThread(parent)
{
    this->m_path = path;
    this->m_number = 0;
    this->m_queue = new QMap<int,QString>();
    this->m_ending_day = false;
    this->m_eoc_ending_day = false;
    this->connect(this,SIGNAL(bufferComplete(QString)),SLOT(handleBuffer(QString)));

    this->m_active_id = "";
    //connect(this->EOCTimer,SIGNAL(timedOut()),SLOT(eoctimerfinished()));
    this->u_structs_remaining = 0;
}
void ArtemaHybrid::run() {
    qDebug() << "ArtemaHybrid::run";
    ArtemaTimer * Tpol = new ArtemaTimer(this,5550,"Tpol");               // 5 seconds
    ArtemaTimer * T0 = new ArtemaTimer(this,1790,"T0");                 // 1 second timeout
    ArtemaTimer * T1 = new ArtemaTimer(this,6100,"T1");                 // 6 seconds, repeat timer for send
    ArtemaTimer * T2 = new ArtemaTimer(this,6100,"T2");                 // 6 seconds, repeat timer for receive
    this->JStruct = new ArtemaTimer(this,6100,"JStruct");                 // 6 seconds, repeat timer for receive
    this->StartUpTimer = new ArtemaTimer(this,10000,"StartUpTimer"); // 6 seconds, repeat timer for receive\
    this->EOCTimer = new ArtemaTimer(this,60000,"EOCTimer");
#ifdef FAIL_TRANSACTIONS
    qDebug() << "FAIL_TRANSACTIONS";
     ArtemaTimer * ft = new ArtemaTimer(this,3,"FailTimer");
#endif

    this->m_Reader = new Reader(this,this->m_path,O_RDWR,true);
    this->m_Reader->configure("ArtemaHybrid");
    qDebug() << "ArtemaHybrid::run: Reader configured";
    QString buffer;
    char ENQ = 0x05;
    char STX = 0x02;
    char ETX = 0x03;
    char ACK = 0x06;
    char NAK = 0x15;

    char b;
    bool parity_flag = false;
    int w = -1;

    this->running = true;
    this->m_ready = false;
    this->m_data_to_send = "";
    this->m_terminal_state = "OFFLINE";
    this->m_state = 0;
    this->m_ecr_ready = true;
    this->m_number = 0;
    this->newState(0,"ONLINE");
#ifdef FAIL_TRANSACTIONS
    ft->start();
#endif
    this->StartUpTimer->start();
    while (this->running) {
        if (this->m_ending_day && this->JStruct->timeOut()) {
            this->m_ending_day = false;
        }
        if (this->m_eoc_ending_day && this->EOCTimer->timeOut()) {
            this->m_eoc_ending_day = false;
        }
#ifdef FAIL_TRANSACTIONS
        if (ft->timeOut()) {
            qDebug() << "ft timed out..";
            this->test_suite();
            ft->restart();
        }
#endif
        b = this->read();
        //this->log("State: " + QString::number(this->m_state) + "\n");
        //this->log(b);
        switch(this->m_state) {
            case 0:
                if (bmask(b) == ENQ && checkParity(b)) {
                    this->newState(1,"ONLINE");
                    Tpol->restart();
                } else if (Tpol->timeOut()) {
                    this->newState(0,"OFFLINE");
                    emit pollingTimeOut();
                    Tpol->restart();
                }
                break;
           case 1:
                if (this->m_ready && this->m_data_to_send != "") {
                    w = 0;
                    this->newState(3,"ONLINE");
                } else if (this->m_ecr_ready) {
                    this->send(ACK);
                    this->newState(2,"ONLINE");
                } else {
                    this->m_ready = false;
                    this->newState(0,"OFFLINE");
                }
                break;
           case 2:
                if (Tpol->timeOut()) {
                    qDebug() << "TPol is timed out...";
                    this->newState(0,"OFFLINE");
                    emit pollingTimeOut();
                } else if (bmask(b) == ENQ && checkParity(b)) {
                    this->m_ready = true;
                    Tpol->restart();
                    this->newState(1,"ONLINE");
                } else if (bmask(b) == STX && checkParity(b)) {
                    parity_flag = true;
                    buffer = "";
                    T0->restart();
                    if (!this->StartUpTimer->timeOut())
                        this->StartUpTimer->restart();
                    this->newState(5,"ONLINE");
                }
                break;
            case 3:
                if (w < 3) {
                    this->send(this->m_data_to_send);
                    T1->restart();
                    this->newState(4,"ONLINE");
                } else {
                    this->newState(0,"OFFLINE");
                    this->m_data_to_send = "";
                    this->m_Reader->flush();
                    emit error(this->m_active_id,"ProtocolOffline");
                }
                break;
             case 4:
                if ((bmask(b) == ENQ || bmask(b) == NAK) && checkParity(b)) {
                    w++;
                    this->newState(3,"ONLINE");
                } else if (bmask(b) == ACK && checkParity(b)) {
                      this->newState(0,"ONLINE");
                      emit dataSent(this->m_data_to_send);
                      this->m_data_to_send = "";
                      emit success(this->m_active_id,"DataAck");
                } else if (T1->timeOut()) {
                    emit sendTimeOut();
                    this->newState(0,"OFFLINE");
                    this->m_data_to_send = "";
                    this->m_Reader->flush();
                    emit error(this->m_active_id,"ProtocolOffline");
                }
                break;
             case 5:
                if (b != NULL) {
                    qDebug() << "b: " << QString::number(bmask(b));
                    qDebug() << "b: " << checkParity(b);
                }
                if (T0->timeOut()) {
                    qDebug() << "T0 is timed out...";
                    this->newState(0,this->m_terminal_state);
                    emit receiveTimeOut();
                } else if (bmask(b) == ETX && checkParity(b)) {
                    qDebug() << "Restarting T0";
                    T0->restart();
                    this->newState(6,"ONLINE");
                } else if (this->m_Reader->has_read) {
                    if (!checkParity(b)) {
                        parity_flag = false;
                    }
                    buffer += bmask(b);
                    T0->restart();
                }
                break;
            case 6:

                if (T0->timeOut()) {
                    this->newState(0,this->m_terminal_state);
                    emit receiveTimeOut();
                } else if (this->m_Reader->has_read) {
                    qDebug() << "";
                    // We need to fix checkParity and checkLRC becuase they just aren't working when they should...
                    if (checkParity(b) && checkLRC(bmask(b),calculateLRC(buffer)) && parity_flag) {
                        this->send(ACK);
                        this->newState(0,"ONLINE");
                        qDebug() << "Emitting bufferComplete";
                        this->handleBuffer(buffer);
                        buffer = "";
                    } else {
                        this->send(NAK);
                        T2->restart();
                        this->newState(7, "ONLINE");
                    }
                }
                break;
             case 7:
                if (T2->timeOut()) {
                    this->newState(0,"OFFLINE");
                    emit repeatTimeOut();
                } else if (bmask(b) == STX && checkParity(b)) {
                    T0->restart();
                    parity_flag = true;
                    buffer = "";
                    this->newState(5,"ONLINE");
                } else if (bmask(b) == ENQ && checkParity(b)) {
                    Tpol->restart();
                    this->newState(1,"ONLINE");
                }
                break;
        }
    }
}
void ArtemaHybrid::handleBuffer(QString buffer) {
    qDebug() << "handleBuffer() " << buffer ;
    if (buffer.length() < 1) {
        return;
    }
    if (buffer.at(0) == 'V') {
        return;
    }
    if (this->EOCTimer == NULL) {
        this->EOCTimer = new ArtemaTimer(this,60,"EOCTimer");
    }
    if (this->JStruct == NULL) {
        this->JStruct = new ArtemaTimer(this,6,"JStruct");                 // 6 seconds, repeat timer for receive
    }
    if (this->StartUpTimer == NULL) {
        this->StartUpTimer = new ArtemaTimer(this,10,"StartUpTimer"); // 6 seconds, repeat timer for receive
    }
    // A this point, we need to parse the struct that came back and decide what to do.
    // all we really want to do is find the _nummer field, and link that to the
    // m_queue[] member variable so that we write the response with the correct id
    this->assign_active_id(buffer);
    // Decide if it is a kein ergebnis situation
    // If we are still in startup mode, it should skip this because of course nummer wouldn't match.
    if (buffer.at(0) != 'J' &&
        this->m_nummer != this->m_number)
    {
        emit bytesRead(this->m_active_id,buffer);
        this->buildMessageQuery(buffer);
        emit error(this->m_active_id, "NoResult");
        qDebug() << "Message does not refer to valid nummer...";
        return;
    }

    if (buffer.at(0) == 'P') {
        this->buildMessageQuery(buffer);
        emit bytesRead(this->m_active_id,buffer);
        this->parsePStruct(buffer);
    } else if (buffer.at(0) == 'M'){
        this->buildMessageQuery(buffer);
        emit bytesRead(this->m_active_id,buffer);
        this->parseMStruct(buffer);
    } else if (buffer.at(0) == 'N') {
        this->buildMessageQuery(buffer);
        emit bytesRead(this->m_active_id,buffer);
        this->parseNStruct(buffer);
    } else if (buffer.at(0) == 'J') {
        if (this->m_ending_day) {
            this->JStruct->restart();
        }
        emit bytesRead(this->m_active_id,buffer);
        this->buildMessageQuery(buffer);
        this->parseJStruct(buffer);
    } else if (buffer.at(0) == 'L') {
        this->buildMessageQuery(buffer);
        emit bytesRead(this->m_active_id,buffer);
        this->parseLStruct(buffer);
    } else if (buffer.at(0) == 'T') {
        this->EOCTimer->start();
        this->buildMessageQuery(buffer);
        emit bytesRead(this->m_active_id,buffer);
        this->parseTStruct(buffer);
    } else if (buffer.at(0) == 'U') {
        this->EOCTimer->restart();
        this->buildMessageQuery(buffer);
        emit bytesRead(this->m_active_id,buffer);
        this->parseUStruct(buffer);
        this->u_structs_remaining--;
        if (this->u_structs_remaining <= 0) {
            this->u_structs_remaining = 0;
            this->EOCTimer->kill();
            this->m_eoc_ending_day = false;
            emit success(this->m_active_id,"EOCComplete");
        }
    } else if (buffer.at(0) == 'S') {
        this->EOCTimer->kill();
        this->m_eoc_ending_day = false;
        this->u_structs_remaining == 0;
        this->buildMessageQuery(buffer);
        emit bytesRead(this->m_active_id,buffer);
        this->parseSStruct(buffer);
    } else if (buffer.at(0) == 'V') {
        return;
    } else {
        emit bytesRead(this->m_active_id,buffer);
        this->buildMessageQuery(buffer);
    }
}

void ArtemaHybrid::send(QString data) {
      int len = data.length();
      int i = 0;
      char lrc = 0;
      char stx = 0x02;
      char etx = 0x03;
      char * bytes;
      char buffer[1024];
        qDebug() << "HERE  4";
      bytes = (char *) malloc(sizeof(char) * 1024);
      strcpy(bytes,data.toLatin1().data());
      qDebug() << "HERE  5";
      qDebug() << "PayLife Sending data: " << data << " of len " << QString::number(len) <<
              " " << " Bytes is: " << bytes ;
      //qDebug() << "sprintf 5";
       sprintf(bytes,"%s%c",bytes,etx);
       qDebug() << "HERE  6";
      while (i < len + 1) {
        lrc ^= bytes[i];
        i++;
      }
      qDebug() << "HERE  7";
      //lrc = calclrc(bytes,strlen(bytes));
        //qDebug() << "sprintf 6";
      sprintf(buffer,"%c%s%c",stx,bytes,lrc);
      qDebug() << "HERE  8";
      qDebug() << "\nStarting";
        printf("Writing: [%s]\n", buffer);
        qDebug() << "HERE  9";
      int j;
      for (j = 0; j < len + 3; j++) {
          //buffer[j] = encodeParity(buffer[j]);
      }
      write(this->m_Reader->m_descriptor,buffer,len + 3);
      qDebug() << "Done";
      free(bytes);
}
char ArtemaHybrid::read() {
    char b;

    b = this->m_Reader->read_char();

    return b;
}
void ArtemaHybrid::send(char b) {

    this->m_Reader->write_char(encodeParity(b));
}
void ArtemaHybrid::eod(QString id) {
    this->m_number++;
    if (this->m_number > 9) {
        this->m_number = 0;
    }
    this->m_queue->insert(this->m_number,id);
    QSettings s("JolieRouge","CuteCredit");
    QString konto = s.value("artema_hybrid_konto").toString();
    this->m_data_to_send = artema_e_struct("00000000","2",this->m_number, "00", konto);
}
void ArtemaHybrid::eoc(QString id, QString data) {
    this->m_number++;
    if (this->m_number > 9) {
        this->m_number = 0;
    }
    this->m_queue->insert(this->m_number,id);
    QString n = QString::number(this->m_number);
    QString r_struct = "R 00 000 1" + n + "99 000000 000000 0";
    r_struct = r_struct.replace(" ","");
    this->m_ending_day = true;
    this->m_data_to_send = r_struct;
}
void ArtemaHybrid::sendData(QString id,QString data) {
    qDebug() << "AH Received: " << data;
    if (!this->StartUpTimer->timeOut()) {
        qDebug() << "Startup timer";
         emit wait_(id,this->StartUpTimer->timeRemaining());
         return;
    }

    if (this->m_ending_day && !this->JStruct->timeOut()) {
        emit wait_(id,this->JStruct->timeRemaining());
        return;
    }
    if (this->m_eoc_ending_day && !this->EOCTimer->timeOut()) {
        emit wait_(id,this->EOCTimer->timeRemaining());
    }
    this->m_number++;
    if (this->m_number > 9) {
        this->m_number = 0;
    }
    qDebug() << "nummer inced";
    this->m_queue->insert(this->m_number,id);
    this->m_active_id = id;
    qDebug() << "id inserted";
    // i.e. the string that we get should be like 30.50:U:23:01 as seen in the communication example
    // for the pay command.
    QSettings settings("JolieRouge", "CuteCredit");
    QString konto = settings.value("artema_hybrid_konto").toString();
    QStringList parts = data.split(" ");
    QString personal;
    if (parts.length() < 3) {
        emit error(id,"Not enough arguments sent with PAY command!");
    }
    if (parts.length() >= 4) {
        personal = parts.at(3);
    } else {
       personal = "00";
    }
    QString price = parts.at(2);
    QString ind = settings.value("artema_hybrid_ind").toString();
    qDebug() << "Got This Far";
    if (ind == "") {
        ind = "U";
    }

    if (konto == "") {
        emit error(id,"You must use SET artema_hybrid_konto 00-99 to set your account id for transactions.");
        return;
    }

    if (price == "") {
        emit error(id,"You must send a price with your request to PAY");
    }

    qDebug() << "Parts split and validated";
    // right now, we never send anything but an a struct, even when we are doing a tagesende...
    // artema_a_struct(QString price,QString ind,int nummer, QString personal,QString konto)
    this->m_data_to_send = artema_e_struct(price,ind,this->m_number, personal, konto);
}
void ArtemaHybrid::buildMessageQuery(QString msg) {
    QString query = "INSERT INTO messages VALUES(";
    query += "'" + this->m_active_id + "',";
    query += "'" + msg + "',";
    QDateTime *d = new QDateTime();
    query += QString::number(d->toTime_t()) + ",";
    // i.e. the ind field here
    query += "'',";
    // and the sa field here...
    query += "'" + QString(msg.at(0)) + "',";
    query += "'','','','','','','','','','','','','','','','','','');"; // because we have to insert all fields? Weird...
    emit saveMessage(this->m_active_id,query);

}
void ArtemaHybrid::parsePStruct(QString data) {
    int index;
    QSettings settings("JolieRouge", "CuteCredit");
    QHash<QString,QString> kv;
    kv["store_name"] = settings.value("store_name").toString();
    kv["end_of_receipt"] = settings.value("end_of_receipt").toString();
    kv["m_active_id"] = this->m_active_id;
    QString ind = data.at(3);
    QString p_text;
    for (index = 44; index < 44+16; index++) {
        p_text += data.at(index);
    }
    QString p_ergebnis;
    for (index = 4; index < 4+40; index++) {
        p_ergebnis += data.at(index);
    }
    kv["result"] = split_lines_at_space(p_ergebnis,33).join("${cr}");
    if (QString("467B").indexOf(ind) != -1) {
      QString p_zusatz;
      for (index = 61; index < 61+37; index++) {
          p_zusatz += data.at(index);
      }
      kv["p_zusatz"] = p_zusatz;
    } else {
      kv["p_zusatz"] = "";
    }
    if (ind == "0" || ind == "F") {
      int numrows = QChar::digitValue(data.at(62).unicode());

        QStringList receipt_lines;
        int i = 0;
        QString line;
        int start = 63;
        int offset;
        qDebug() << "Numrows is: " << QString::number(numrows);
        while (i < numrows) {
            line = "";
            offset = (start + (33 * i));
            for (index = offset; index < offset+33; index++) {
                if (index >= data.length()) {
                    break;
                } else {
                 line += data.at(index);
                }
            }
            receipt_lines.append(line);
            i++;
        }
        QString receipt_text = receipt_lines.join("${cr}");
        kv["receipt_text"] = receipt_text;
    } else {
      kv["receipt_text"] = "";
    }
    emit saveMessage(this->m_active_id,update_query("ind", ind, this->m_active_id));
    emit saveMessage(this->m_active_id,update_query("x_text", p_text, this->m_active_id));
    emit saveMessage(this->m_active_id,update_query("result", p_ergebnis, this->m_active_id));

    if (QString("07CF").indexOf(ind) != -1) {
        if (p_text.indexOf("B-K:") != -1 ||
            p_text.indexOf("edc:") != -1 ||
            p_text.indexOf("QCK:") != -1 ||
            p_text.indexOf("KMC:") != -1 ||
            p_text.indexOf("KVI:") != -1 ||
            p_text.indexOf("KDI:") != -1 ||
            p_text.indexOf("KAM:") != -1 ||
            p_text.indexOf("KJB:") != -1)
        {
            if (ind == "0") {
                emit saveMessage(this->m_active_id,update_query("signature_required", data.at(61), this->m_active_id));
                kv["signature_line"] = "Unterschrift:\n\n_____________________________\n\n";
            } else {
                kv["signature_line"] = "\n";
            }

            QString amnt = toNumber(p_text);
            emit paid(this->m_active_id, amnt);
            qDebug() << "amnt is: " << toNumber(amnt);

            qreal total = toNumber(amnt).replace(",",".").toFloat();
            update_daily_totals("artema_hybrid_total",total);
            // now we need to parse p_text
            if ((ind == "0" || ind == "D") && (p_text.indexOf("B-K:") != -1 || p_text.indexOf("edc:") != -1 )) {
              p_text.replace("B-K:","Maestro B E Z A H L T\nB-KASSE:");
              p_text.replace("edc:","Maestro B E Z A H L T\nB-KASSE:");
              
            } else if ((ind == "0" || ind == "D") && p_text.indexOf("QCK") != 01) {
              p_text.replace("QCK:","Quick B E Z A H L T\nB-KASSE:");
            } else if ((ind == "0" || ind == "D") && p_text.indexOf("KMC") != 01) {
              p_text.replace("KMC:","Mastercard B E Z A H L T\nB-KASSE:");
            } else if ((ind == "0" || ind == "D") && p_text.indexOf("KVI") != 01) {
              p_text.replace("KVI:","Visa B E Z A H L T\nB-KASSE:");
            } else if ((ind == "0" || ind == "D") && p_text.indexOf("KDI") != 01) {
              p_text.replace("KDI:","Diner's Club B E Z A H L T\nB-KASSE:");
            } else if ((ind == "0" || ind == "D") && p_text.indexOf("KAM") != 01) {
              p_text.replace("KAM:","American Express B E Z A H L T\nB-KASSE:");
            } else if ((ind == "0" || ind == "D") && p_text.indexOf("KJB") != 01) {
              p_text.replace("KJB:","Jcb B E Z A H L T\nB-KASSE:");
            } else {
              p_text.replace("B-K:", "B-KASSE:");
            }
            if (ind == "D") {
                p_text.replace("B-KASSE:", "STORNO B-KASSE:");
            }
            kv["p_text"] = p_text;
            emit display(this->m_active_id,p_text);
            ReceiptTemplate * r = new ReceiptTemplate(this,"cash_record_p");
            emit print(this->m_active_id,r->parse(kv));
            r = new ReceiptTemplate(this,"credit_card_receipt_p");
            emit print(this->m_active_id,r->parse(kv));
        } else {
            qDebug() << "p_ind is 0, yet didnt match up p_text, p_text is: " << p_text;
        }
    } else if (ind == "2") {
        // in this case, this is a p struct from a successfull End of Day (Tagesende)
        QString amnt = toNumber(p_text);
         qreal total = amnt.toFloat();
         qreal current_total = settings.value("artema_hybrid_total").toReal();
         if (!current_total == total) {
             qreal diff;
             if (total > current_total) {
                // handle the case where the hybrid says it's greater
                diff = total - current_total;
                emit error(this->m_active_id, "Total of transactions is less than total reported by End of Day. Hybrid thinks: " + QString::number(total) +
                                              ", I think:" + QString::number(current_total) +
                                              " which is a difference of: " + QString::number(diff));
             } else {
                 // handle the case where the hybrid amount is less.
                 diff = current_total - total;
                 emit error(this->m_active_id, "Total of transactions is greater than total reported by End of Day. Hybrid thinks: " + QString::number(total) +
                                               ", I think:" + QString::number(current_total) +
                                               " which is a difference of: " + QString::number(diff));
             }
         }
         kv["p_text"] = p_text;
         settings.setValue("artema_hybrid_total",0);
         ReceiptTemplate * r = new ReceiptTemplate(this,"cash_record_p");
         emit print(this->m_active_id,r->parse(kv));
     } else {
        qDebug() << "Oops..." << data.at(3);
     }
}
void ArtemaHybrid::parseMStruct(QString m) {
        QString ind = m.at(3);
        QSettings s("JolieRouge","CuteCredit");
        QHash<QString,QString> kv;
        kv["store_name"] = s.value("store_name").toString();
        kv["end_of_receipt"] = s.value("end_of_receipt").toString();
        kv["m_active_id"] = this->m_active_id;
        emit saveMessage(this->m_active_id,update_query("ind", ind, this->m_active_id));
        int index;

        QString name;
        for (index = 5; index < 5+20; index++) {
            name += m.at(index);
        }
        kv["name"] = name ;
        emit saveMessage(this->m_active_id,update_query("name", name, this->m_active_id));

        QString card;
        for (index = 25; index < 25+21; index++) {
            card += m.at(index);
        }
        kv["card"] = card ;
        emit saveMessage(this->m_active_id,update_query("card", card, this->m_active_id));

        QString expiry_date;
        for (index = 46; index < 46+5; index++) {
            expiry_date += m.at(index);
        }
        kv["expiry_date"] = expiry_date;
        emit saveMessage(this->m_active_id,update_query("expiry_date", expiry_date, this->m_active_id));
        kv["lese"] = m.at(51);

        QString uid;
        for (index = 52; index < 52+16; index++) {
            uid += m.at(index);
        }
        kv["uid"] = uid;
        emit saveMessage(this->m_active_id,update_query("uid", uid, this->m_active_id));

        QString tid;
        for (index = 68; index < 68+8; index++) {
            tid += m.at(index);
        }
        kv["tid"] = tid ;
        emit saveMessage(this->m_active_id,update_query("tid", tid, this->m_active_id));

        QString receipt_id;
        for (index = 76; index < 76+6; index++) {
            receipt_id += m.at(index);
        }
        kv["receipt_id"] = receipt_id ;
        emit saveMessage(this->m_active_id,update_query("receipt_id", receipt_id, this->m_active_id));

        QString authorization_number;
        for (index = 82; index < 82+6; index++) {
            authorization_number += m.at(index);
        }
        kv["genehm"] = authorization_number;
        emit saveMessage(this->m_active_id,update_query("auth", authorization_number, this->m_active_id));

        QString result;
        for (index = 88; index < 88+20; index++) {
            result += m.at(index);
        }
        kv["result"] = result ;
        emit saveMessage(this->m_active_id,update_query("result", result, this->m_active_id));

        QString total;
        for (index = 108; index < 108+8; index++) {
            total += m.at(index);
        }
        emit saveMessage(this->m_active_id,update_query("total", total, this->m_active_id));


        total = total.trimmed();
        qreal f_total = this->total2qreal(total);
        kv["total"] = QString::number(f_total) ;
        int n_ind = ind.toInt();
        if (n_ind < 8) {
            // it's a positive transaction
            update_daily_totals("artema_hybrid_cc_total",f_total);
            emit paid(this->m_active_id,cents2float(total));
        } else {
            update_daily_totals("artema_hybrid_cc_total",f_total * -1);
            emit success(this->m_active_id,cents2float(total) + " Refunded.");
        }

        QString text;
        for (index = 130; index < 130+16; index++) {
            text += m.at(index);
        }
        kv["text"] = text ;

        emit saveMessage(this->m_active_id,update_query("x_text", text, this->m_active_id));
        emit saveMessage(this->m_active_id,update_query("customer_display_text", text, this->m_active_id));

        QString transactions_remaining;
        for (index = 146; index < 146+4; index++) {
            transactions_remaining += m.at(index);
        }
        if (transactions_remaining.toInt() <= 10) {
            emit notify(this->m_active_id,"Only " + transactions_remaining + " transactions remaining before EOD required.");
        }
        emit saveMessage(this->m_active_id,update_query("transactions_remaining", transactions_remaining, this->m_active_id));

        QString signature_required = m.at(150);
        if (signature_required == "1") {
          kv["signature_line"] = "Unterschrift: \n\n______________________________";
        } else {
          kv["signature_line"] = "";
        }
        emit saveMessage(this->m_active_id,update_query("signature_required", signature_required, this->m_active_id));
        int numrows = QChar::digitValue(m.at(151).unicode());

        QStringList receipt_lines;
        int i = 0;
        QString line;
        int start = 152;
        int offset;
        while (i < numrows) {
            line = "";
            offset = (start + (33 * i));
            for (index = offset; index < offset+33; index++)
                line += m.at(index);
            receipt_lines.append(line);
            i++;
        }
        QString receipt_text = receipt_lines.join("${cr}");
        kv["belegzeile"] = receipt_text;
        emit saveMessage(this->m_active_id,update_query("receipt_text", receipt_text, this->m_active_id));
        ReceiptTemplate * r = new ReceiptTemplate(this,"cash_record");
        emit print(this->m_active_id,r->parse(kv));
        r = new ReceiptTemplate(this,"credit_card_receipt");
        emit print(this->m_active_id, r->parse(kv));
}
void ArtemaHybrid::parseLStruct(QString l) {
    QString ind = l.at(3);
    int index;
    QString ind_error;
    if (ind == "0")
        ind_error = "ProtocolError";
    if (ind == "1")
        ind_error = "RequestError";
    if (ind == "2")
        ind_error = "CardReadingError";
    if (ind == "3")
        ind_error = "ConnectionInterruption";
    if (ind == "4")
        ind_error = "RequestError";
    if (ind == "5")
        ind_error = "ComputationError";
    if (ind == "6")
        ind_error = "ExternalCancellation";
    if (ind == "7")
        ind_error = "CardBlackListed";
    if (ind == "8")
        ind_error = "AmountCancellation";
    if (ind == "9")
        ind_error = "BusinessCaseError";
    if (ind == "A")
        ind_error = "UserCancellation";
    if (ind == "F")
        ind_error = "ATMOccupied";
    if (ind == "K")
        ind_error = "ManualCardDataRequired";
    if (ind == "T")
        ind_error = "InternalTimeout";
    if (ind == "V")
        ind_error = "MemoryFull";
    int number = QChar::digitValue(ind.at(0).unicode());
    emit error(this->m_active_id,ind_error);
    QString text;
    for (index = 5; index < 5+16; index++) {
        text += l.at(index);
    }
    emit fail(this->m_active_id,text);
    QString l_display;
    for (index = 22; index < 22+20; index++) {
        l_display += l.at(index);
    }
    emit display(this->m_active_id,l_display);
}
void ArtemaHybrid::parseNStruct(QString n) {
    QString ind = n.at(3);
    int index;
    QString n_text;
    for (index = 4;index < 4+16; index++)
        n_text += n.at(index);
    if (n_text.at(0) == '*') {
        qDebug() << "Try again";
    }
    QString ind_error;
    if (ind == "0")
        ind_error = "OperatingError";
    if (ind == "1")
        ind_error = "ReadingError";
    if (ind == "2")
        ind_error = "LineProblem";
    if (ind == "3")
        ind_error = "ProcessingProblem";
    if (ind == "4")
        ind_error = "DenialOrderNotAllowed";
    if (ind == "5")
        ind_error = "DenialMissingAuthorization";
    if (ind == "6")
        ind_error = "DenialUnknownCard";
    if (ind == "7")
        ind_error = "AbortRemovableDeviceError";
    if (ind == "8")
        ind_error = "AbortDeviceDefectSuspension";
    if (ind == "9")
        ind_error = "AbortNoTESSum";
    if (ind == "A")
        ind_error = "AbortNoTESSumLineProblem";
    if (ind == "B")
        ind_error = "AbortLineProblemVoiceAuth";
    if (ind == "G")
        ind_error = "BusinessCaseBlocked";
    if (ind == "J")
        ind_error = "NoEActivityLog";
    emit saveMessage(this->m_active_id,update_query("x_text", n_text, this->m_active_id));
    emit saveMessage(this->m_active_id,update_query("ind", ind, this->m_active_id));
    emit fail(this->m_active_id,n_text);
    emit error(this->m_active_id,ind_error);
}
void ArtemaHybrid::parseJStruct(QString j) {
  int chars = 33;
  if (j.at(3) == '1') {
    chars = 16;
  }
  int i;
  int x = 0;
  QStringList lines;
  QString line;
  for (i = 4; i < j.length(); i++) {
    line += j.at(i);
    x++;
    if (x == chars) {
      lines.append(line);
      line = "";
    }
  }
  QString receipt_text = lines.join("<CR>");
  emit saveMessage(this->m_active_id,update_query("receipt_text", receipt_text, this->m_active_id));
}
void ArtemaHybrid::parseTStruct(QString data) {
    int index;
    QString ind = data.at(3);
    QHash<QString,QString> kv;
    emit saveMessage(this->m_active_id,update_query("ind",ind,this->m_active_id));
    QString tid;
    for (index = 5; index < 5+8; index++) {
        tid += data.at(index);
    }
    kv["t_tid"] = tid;
    emit saveMessage(this->m_active_id,update_query("tid",tid,this->m_active_id));
    QString tcz;
    for (index = 13; index < 13+2; index++) {
        tcz += data.at(index);
    }
    emit saveMessage(this->m_active_id,update_query("auth",tcz,this->m_active_id));
    QString t_text;
    for (index = 15; index < 15+16; index++) {
        t_text += data.at(index);
    }
    emit saveMessage(this->m_active_id,update_query("x_text",t_text,this->m_active_id));
    QString t_anz;
    for (index = 31; index < 31+2; index++) {
        t_anz += data.at(index);
    }
    this->u_structs_remaining = t_anz.toInt();
    qDebug() << "u_structs_remaining set to: " << QString::number(this->u_structs_remaining);
    emit saveMessage(this->m_active_id,update_query("transactions_remaining",t_anz,this->m_active_id));
    QString t_kauf;
    for (index = 33; index < 33+20; index++) {
        t_kauf += data.at(index);
    }
    this->t_kauf = t_kauf;
    QString t_storno;
    for (index = 53; index < 53+20; index++) {
        t_storno += data.at(index);
    }
    this->t_storno = t_storno;
    QString t_gutschrift;
    for (index = 73; index < 73+20; index++) {
        t_gutschrift += data.at(index);
    }
    this->t_gutschr = t_gutschrift;
    QString user_display_text = t_kauf + "," + t_storno + "," + t_gutschrift;
    emit saveMessage(this->m_active_id,update_query("user_display_text",user_display_text,this->m_active_id));
    ReceiptTemplate * r = new ReceiptTemplate(this,"t_struct");
    emit print(this->m_active_id,r->parse(kv));
}
void ArtemaHybrid::parseUStruct(QString data) {
    int index;
    QString ind = data.at(3);
    emit saveMessage(this->m_active_id,update_query("ind",ind,this->m_active_id,"U"));
    QHash<QString,QString> sv;
    sv["t_kauf"] = this->t_kauf;
    sv["t_storno"] = this->t_storno;
    sv["t_gutschr"] = this->t_gutschr;
    QString u_satz;
    for (index = 5; index < 5+2; index++) {
        u_satz += data.at(index);
    }
    QString u_name;
    for (index = 7; index < 7+20; index++) {
        u_name += data.at(index);
    }
    sv["u_name"] = u_name;
    emit saveMessage(this->m_active_id,update_query("name",u_name,this->m_active_id,"U"));
    QString u_uid;
    for (index = 27; index < 27+16; index++) {
        u_uid += data.at(index);
    }
    sv["u_uid"] = u_uid;
    emit saveMessage(this->m_active_id,update_query("name",u_name,this->m_active_id,"U"));
    emit saveMessage(this->m_active_id,update_query("name",u_name,this->m_active_id,"U"));
    QString u_result;
    for (index = 43; index < 43+20; index++) {
        u_result += data.at(index);
    }
    sv["u_result"] = u_result;
    emit saveMessage(this->m_active_id,update_query("result",u_result,this->m_active_id,"U"));
    QString u_datum;
    for (index = 63; index < 63+6; index++) {
        u_datum += data.at(index);
    }
    sv["u_datum"] = u_datum;

    QString u_zeit;
    for (index = 69; index < 69+6; index++) {
        u_zeit += data.at(index);
    }
    sv["u_zeit"] = u_zeit;

    if (ind != "4") {
        QString u_sum_vz = data.at(75);
        QString u_sum;
        sv["u_sum_vz"] = u_sum_vz;
        for (index = 76; index < 76+10; index++) {
            u_sum += data.at(index);
        }
        sv["u_sum"] = cents2float(u_sum);
    } else {
        sv["u_sum_vz"] = "";
        sv["u_sum"] = "";
    }

    QString u_ka_betr;
    for (index = 86; index < 86+10; index++) {
        u_ka_betr += data.at(index);
    }
    sv["u_ka_betr"] = cents2float(u_ka_betr);
    QString u_ka_anz;
    for (index = 96; index < 96+4; index++) {
        u_ka_anz += data.at(index);
    }
    sv["u_ka_anz"] = u_ka_anz;

    QString u_st_betr;
    for (index = 100; index < 100+10; index++) {
        u_st_betr += data.at(index);
    }
    sv["u_st_betr"] = cents2float(u_st_betr);
    QString u_st_anz;
    for (index = 110; index < 110+4; index++) {
        u_st_anz += data.at(index);
    }
    sv["u_st_anz"] = u_st_anz;

    QString u_gu_betr;
    for (index = 114; index < 114+10; index++) {
        u_gu_betr += data.at(index);
    }
    sv["u_gu_betr"] = cents2float(u_gu_betr);
    QString u_gu_anz;
    for (index = 124; index < 124+4; index++) {
        u_gu_anz += data.at(index);
    }
    sv["u_gu_anz"] = u_gu_anz;

    if (data.length() >= 129) {
        QString u_diff_vz = data.at(128);
        sv["u_diff_vz"] = u_diff_vz;
    } else {
        sv["u_diff_vz"] = "";
    }


    if (data.length() >= 139) {
        QString u_diff;
        for (index = 129; index < 129+10; index++) {
            u_diff += data.at(index);
        }
        sv["u_diff"] = cents2float(u_diff);
    } else {
        sv["u_diff"] = "";
    }
    ReceiptTemplate *r = new ReceiptTemplate(this,"u_struct");
    emit print(this->m_active_id,r->parse(sv));
}
void ArtemaHybrid::parseSStruct(QString data) {
    QString ind = data.at(3);
    int index;
    QString s_text;
    for (index = 5;index < 5+16; index++)
        s_text += data.at(index);
    QString ind_error;
    if (ind == "0")
        ind_error = "ProtocolError";
    if (ind == "1")
        ind_error = "RequestError";
    if (ind == "2")
        ind_error = "CardReadError";
    if (ind == "3")
        ind_error = "LineProblem";
    if (ind == "4")
        ind_error = "ProcessingProblem";
    if (ind == "5")
        ind_error = "InternalAbort";
    if (ind == "9")
        ind_error = "BusinessCaseError";
    if (ind == "A")
        ind_error = "OperatorAbort";
    if (ind == "E")
        ind_error = "CollectResultOfClosing";
    if (ind == "F")
        ind_error = "DeviceBusy";
    if (ind == "T")
        ind_error = "InternalErrorTimeout";
}

void ArtemaHybrid::ArtemaHybrid::test_suite() {
    qDebug() << "testing t_struct";
    this->m_number = 0;
    this->test_t_struct();
    sleep(1);
    this->test_u_struct();

}
void ArtemaHybrid::log(char b) {
    QFile f("/var/log/paylife.log");
    QString c;
    switch(bmask(b)) {
        case 0x05: {
             c = "ENQ";
             break;
        }
        case 0x15: {
             c = "NAK";
             break;
        }
        case 0x03: {
            c = "ETX";
            break;
        }
        case 0x02: {
            c = "STX";
            break;
        }
        case NULL: {
           return;
            break;
        }
        default:
            c += b;
            break;
    }
    if (f.open(QIODevice::Append)) {
        QTextStream out(&f);
        out << "Read Byte is: '" << QString::number(bmask(b)) << "'\n";
        f.close();
    }
}
void ArtemaHybrid::log(QString s) {
    QFile f("/var/log/paylife.log");
    if (f.open(QIODevice::Append)) {
        QTextStream out(&f);
        out << s;
        f.close();
    }
}
