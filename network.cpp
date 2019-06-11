#include "network.h"
#include <QNetworkInterface>
#include <QNetworkAddressEntry>

Network::Network()
{

}

void Network::serachServer()
{
    socket = new QTcpSocket();
    connect(socket, &QTcpSocket::connected, this, &Network::connected);
    foreach (QNetworkInterface i, QNetworkInterface().allInterfaces()){
        foreach (QNetworkAddressEntry ae, i.addressEntries()){
            if (ae.ip().protocol() == QAbstractSocket::IPv4Protocol && ae.ip() != QHostAddress(QHostAddress::LocalHost)){
                QHostAddress ip = ae.ip();
                QHostAddress nm = ae.netmask();
                qDebug() << "Host IP: " << ip.toString();
                qDebug() << "Netmask: " << nm.toString();
                quint32 nip = ip.toIPv4Address() & nm.toIPv4Address();
                qDebug() << "Network: " << QHostAddress(nip).toString();

                connect(socket, &QTcpSocket::readyRead, this, &Network::readMessage);
                while (nip < (nip | quint32(255))){
                    QHostAddress address = QHostAddress(nip++);
//                    if (address == ip) continue;
                    socket->connectToHost(address, 22222);
                    qDebug() << "Connect to host: " << address.toString();
                    if (socket->waitForConnected(10)){
                        qDebug() << "Connection success!";
                        break;
                    }
                }
                socket->disconnect(socket, SLOT(readyRead));
                emit noHostAvailable();
            }
        }
    }
}

void Network::createServer()
{
    server = new QTcpServer();
    server->listen(QHostAddress::Any, 22222);
    connect(server, &QTcpServer::newConnection, [=](){
        QTcpSocket *client = new QTcpSocket();
        connect(client, &QTcpSocket::readyRead, this, &Network::readMessage);
        connect(client, &QTcpSocket::connected, [=](){socketList.append(client);});
        connect(client, &QTcpSocket::disconnected, [=](){socketList.removeOne(client);});
        client = server->nextPendingConnection();
        emit connected();
    });
}

void Network::readMessage()
{
    QByteArray message;
    emit newMessage(message);
}

void Network::sendMessage(QByteArray)
{

}
