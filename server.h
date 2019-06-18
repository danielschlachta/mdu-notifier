#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QTime>

struct ServerData
{
    long timeElapsed;

    bool displayIEC;
    int warningThreshold;
    int transmitInterval;

    long long usedBytes;
    long long capBytes;
    long capTime;
};

class Server : public QTcpServer
{
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);
    ~Server();

    bool isOpen = false;
    bool hasError = false;
    QTime lastReception;

    void open(int port);
    void close();

private:
    QTcpSocket tcpSocket;

signals:
    void serverError(QString message);
    void dataReceived(ServerData data);

public slots:
    void tcpReady();
    void tcpError(QAbstractSocket::SocketError error);

protected:
    void incomingConnection(qintptr descriptor);
};


#endif // SERVER_H