#ifndef HELPERS_H
#define HELPERS_H
#include <QSettings>
#include <QString>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <signal.h>
#include <QStringList>
#include <QDebug>
#include <math.h>


static char encodeParity(char b);
    #ifdef LINUX

static int openPort(QString path, int mode = O_RDWR) {
    int fd;
    fd = open(path.toAscii().data(),mode | O_NOCTTY | O_NDELAY | O_NONBLOCK);
    if (fd == -1) {
        printf("open_port: failed to open %s", path.toAscii().data());
    } else {
        fcntl(fd,F_SETFL,0);
    }
    return fd;
}
static void configureArtemaHybridPort(int fd) {
    struct termios options;
    tcgetattr(fd, &options); // Get the current options for the port...
    cfsetispeed(&options, B19200); // Set the baud rates
    cfsetospeed(&options, B19200);
    //printf("c_cflag = %X \n", options.c_cflag);
    options.c_cflag &= ~CSIZE;
    //printf("c_cflag = %X \n", options.c_cflag);
    options.c_cflag |= ( CLOCAL | CREAD | CS8 ); // Enable the receiver and set local mode...  | CS7 | PARENB
    //printf("c_cflag = %X \n", options.c_cflag);
    tcsetattr(fd, TCSANOW, &options); // Set the new options for the port...

}
static void configureFIFO(int fd) {
    fd_set set;
    struct timeval timeout;
    FD_ZERO (&set);
    FD_SET (fd, &set);
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    TEMP_FAILURE_RETRY (select (FD_SETSIZE,
                              &set, NULL, NULL,
                              &timeout));
}
    #endif
