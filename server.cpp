#include <QJsonDocument>
#include <QJsonObject>

#include "server.h"

ServerData::ServerData()
{
    slotlists.append(new SlotList(0, 9, 10 * 1000, 3));
    slotlists.append(new SlotList(1, 24, 300 * 1000, 4));
    slotlists.append(new SlotList(2, 12, 3600 * 1000, 6));
}

ServerData::~ServerData()
{
    for (int i = 0; i < slotlists.size(); i++)
        delete slotlists.at(i);
}

Server::Server(QObject *parent) : QTcpServer(parent)
{
    connect(&tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(tcpError(QAbstractSocket::SocketError)));

    connect(&tcpSocket, SIGNAL(readyRead()), this, SLOT(tcpReady()));

    tcpSocket.setSocketOption(QAbstractSocket::KeepAliveOption, true);

    receivedLen = -1;
}

Server::~Server()
{
    tcpSocket.disconnectFromHost();
}

void Server::open(int port, QString secret, QString sim)
{
    serverSecret = secret;
    serverSim = sim;

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
    if (isOpen) {
        tcpSocket.close();
        QTcpServer::close();
    }

    hasError = false;
    isOpen = false;
}

void Server::tcpReady()
{
    QString data = tcpSocket.readAll();

    if (data.size() < 1)
    {
       tcpSocket.close();
       return;
    }

    int index;

    if (receivedLen < 0) {
        if ((index = data.indexOf("Content-Length: ")) > 0) {
            data.remove(0, index + 16);

            index = data.indexOf("\r\n");
            receivedLen = data.mid(0, index).toInt();

            data.remove(0, index + 4);
            receivedData = data;
        } else {
            tcpSocket.close();
            emit dataReceived(nullptr);
            return;
        }
    } else {
        receivedData.append(data);
    }

    // qDebug() << receivedData;

    ServerData *serverData = nullptr;

    if (receivedData.size() >= receivedLen) {
        receivedLen = -1;

        QString response = "HTTP/1.1 200 OK\r\n";

        QJsonDocument doc = QJsonDocument::fromJson(receivedData.toUtf8().data());
        QJsonObject obj = doc.object();

        // qDebug() << doc.toJson(QJsonDocument::Indented).toStdString().c_str();

        if (obj.value("secret").toString().compare(serverSecret) != 0) {
            response.append("Content-Length: 12\r\nConnection: close\r\n"
                "Content-Type: text/html; charset=UTF-8\r\n\r\nWrong secret");
            emit dataReceived(nullptr);
        } else {
            response.append("Content-Length: 2\r\nConnection: close\r\n"
                "Content-Type: text/html; charset=UTF-8\r\n\r\n[]");

            QString iStr;

            for (int i = 0; iStr = QString::number(i), obj.contains(iStr); i++) {
                QJsonObject card = obj.value(iStr).toObject();

                if (card.value("type").toString().compare("push") == 0) {

                    if (serverData != nullptr)
                        delete serverData;
                    serverData = new ServerData();

                    serverData->caption = card.value("caption").toString();

                    QStringList data = card.value("data").toString().split(":");

                    long long lastChange = data.value(0).toLongLong();
                    long long lastUpdate = data.value(1).toLongLong();
                    long long current = data.value(2).toLongLong();
                    long long floor = data.value(3).toLongLong();
                    bool hasLimit = data.value(4).toStdString().compare("1") == 0;
                    long long limit = data.value(5).toLongLong();
                    bool hasUsedWarning = data.value(6).toStdString().compare("1") == 0;
                    long long usedWarning = data.value(7).toLongLong();
                    long long usedLastSeen = data.value(8).toLongLong();
                    bool hasRemainWarning = data.value(9).toStdString().compare("1") == 0;
                    long long remainWarning = data.value(10).toLongLong();
                    long long remainLastSeen = data.value(11).toLongLong();

                    serverData->lastChange = lastChange;
                    serverData->lastUpdate = lastUpdate;

                    serverData->used = current - floor;

                    if (serverData->used < 0)
                        serverData->used = 0;

                    if (hasLimit) {
                        serverData->limit = limit;
                        serverData->remaining = limit - serverData->used;

                        if (serverData->remaining < 0)
                            serverData->remaining = 0;

                    } else {
                        serverData->limit = 0;
                        serverData->remaining = 0;
                    }

                    if (hasUsedWarning)
                        serverData->usedWarning = usedWarning;
                    else
                        serverData->usedWarning = 0;

                    serverData->usedLastSeen = usedLastSeen;

                    if (hasRemainWarning)
                        serverData->remainWarning = remainWarning;
                    else
                        serverData->remainWarning = 0;

                    serverData->remainLastSeen = remainLastSeen;

                    QJsonObject slotsObj = card.value("slots").toObject();

                    for (int j = 0; j < serverData->slotlists.count(); j++)
                    {
                        for (int k = 0; k < serverData->slotlists[j]->count(); k++)
                        {
                            QString index = QString::number(j) + ":" + QString::number(k);
                            QStringList values = slotsObj.value(index).toString().split(":");

                            if (values.count() == 3)
                                serverData->slotlists.data()[j]->update(k,
                                        values[0].toLongLong(), values[1].toLongLong(), values[2].toLongLong());
                        }
                    }
                }
            }
        }

        tcpSocket.write(response.toLocal8Bit());
        tcpSocket.close();
    }

    if (serverData != nullptr)
        emit dataReceived(serverData);
}

void Server::tcpError(QAbstractSocket::SocketError error)
{
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

