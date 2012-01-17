#ifndef ARTEMATIMER_H
#define ARTEMATIMER_H

#include <QObject>
#include <QTimer>
class ArtemaTimer : public QObject
{
    Q_OBJECT
public:
    explicit            ArtemaTimer(QObject *parent = 0, int interval = 0, QString name = "Anonymous");
    void                start();
    void                restart();
    int                 timeRemaining();
    bool                isRunning() {
        return this->p_running;
    }
    void                kill();
    QString             m_name;
private:
    bool                p_timeout;
    bool                p_running;
    int                 p_times_ran;
    int                 p_interval;
    int                 p_start_time;
    QTimer *            Timer;
signals:
    void timedOut();
public slots:
    bool                timeOut(); // Return true|false depending on p_timeout value
private slots:
    void                _timeOut();

};

#endif // ARTEMATIMER_H
