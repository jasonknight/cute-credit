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
    QString             m_name;
private:
    bool                p_timeout;
    int                 p_times_ran;
    int                 p_interval;
    QTimer *            Timer;
signals:

public slots:
    bool                timeOut(); // Return true|false depending on p_timeout value
private slots:
    void                _timeOut();

};

#endif // ARTEMATIMER_H
