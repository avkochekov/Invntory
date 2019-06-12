#include "network.h"
#include <QNetworkInterface>
#include <QNetworkAddressEntry>

Network::Network()
{
    /// Создает создает объекты server и socket для передачи данных по локальной сети
    /// Подключает необходимые для передачи данных сигналы и слоты
    socket = new QTcpSocket();
    server = new QTcpServer();

    connect(socket, &QTcpSocket::readyRead, this, &Network::readMessage);
    connect(server, &QTcpServer::newConnection, [=](){
        socket = server->nextPendingConnection();
        connect(socket, &QTcpSocket::readyRead, this, &Network::readMessage);
        emit connected();
    });
}

void Network::disconnect()
{
    /// Прерывает все ранее созданные соединения
    socket->disconnectFromHost();
    server->close();
}

void Network::serachServer()
{
    /// Перебирает IP адреса локальной сети
    /// Первый подключенный хост записывается в socket
    /// Если ни по одному адресу нет хоста, к которому можно подключиться
    /// подает сигнал noHostAvailable

    socket->disconnectFromHost();
    foreach (QNetworkInterface i, QNetworkInterface().allInterfaces()){
        foreach (QNetworkAddressEntry ae, i.addressEntries()){
            if (ae.ip().protocol() == QAbstractSocket::IPv4Protocol && ae.ip() != QHostAddress(QHostAddress::LocalHost)){
                QHostAddress localIp = ae.ip();
                QHostAddress netmask = ae.netmask();
                quint32 networkIp = localIp.toIPv4Address() & netmask.toIPv4Address();
                quint32 addressCount = localIp.toIPv4Address() & !netmask.toIPv4Address() - 2;
                qDebug() << "Host IP: " << localIp.toString();
                qDebug() << "Netmask: " << netmask.toString();
                qDebug() << "Network: " << QHostAddress(networkIp).toString();
                qDebug() << "Possibe hosts: " << QHostAddress(addressCount).toString();

                while (networkIp < (networkIp | addressCount)){
                    QHostAddress address = QHostAddress(networkIp++);
//                    if (address == ip) continue;
                    socket->connectToHost(address, 22222);
                    qDebug() << "Connect to host: " << address.toString();
                    if (socket->waitForConnected(10)){
                        qDebug() << "Connection success!";
                        emit connected();
                        return;
                    }
                }
                emit noHostAvailable();
            }
        }
    }
}

void Network::createServer()
{
    /// Запускает прослушку порта 22222
    server->listen(QHostAddress::Any, 22222);
}

void Network::readMessage()
{
    /// При получении нового сообщения подает сигнал newMessage с данными
    emit newMessage(socket->readAll());
}

void Network::sendMessage(QByteArray data)
{
    /// Пересылает сообщение на сервер/подключенный хост

    if(socket->isWritable()) {
        socket->write(data);
        socket->flush();
    }
}
