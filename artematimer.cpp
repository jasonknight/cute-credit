#include "artematimer.h"
#include "cute_credit.h"
ArtemaTimer::ArtemaTimer(QObject *parent, int interval, QString name) :
    QObject(parent)
{
    this->Timer = new QTimer(0);
    this->Timer->connect(this->Timer,SIGNAL(timeout()),this,SLOT(_timeOut()));
    this->p_timeout = false;
    this->p_times_ran = 0;
    this->p_interval = interval;
    this->m_name = name;
}
void ArtemaTimer::start() {
    this->p_timeout = false;
    this->Timer->start(this->p_interval);
}
void ArtemaTimer::restart() {
    this->p_timeout = false;
    this->Timer->start(this->p_interval);;
}

void ArtemaTimer::_timeOut() {
    this->p_timeout = true;
    this->p_times_ran++;
    this->Timer->stop();
}
bool ArtemaTimer::timeOut() {
    return this->p_timeout;
}
