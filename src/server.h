#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QFile>
#include<opencv/cv.h>
#include"result.h"

QT_BEGIN_NAMESPACE
class QTcpServer;
class QNetworkSession;
class QTcpSocket;
QT_END_NAMESPACE

class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = 0);
    ~Server();

protected:
    QString picName = "../data/rcve.jpg";
    const int loadSize = 1024 * 4;

private slots:
    void sessionOpened();
    void rcveData();
    void newConnection();
//    void sendPicture(Result result);
    void updateClientProgress(qint64 bytes);
    void sendResult(Result result);
    void sendPerson();
    void faceNotFound();

private:
    QTcpServer *tcpServer;
    QNetworkSession *networkSession;
    QTcpSocket *clientConnection;
    int totalSize;
    int isSketch;
    int bytesReceived;
    QFile *rcveFile;
    QFile *sendFile;
    int totalSendBytes;
    int bytesWritten;
    int bytesToWrite;
    QByteArray outBlock;
    std::vector<Person> sendPersonList;
    int sendNum;
    int sendPersonNum;

signals:
    void receivePicture(int isSketch, QString picName);
};

#endif // SERVER_H
