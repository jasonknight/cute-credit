#ifndef MOCK_H
#define MOCK_H

#include <QObject>
#include <QDebug>
class Mock : public QObject
{
    Q_OBJECT
public:
    explicit Mock(QObject *parent = 0);

signals:

public slots:
    void recv_qstring(QString data) {
        qDebug() << "Mock::recv_qstring received: " << data;
    }
    void sent_qstring(QString data) {
        qDebug() << "Mock::sent_qstring received: " << data;
    }

};

#endif // MOCK_H