// Non Linux Specific Functions
static char bmask(char b){
    char c = b;
    //qDebug() << QString::number(b);
    c &= 0b01111111;
    //qDebug() << QString::number(b);
    return c;
}
static char calculateLRC(QString s) {
    qDebug() << "Calculating LRC";
    QByteArray ba = s.toAscii();
    char lrc;
    int i = 0;
    while (i < ba.size()) {
        lrc ^= ba.at(i);
        i++;
    }
    lrc ^= 0x03;
    return lrc;
}
static bool checkLRC(char lrc1,char lrc2) {
    // needs to be fixed...
    qDebug() << "checkLrc: " << (lrc1 == lrc2);
    return (lrc1 == lrc2);
}
static bool checkParity(char b) {
    int nob = 8;
    int i;
    int counter = 0;
    char bb = 1;
    for (i = 0; i < nob; i++) {
        if (b & bb) {
            counter++;
        }
        bb = bb << 1;
    }
    return (counter % 2 == 0);
}
static char encodeParity(char b) {
    if (!checkParity(b)) {
        b |= 0b10000000;
    }
    return b;
}
static void update_daily_totals(QString index, qreal total) {
    QSettings settings("JolieRouge", "CuteCredit");
    qDebug() << "Received: " << index << " + " << QString::number(total);
    qreal current_total = settings.value(index).toReal();
    settings.setValue(index,current_total + total);

    qreal trans_total = settings.value("trans_total").toReal();
    settings.setValue("trans_total",trans_total + total);

    qDebug() << index << " is: " << settings.value(index).toString();
}
//QString s = "M1121Visa Credit         **** **** **** 5894  10/167300311358    20107970113000086000000B E Z A H L T       0000329500000000000000BEZAHLT!        056511EA0000000031010               C03";
static void parse_m_struct(QString m) {
    QChar ind = m.at(3).unicode();
    int number = QChar::digitValue(m.at(60).unicode());
    int index;
    QString name;
    for (index = 5; index < 5+20; index++) {
        name += m.at(index);
    }
    qDebug() << "Name: " << name;
    QString card;
    for (index = 25; index < 25+21; index++) {
        card += m.at(index);
    }
    qDebug() << "Card: " << card;
    QString expiry_date;
    for (index = 46; index < 46+5; index++) {
        expiry_date += m.at(index);
    }
    qDebug() << "Exp: " << expiry_date;
    QString uid;
    for (index = 52; index < 52+16; index++) {
        uid += m.at(index);
    }
    qDebug() << "uid: " << uid;
    QString tid;
    for (index = 68; index < 68+8; index++) {
        tid += m.at(index);
    }
    qDebug() << "tid: " << tid;
    QString receipt_id;
    for (index = 76; index < 76+6; index++) {
        receipt_id += m.at(index);
    }
    qDebug() << "receipt_id: " << receipt_id;
    QString authorization_number;
    for (index = 82; index < 82+6; index++) {
        authorization_number += m.at(index);
    }
    qDebug() << "auth: " << authorization_number;
    QString result;
    for (index = 88; index < 88+20; index++) {
        result += m.at(index);
    }
    QString total;
    for (index = 108; index < 108+8; index++) {
        total += m.at(index);
    }
    QString text;
    for (index = 130; index < 130+16; index++) {
        text += m.at(index);
    }
    QString transactions_remaining;
    for (index = 146; index < 146+4; index++) {
        transactions_remaining += m.at(index);
    }
    qDebug() << "trans: " << transactions_remaining;
    QString signature_required = m.at(150);
    qDebug() << "Signature Required: " << signature_required;
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
    qDebug() << "ergebnis: " << result << " betrag: " << total << " text: " << text << " Receipt text: " << receipt_text;

}
static void parse_l_struct(QString l) {
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
    QString text;
    for (index = 5; index < 5+16; index++) {
        text += l.at(index);
    }
    QString l_display;
    for (index = 22; index < 22+20; index++) {
        l_display += l.at(index);
    }
}
static void parse_n_struct(QString n) {
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
    qDebug() << "ind_error: " << ind_error;
    qDebug() << "n_text: " << n_text;
}
static QString update_query(QString field, QString value, QString id, QString sa) {
    QString query = "UPDATE messages SET FIELD = VALUE WHERE ref_id = 'REFID' AND sa = 'IND';";
    query = query.replace("FIELD",field);
    value = value.replace("'","&squo;");
    query = query.replace("VALUE","'"+value+"'");
    query = query.replace("REFID",id);
    query = query.replace("IND",sa);
    qDebug() << query;
    return query;
}
static QString update_query(QString field, int value, QString id, QString sa) {
    QString query = "UPDATE messages SET FIELD = VALUE WHERE ref_id = 'REFID' AND sa = 'IND';";
    query = query.replace("FIELD",field);
    query = query.replace("VALUE",QString::number(value));
    query = query.replace("REFID",id);
    query = query.replace("IND",sa);
    return query;
}
static QString update_query(QString field, QString value, QString id) {
    QString query = "UPDATE messages SET FIELD = VALUE WHERE ref_id = 'REFID';";
    query = query.replace("FIELD",field);
    value = value.replace("'","&squo;");
    query = query.replace("VALUE","'"+value+"'");
    query = query.replace("REFID",id);
    qDebug() << query;
    return query;
}
static QString update_query(QString field, int value, QString id) {
    QString query = "UPDATE messages SET FIELD = VALUE WHERE ref_id = 'REFID';";
    query = query.replace("FIELD",field);
    query = query.replace("VALUE",QString::number(value));
    query = query.replace("REFID",id);
    return query;
}
static bool eql(qreal q1, qreal q2) {
    return (QString::number(q1) == QString::number(q2));
}
static QString toCents(QString s) {
    int i;
    QString allowed = "0123456789";
    QString new_s;
    for (i = 0; i < s.length();i++) {
        if (allowed.indexOf(s.at(i)) != -1) {
            new_s += s.at(i);
        }
    }
    return new_s;
}
static QString toNumber(QString s) {
    int i;
    QString last_char = "";
    QString allowed = ".,";
    QString nums = "0123456789";
    QString new_s;
    for (i = 0; i < s.length();i++) {
        // We concat only if it is in the allowed chars and the last_char seen is a number, in case we are looking at a period.
        // if we didn't, then numbes like Hello World.3.6, would render a return of: .3.5, which would be incorrect.
        if (nums.indexOf(s.at(i)) != -1) {
            new_s += s.at(i);
        } else if (nums.indexOf(last_char) != -1 && allowed.indexOf(s.at(i)) != -1 && i != s.length() - 1) {
           new_s += s.at(i);
       }
        last_char = s.at(i);
    }
    // now we need to convert it to precision 2
    QString s2l = new_s.at(new_s.length()-2);
    QString last = new_s.at(new_s.length()-1);
    if (nums.indexOf(last) != -1 && allowed.indexOf(s2l) != -1) {
        new_s += "0";
    }
    return new_s.replace(",",".");
}
static QString cents2float(QString cents) {
    qreal t = cents.toFloat() / 100;
    return QString::number(t);
}
static QStringList split_lines_at_space(QString line, int max_chars) {
  QStringList lines;
  // Verify that the line is longer than max_chars.
  if (line.length() < max_chars) {
    lines.append(line);
    return lines;
  }
  int head = max_chars;
  QString c_line;
  QString new_line;
  int i;
  while (head > 0) {
   if (line.at(head) == ' ') {
      // yay, we've found the split point.
      for (i = 0; i < head+1; i++) {
        c_line += line.at(i);
      }
      qDebug() << "Appending: " << c_line;
      lines.append(c_line);
      for (i = head; i < line.length(); i++) {
        new_line += line.at(i);
      }
      if (new_line.length() > 0 ) {
        lines.append(split_lines_at_space(new_line,max_chars));
      }
      return lines;
   }
   head--;
  }
  qDebug() << "Failed to split lines.";
  lines.append(line);
  return lines;
}
#endif // HELPERS_H
