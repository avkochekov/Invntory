#include "network.h"
#include <QNetworkInterface>
#include <QNetworkAddressEntry>

Network::Network()
{
    socket = new QTcpSocket();
    server = new QTcpServer();

    connect(socket, &QTcpSocket::readyRead, this, &Network::readMessage);

}

void Network::serachServer()
{
    socket->disconnectFromHost();
    foreach (QNetworkInterface i, QNetworkInterface().allInterfaces()){
        foreach (QNetworkAddressEntry ae, i.addressEntries()){
            if (ae.ip().protocol() == QAbstractSocket::IPv4Protocol && ae.ip() != QHostAddress(QHostAddress::LocalHost)){
                QHostAddress ip = ae.ip();
                QHostAddress nm = ae.netmask();
                qDebug() << "Host IP: " << ip.toString();
                qDebug() << "Netmask: " << nm.toString();
                quint32 nip = ip.toIPv4Address() & nm.toIPv4Address();
                qDebug() << "Network: " << QHostAddress(nip).toString();

                while (nip < (nip | quint32(255))){
                    QHostAddress address = QHostAddress(nip++);
//                    if (address == ip) continue;
                    socket->connectToHost(address, 22222);
                    qDebug() << "Connect to host: " << address.toString();
                    if (socket->waitForConnected(10)){
                        qDebug() << "Connection success!";
                        emit connected();
                        return;
                    }
                }
                socket->disconnect();
                emit noHostAvailable();
            }
        }
    }
}

void Network::createServer()
{
    server->listen(QHostAddress::Any, 22222);
    connect(server, &QTcpServer::newConnection, [=](){
        connect(socket, &QTcpSocket::readyRead, this, &Network::readMessage);
        connect(socket, &QTcpSocket::disconnected, [=](){
            socket = new QTcpSocket();
            socket->disconnect();
        });
        socket = server->nextPendingConnection();
        emit connected();
    });
}

void Network::readMessage()
{
    incomingDataStream.setVersion(QDataStream::Qt_5_9);
    socket->waitForBytesWritten();
    QByteArray data;
    incomingDataStream.startTransaction();
    incomingDataStream >> data;

    qDebug() << "[RECEIVED PART] " << data << endl;

    if (!incomingDataStream.commitTransaction())
        return;
    qDebug() << "[RECEIVE] " << data << endl;
    emit newMessage(data);
}

void Network::sendMessage(QByteArray data)
{
    if(socket->isWritable()) {
        qDebug() << "[SEND] " << socket << data << endl;
        QByteArray block;
        QDataStream m_outcomingMessage(&block, QIODevice::WriteOnly);
        m_outcomingMessage.setVersion(QDataStream::Qt_5_9);
        m_outcomingMessage << data;
        socket->write(block);
        socket->flush();
    }
}
