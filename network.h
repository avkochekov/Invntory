#ifndef NETWORK_H
#define NETWORK_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class Network : public QObject
{
    Q_OBJECT
public:
    explicit Network();
    void sendMessage();

private:
    QTcpServer *server;
    QTcpSocket *socket;
    bool socetConnected;
    QList<QTcpSocket *> socketList;

signals:
    void noHostAvailable();
    void connected();
    void newMessage(QByteArray);

public slots:
    void serachServer();
    void createServer();

    void readMessage();
    void sendMessage(QByteArray);
};

#endif // NETWORK_H
