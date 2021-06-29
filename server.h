#ifndef SERVER_H
#define SERVER_H

#include <QTcpServer>
#include <QTcpSocket>

struct ServerData
{
    bool active;
    long rxtime;

    long long used;
    long long limit;
};

class Server : public QTcpServer
{
    Q_OBJECT

public:
    explicit Server(QObject *parent = nullptr);
    ~Server();

    bool isOpen = false;
    bool hasError = false;

    void open(int port, QString secret, QString sim);
    void close();

private:
    QTcpSocket tcpSocket;

    QString serverSecret;
    QString serverSim;

    QString receivedData;
    int receivedLen;

signals:
    void serverError(QString message);
    void dataReceived(ServerData *data);

public slots:
    void tcpReady();
    void tcpError(QAbstractSocket::SocketError error);

protected:
    void incomingConnection(qintptr descriptor);
};


#endif // SERVER_H
