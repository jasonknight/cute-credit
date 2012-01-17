#include "reader.h"
#include "cute_credit.h"

Reader::Reader(QObject *parent, QString path, int mode, bool keepalive) :
    QObject(parent)
{
    this->m_path = path;
    this->m_mode = mode;
    this->m_keepalive = keepalive;
    if (this->m_keepalive) {
        this->m_descriptor = openPort(path,mode);
        qDebug() << path << " opened descriptor is: " << QString::number(this->m_descriptor);
    }
    this->has_read = false;
}
QString Reader::read_string(int len) {
    if (!this->m_keepalive) {
        qDebug() << "Opening: " << this->m_path;
        this->m_descriptor = openPort(this->m_path,this->m_mode);
    }
    QString ret;
    char * buffer = (char *)malloc(len * sizeof(char) + 1);
    if (buffer) {
        strcpy(buffer,"");
        int rb = read(this->m_descriptor,buffer,len);
        int i;
        for (i = 0; i < strlen(buffer); i++) {
            if (buffer[i] == 0x04) { // i.e. we have to write EOT, end of transmission, when we communicate via the fifo
                break;
            } else {
                ret += buffer[i];
            }
        }
        free(buffer);
    } else {
        ret = "";
    }
    if (!this->m_keepalive) {
        qDebug() << "Closing: " << this->m_path;
        close(this->m_descriptor);
    }
    return ret;
}
bool Reader::write_string(QString data) {
    if (!this->m_keepalive) {
        qDebug() << "Opening: " << this->m_path;
        this->m_descriptor = openPort(this->m_path,this->m_mode);
    }
    write(this->m_descriptor,data.toAscii().data(),data.length());
    qDebug() << "Wrote: " << data;
    if (!this->m_keepalive) {
        qDebug() << "Closing: " << this->m_path;
        close(this->m_descriptor);
    }
    return true;
}
bool Reader::write_string(const char *data) {
    if (!this->m_keepalive) {
        qDebug() << "Opening: " << this->m_path;
        this->m_descriptor = openPort(this->m_path,this->m_mode);
    }
    write(this->m_descriptor,data,strlen(data));
    qDebug() << "Wrote charz: " << data;
    if (!this->m_keepalive) {
        qDebug() << "Closing: " << this->m_path;
        close(this->m_descriptor);
    }
    return true;
}

bool Reader::write_char(char b) {
    char buffer[1];
    int i;
    buffer[0] = b;
    //printf(" %X (%c) ",bmask(b),bmask(b));
    i = write(this->m_descriptor, buffer,1);
    return true;
}
char Reader::read_char() {
    char buffer[1];
    //read(this->m_descriptor,buffer,1);
    //return buffer[0];
    if (this->m_keepalive) {
        fd_set f;
        FD_ZERO(&f);
        FD_SET(this->m_descriptor,&f);
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100;
        int i = select(this->m_descriptor + 1, &f,NULL,NULL,&tv);
        if (i>0) {
            if (FD_ISSET(this->m_descriptor,&f)) {
                read(this->m_descriptor,buffer,1);
                this->has_read = true;
            } else {
                this->has_read = false;
                return NULL;
            }
        } else {
            this->has_read = false;
            return NULL;
        }
    } else {
        read(this->m_descriptor,buffer,1);
        this->has_read = true;
    }
    return buffer[0];
}
void Reader::configure(QString type) {
    if (type == "ArtemaHybrid") {
        configureArtemaHybridPort(this->m_descriptor);
    }
}
