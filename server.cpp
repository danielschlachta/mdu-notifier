#include "server.h"

Server::Server(QObject *parent) : QTcpServer(parent)
{
    connect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(tcpError(QAbstractSocket::SocketError)));

    connect(&tcpSocket, SIGNAL(readyRead()), this, SLOT(tcpReady()));

    tcpSocket.setSocketOption(QAbstractSocket::KeepAliveOption, true);

    lastReception.start();
}

Server::~Server()
{
    tcpSocket.disconnectFromHost();
}

void Server::open(int port)
{
    if (isOpen)
        return;

    if(!this->listen(QHostAddress::Any, static_cast<quint16>(port)))
    {
        hasError = true;
        emit serverError(tr("Unable to listen to port."));
    }
    else
        isOpen = true;
}

void Server::close()
{
    if (isOpen)
        QTcpServer::close();

    hasError = false;
}

long long getValue(QStringList stringList, int index, const char *ckey)
{
    QString key(ckey);
    key += QString::number(index) + "=";

    for (int i = 0; i < stringList.size(); i++)
    {
        QString string = stringList.value(i);

        if (string.left(key.length()) == key)
            return string.right(string.length() - key.length()).toLongLong();
    }

    return -1;
}

void Server::tcpReady()
{
    QStringList data = tr(tcpSocket.read(tcpSocket.bytesAvailable())).split(" ");

    if (data.size() < 1)
        return;

    QStringList request = data.value(1).split("&");

    if (request.value(0) != "/app.php?s=hoot")
        return;

    ServerData serverData;

    serverData.timeElapsed = lastReception.elapsed() / 1000;
    serverData.usedBytes = getValue(request, 1, "rx") - getValue(request, 2, "rx")
            + getValue(request, 1, "tx") - getValue(request, 2, "tx") + getValue(request, 3, "tx");
    serverData.capBytes = getValue(request, 3, "rx");
    serverData.capTime = getValue(request, 3, "t") / 1000;
    serverData.transmitInterval = static_cast<int>(getValue(request, 4, "rx"));
    serverData.warningThreshold = static_cast<int>(getValue(request, 4, "tx"));
    serverData.displayIEC =getValue(request, 5, "rx");

    lastReception.restart();

    tcpSocket.flush();
    tcpSocket.close();

    emit dataReceived(serverData);
}

void Server::tcpError(QAbstractSocket::SocketError error)
{
    qDebug("Socket error: %s", tcpSocket.errorString().toLocal8Bit().data());

    if (error != QAbstractSocket::RemoteHostClosedError)
    {
        hasError = true;
        close();
        emit serverError(tcpSocket.errorString());
    }
}

void Server::incomingConnection(qintptr descriptor)
{
    tcpSocket.setSocketDescriptor(descriptor); // fail silently
}

