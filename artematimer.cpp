#include "artematimer.h"
#include "cute_credit.h"
#include <QDateTime>
ArtemaTimer::ArtemaTimer(QObject *parent, int interval, QString name) :
    QObject(parent)
{
    this->p_timeout = true;
    this->p_times_ran = 0;
    this->p_interval = interval;
    this->m_name = name;
    this->p_running = false;
}
void ArtemaTimer::start() {
    this->p_running = true;
    this->p_timeout = false;
    QDateTime now = QDateTime::currentDateTime();
    int now_t = now.toTime_t();
    this->p_start_time = now_t;
}
void ArtemaTimer::restart() {
    this->p_running = true;
    //qDebug() << "restarted " << this->m_name;
    this->p_timeout = false;
    this->start();
}

void ArtemaTimer::_timeOut() {
    this->p_timeout = true;
    emit timedOut();
    //this->p_times_ran++;
}
int ArtemaTimer::timeRemaining() {
    QDateTime now = QDateTime::currentDateTime();
    int diff = now.msecsTo(QDateTime::fromTime_t(this->p_start_time)) * -1;
    if (this->p_interval - diff < 0)
        this->_timeOut();
    return this->p_interval - diff;
}
void ArtemaTimer::kill() {
    this->p_running = false;
    this->p_timeout = true;

    emit timedOut();
}
bool ArtemaTimer::timeOut() {
    if (this->p_timeout)
        return true;
    QDateTime now = QDateTime::currentDateTime();
    int diff = now.msecsTo(QDateTime::fromTime_t(this->p_start_time)) * -1;
    if (this->m_name == "FailTimer") {
        //qDebug() << "Difference: " << QString::number(diff) << " for " << this->m_name;
    }
    if (diff >= this->p_interval) {
            this->_timeOut();
    }
    return this->p_timeout;
}
