#include "artemahybrid.h"
#include "artematimer.h"
#include "cute_credit.h"
#include "helpers.h"
#include "paylife_structs.h"
#include <QDateTime>
#include <QSettings>
ArtemaHybrid::ArtemaHybrid(QObject *parent, QString path) :
    QThread(parent)
{
    this->m_path = path;
    this->m_number = 0;
    this->m_queue = new QMap<int,QString>();
}
void ArtemaHybrid::run() {
    qDebug() << "ArtemaHybrid::run";
    ArtemaTimer * Tpol = new ArtemaTimer(this,5000,"Tpol");               // 5 seconds
    ArtemaTimer * T0 = new ArtemaTimer(this,1000,"T0");                 // 1 second timeout
    ArtemaTimer * T1 = new ArtemaTimer(this,6000,"T1");                 // 6 seconds, repeat timer for send
    ArtemaTimer * T2 = new ArtemaTimer(this,6000,"T2");                 // 6 seconds, repeat timer for receive

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

    while (this->running) {
        qDebug() << "Running";
        b = this->read();
        switch(this->m_state) {
            case 0:
                if (bmask(b) == ENQ && checkParity(b)) {
                    this->newState(1,"ONLINE");
                } else if (Tpol->timeOut()) {

                    this->newState(0,"OFFLINE");
                    emit pollingTimeOut();
                }
                Tpol->restart();
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
                    this->m_state = 1;
                } else if (bmask(b) == STX && checkParity(b)) {
                    parity_flag = true;
                    buffer = "";
                    this->newState(5,"ONLINE");
                }
                break;
            case 3:
                if (w < 3) {
                    qDebug() << "\n\n";
                    //this->send(STX);
                    this->send(this->m_data_to_send);
                    //this->send(ETX);
                    //this->send(calculateLRC(this->m_data_to_send));
                     qDebug() << "\n\n";
                    T1->restart();
                    this->newState(4,"ONLINE");
                } else {
                    this->newState(0,"OFFLINE");
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
                } else if (T1->timeOut()) {
                    emit sendTimeOut();
                    this->newState(0,"OFFLINE");
                }
                break;
             case 5:
                if (T0->timeOut()) {
                    this->newState(0,this->m_terminal_state);
                    emit receiveTimeOut();
                } else if (bmask(b) == ETX && checkParity(b)) {
                    T0->restart();
                    this->newState(6,"ONLINE");
                } else if (b != NULL) {
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
                } else if (b != NULL) {
                    // We need to fix checkParity and checkLRC becuase they just aren't working when they should...
                    if (checkParity(b) && checkLRC(bmask(b),calculateLRC(buffer)) && parity_flag) {
                        this->send(ACK);
                        this->newState(0,"ONLINE");
                        // A this point, we need to parse the struct that came back and decide what to do.
                        // all we really want to do is find the _nummer field, and link that to the
                        // m_queue[] member variable so that we write the response with the correct id.
                        int nummer = -1;
                        qDebug() << "Buffer at 0" << buffer.at(0);
                        this->buildMessageQuery(buffer);
                        if (buffer.at(0) == 'P') {
                            nummer = QChar::digitValue(buffer.at(60).unicode());
                            this->m_active_id = this->m_queue->value(nummer);
                            emit bytesRead(this->m_active_id,buffer);
                            this->parsePStruct(buffer);
                        } else if (buffer.at(0) == 'M'){
                            nummer = QChar::digitValue(buffer.at(4).unicode());
                            this->m_active_id = this->m_queue->value(nummer);
                            emit bytesRead(this->m_active_id,buffer);
                            this->parseMStruct(buffer);
                        } else if (buffer.at(0) == 'N') {
                            this->m_active_id = this->m_queue->value(nummer);
                            nummer = QChar::digitValue(buffer.at(20).unicode());
                            emit bytesRead(this->m_active_id,buffer);
                        } else if (buffer.at(0) == 'J') {
                            emit bytesRead(this->m_active_id,buffer);
                        } else {
                            this->m_active_id = "000000";
                            emit bytesRead("000000",buffer);
                        }

                        buffer = "";
                    } else {
                        qDebug() << "This is failing...";
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
    qDebug() << "Reading:";
    b = this->m_Reader->read_char();
    qDebug() << "Read...";
    return b;
}
void ArtemaHybrid::send(char b) {

    this->m_Reader->write_char(encodeParity(b));
}
void ArtemaHybrid::sendData(QString id,QString data) {
    qDebug() << "AH Received: " << data;
    this->m_number++;
    if (this->m_number > 9) {
        this->m_number = 0;
    }
    qDebug() << "nummer inced";
    this->m_queue->insert(this->m_number,id);
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
    query += "'" + QString(msg.at(0)) + "',";
    query += "'','','','','','','','','','','','','','','');"; // because we have to insert all fields? Weird...
    emit saveMessage(this->m_active_id,query);

}
void ArtemaHybrid::parsePStruct(QString data) {
    int index;
    QString ind = data.at(3);
    QString p_text;
    for (index = 44; index < 44+16; index++) {
        p_text += data.at(index);
    }
    QString p_ergebnis;
    for (index = 4; index < 4+40; index++) {
        p_ergebnis += data.at(index);
    }
    emit saveMessage(this->m_active_id,update_query("ind", ind, this->m_active_id));

    if (ind == "0" || ind == "7" || ind == "C") {
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
            }

            QString amnt = toNumber(p_text);
            emit paid(this->m_active_id, amnt);
            qDebug() << "amnt is: " << amnt;
            qreal total = amnt.toFloat();
            update_daily_totals("artema_hybrid_total",total);
            emit display(this->m_active_id,p_text);
            emit print(this->m_active_id,p_ergebnis);
        } else {
            qDebug() << "p_ind is 0, yet didnt match up p_text, p_text is: " << p_text;
        }
    } else if (ind == "2") {
        // in this case, this is a p struct from a successfull End of Day (Tagesende)
        QString amnt;
        for (index = 7; index < p_text.length(); index++) {
            amnt += p_text.at(index);
        }
         qreal total = amnt.toFloat();
         QSettings settings("JolieRouge", "CuteCredit");
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
         settings.setValue("artema_hybrid_total",0);
     } else {
        qDebug() << "Oops..." << data.at(3);
     }
}
void ArtemaHybrid::parseMStruct(QString m) {
        QString ind = m.at(3);
        emit saveMessage(this->m_active_id,update_query("ind", ind, this->m_active_id));
        int index;

        QString name;
        for (index = 5; index < 5+20; index++) {
            name += m.at(index);
        }
        emit saveMessage(this->m_active_id,update_query("name", name, this->m_active_id));

        QString card;
        for (index = 25; index < 25+21; index++) {
            card += m.at(index);
        }
        emit saveMessage(this->m_active_id,update_query("card", card, this->m_active_id));

        QString expiry_date;
        for (index = 46; index < 46+5; index++) {
            expiry_date += m.at(index);
        }
        emit saveMessage(this->m_active_id,update_query("expiry_date", expiry_date, this->m_active_id));

        QString uid;
        for (index = 52; index < 52+16; index++) {
            uid += m.at(index);
        }
        emit saveMessage(this->m_active_id,update_query("uid", uid, this->m_active_id));

        QString tid;
        for (index = 68; index < 68+8; index++) {
            tid += m.at(index);
        }
        emit saveMessage(this->m_active_id,update_query("tid", tid, this->m_active_id));

        QString receipt_id;
        for (index = 76; index < 76+6; index++) {
            receipt_id += m.at(index);
        }
        emit saveMessage(this->m_active_id,update_query("receipt_id", receipt_id, this->m_active_id));

        QString authorization_number;
        for (index = 82; index < 82+6; index++) {
            authorization_number += m.at(index);
        }
        emit saveMessage(this->m_active_id,update_query("auth", authorization_number, this->m_active_id));

        QString result;
        for (index = 88; index < 88+20; index++) {
            result += m.at(index);
        }
        emit saveMessage(this->m_active_id,update_query("result", result, this->m_active_id));

        QString total;
        for (index = 108; index < 108+8; index++) {
            total += m.at(index);
        }
        emit saveMessage(this->m_active_id,update_query("total", total, this->m_active_id));


        total = total.trimmed();
        qreal f_total = this->total2qreal(total);
        int n_ind = ind.toInt();
        if (n_ind < 8) {
            // it's a positive transaction
            update_daily_totals("artema_hybrid_total",f_total);
            emit paid(this->m_active_id,cents2float(total));
        } else {
            update_daily_totals("artema_hybrid_total",f_total * -1);
            emit success(this->m_active_id,cents2float(total) + " Refunded.");
        }

        QString text;
        for (index = 130; index < 130+16; index++) {
            text += m.at(index);
        }

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
        QString receipt_text = receipt_lines.join("<CR>");
        emit saveMessage(this->m_active_id,update_query("receipt_text", receipt_text, this->m_active_id));
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
    emit error(this->m_active_id,ind_error);
    emit fail(this->m_active_id,n_text);
}
