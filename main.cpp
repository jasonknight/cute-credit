#include <QtCore/QCoreApplication>
#include <QSettings>
#include <QProcess>
#include <QSqlQuery>
#include "helpers.h"
#include "cute_credit.h"
#include "mock.h"
#include "artemahybrid.h"
#include "database.h"
void help() {
    qDebug() << "Showing Help";
}
QString parse_arg(QString arg) {
    int index = arg.indexOf("=")+1;
    QString value;
    while (index < arg.length()) {
        //if (arg.at(index) != '"') {
            value += arg.at(index);
        //}
        index++;
    }
    return value;
}
void export_database(int argc, char *argv[],Database * d) {
    QString path = "/var/log";
    QString filename = "cute_credit.tsv";
    QString sep = "\t";
    QString arg;
    QString where = "1";
    for (int i = 1; i < argc; i++) {
      arg = QString(argv[i]);
          if (arg.indexOf("--path") != -1) {
            path = parse_arg(arg);
          }
          if (arg.indexOf("--filename") != -1) {
            filename = parse_arg(arg);
          }
          if (arg.indexOf("--sep") != -1) {
            sep = parse_arg(arg);
            if (sep == "semicolon")
                sep = ";";
            if (sep == "colon")
                sep = ":";
            if (sep == "tab")
                sep = "\t";
            if (sep == "comma")
                sep = ",";
          }
          if (arg.indexOf("--where") != -1) {
            where = parse_arg(arg);
          }
    }
    QString query = "SELECT * FROM messages WHERE " + where + ";";
    qDebug() << query;
    QSqlQuery q = d->runQuery(query);

}

int main(int argc, char *argv[])
{
    QSettings settings("JolieRouge", "CuteCredit");
    QCoreApplication a(argc, argv);
    int pid = settings.value("pid").toInt();
    int m_pid = getppid();
    qDebug() << "PID is: " << QString::number(pid);
    qDebug() << "PID is: " << QString::number(m_pid);
    if (pid != 0 && pid != m_pid) {
        qDebug() << "PIDS don't match" << QString::number(m_pid) << " != " << QString::number(pid);
        #ifdef LINUX
            kill(pid,9);
        #endif

    }
    settings.setValue("pid",m_pid);

    // Now we need to handle cmdline arguments
    Database * d = new Database(0); // we need the database at this point, in case we need to query
    QString arg;
    for (int i = 1; i < argc; i++) {
      arg = QString(argv[i]);
      if (arg == "-h") {
        help();
        return 0;
      }
      if (arg == "--export") {
        export_database(argc, argv,d);
        return 0;
      }
    }
    /*
        At this point we need to create two FIFO files, /tmp/CUTE_CREDIT_IN and /tmp/CUTE_CREDIT_OUT
        the _IN file is where other programs write to send data to the program, and the _OUT
        is where we will write the outcome of the transaction.

        Please see the document in the spec/ directory for an explanation of client/server
        communication!
    */
#ifdef LINUX
    umask(0);
    mknod("/tmp/CUTE_CREDIT_IN",S_IFIFO|0666,0);
    mknod("/tmp/CUTE_CREDIT_OUT",S_IFIFO|0666,0);
#endif
    /*
        Now we need an object to sit and read from the FIFOS to direct the communication
    */

    FIFOController * f = new FIFOController(0);
    ArtemaHybrid * ah = new ArtemaHybrid(0,"/dev/ttyUSB0");
    // We connect the FIFO Controller to the Hybrid sendData
    ah->connect(f,SIGNAL(send(QString,QString)),SLOT(sendData(QString,QString)));
    // we connect the database to the saveMessage
    d->connect(ah,SIGNAL(saveMessage(QString,QString)),SLOT(runQuery(QString,QString)));
    // we connect the fifo controller to the artema hybrid machine...Eventually this will
    // be selectable...
    //f->connect(ah,SIGNAL(bytesRead(QString,QString)),SLOT(recvMsg(QString,QString)));
    f->connect(ah,SIGNAL(display(QString,QString)),SLOT(display(QString,QString)));
    f->connect(ah,SIGNAL(print(QString,QString)),SLOT(print(QString,QString)));
    f->connect(ah,SIGNAL(paid(QString,QString)),SLOT(paid(QString,QString)));
    f->connect(ah,SIGNAL(fail(QString,QString)),SLOT(fail(QString,QString)));
    f->connect(ah,SIGNAL(success(QString,QString)),SLOT(success(QString,QString)));
    f->connect(ah,SIGNAL(error(QString,QString)),SLOT(error(QString,QString)));
    f->connect(ah,SIGNAL(notify(QString,QString)),SLOT(notify(QString,QString)));
    // we connect the database to the Fifo
    f->connect(d,SIGNAL(queryComplete(QString,QSqlQuery)), SLOT(queryComplete(QString, QSqlQuery)));
    d->connect(f,SIGNAL(runQuery(QString,QString)),SLOT(runQuery(QString,QString)));

#ifdef TESTING
    Mock * m = new Mock(0);
    m->connect(f,SIGNAL(send(QString)),SLOT(recv_qstring(QString)));
    m->connect(ah,SIGNAL(bytesRead(QString)),SLOT(recv_qstring(QString)));
    m->connect(ah,SIGNAL(dataSent(QString)),SLOT(sent_qstring(QString)));
#endif
    f->start();
    ah->start();
    a.exec();
    return 0;
}
