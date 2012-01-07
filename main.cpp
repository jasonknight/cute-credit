#include <QtCore/QCoreApplication>
#include <QSettings>
#include <QProcess>
#include "helpers.h"
#include "cute_credit.h"
#include "mock.h"
#include "artemahybrid.h"
#include "database.h"
int main(int argc, char *argv[])
{
    QString n = "N00A*hjkloijjjhghjklk1";
    parse_n_struct(n);
    return 0;
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
    Database * d = new Database(0);
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
