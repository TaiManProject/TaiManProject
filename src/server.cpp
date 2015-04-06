#include "server.h"
#include <QtNetwork>

Server::Server(QObject *parent) : QObject(parent),tcpServer(0), networkSession(0)
{
    QNetworkConfigurationManager manager;
    if (manager.capabilities() & QNetworkConfigurationManager::NetworkSessionRequired) {
        // Get saved network configuration
        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        const QString id = settings.value(QLatin1String("DefaultNetworkConfiguration")).toString();
        settings.endGroup();

        // If the saved network configuration is not currently discovered use the system default
        QNetworkConfiguration config = manager.configurationFromIdentifier(id);
        if ((config.state() & QNetworkConfiguration::Discovered) !=
                QNetworkConfiguration::Discovered) {
            config = manager.defaultConfiguration();
        }
        networkSession = new QNetworkSession(config, this);
        connect(networkSession, SIGNAL(opened()), this, SLOT(sessionOpened()));
        networkSession->open();
    } else {
        sessionOpened();
    }

    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

Server::~Server()
{

}

void Server::sessionOpened()
{
    // Save the used configuration
    if (networkSession) {
        QNetworkConfiguration config = networkSession->configuration();
        QString id;
        if (config.type() == QNetworkConfiguration::UserChoice)
            id = networkSession->sessionProperty(QLatin1String("UserChoiceConfiguration")).toString();
        else
            id = config.identifier();

        QSettings settings(QSettings::UserScope, QLatin1String("QtProject"));
        settings.beginGroup(QLatin1String("QtNetwork"));
        settings.setValue(QLatin1String("DefaultNetworkConfiguration"), id);
        settings.endGroup();
    }

    tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::Any, 54321)) {
        qDebug() << QString("Unable to start the server: %1.").arg(tcpServer->errorString());
        return;
    }
    QString ipAddress;

    // if we did not find one, use IPv4 localhost
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    qDebug() << QString(tr("The server is running on\n\nIP: %1\nport: %2")
                        .arg(ipAddress).arg(tcpServer->serverPort()));
}

void Server::newConnection(){
    bytesReceived = 0;
    clientConnection  = tcpServer->nextPendingConnection();
    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater()));
    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(rcveData()));
    qDebug() << "newConnection.";
}

void Server::rcveData(){
    qDebug() << "rcveData";
    QDataStream in(clientConnection);
    QByteArray inBlock;
    if(bytesReceived == 0){
        in >> totalSize;
        qDebug() << totalSize;
        totalSize += sizeof(int);
        bytesReceived += sizeof(int);
        rcveFile = new QFile(this->picName);
        if(rcveFile->open(QFile::WriteOnly)){

        } else {
            qDebug() << "rcveFailed.";
        }
    }
    if(bytesReceived == sizeof(int)) {
        if(clientConnection->bytesAvailable() < sizeof(int))
            return;
        in  >> isSketch;
        qDebug() << isSketch;
//        in >> isSketch;
//        qDebug() << isSketch;
        totalSize += sizeof(int) ;
        bytesReceived += sizeof(int);
    }
    if(bytesReceived < totalSize){
        bytesReceived += clientConnection->bytesAvailable();
        inBlock = clientConnection->readAll();
        rcveFile->write(inBlock);
        inBlock.resize(0);
    }
    qDebug() << bytesReceived << totalSize;
    if(bytesReceived >= totalSize){
        qDebug() << "receive success.";
        rcveFile->close();
        emit receivePicture(isSketch, this->picName);
    }
}

void Server::sendPicture(Result result) {
    qDebug() << "sendSynPhoto";
    sendFile = new QFile(QString::fromStdString(result.synPhoto));
    connect(clientConnection, SIGNAL(bytesWritten(qint64)), this, SLOT(updateClientProgress(qint64)));
    if(!sendFile->open(QFile::ReadOnly)){
        qDebug() << "Failed to open File";
    } else {
        totalSendBytes = sendFile->size();
        qDebug() << totalSendBytes;
        bytesWritten = 0;
        bytesToWrite = 0;
        outBlock.resize(0);
        QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
        sendOut << totalSendBytes;
        clientConnection->write(outBlock, outBlock.size());
        bytesToWrite = totalSendBytes ;
        outBlock.resize(0);
    }
    qDebug() << "send over";
}

void Server::sendPerson() {
    if (this->sendNum >= sendPersonNum) {
        return;
    }
    totalSendBytes = 0;
    bytesWritten = 0;
    bytesToWrite = 0;
    qDebug() << "sendPerson " << this->sendNum;
    Person p = sendPersonList.at(this->sendNum);
    sendFile = new QFile(QString::fromStdString(p.getPhotoPath()));
    if(!sendFile->open(QFile::ReadOnly)) {
        qDebug() << "Failed to open File:";// << QString.fromStdString(p.getPhotoPath());
    } else {
        totalSendBytes = sendFile->size();
        qDebug() << totalSendBytes;
        QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
//        QByteArray msg = QString::fromStdString(p.getInfo()).toUtf8();
        QByteArray msg = QByteArray(p.getInfo().c_str());
        qDebug() << msg.size() << msg<< totalSendBytes;
        sendOut /*<< msg.size() */<< msg<< totalSendBytes;
        clientConnection->write(outBlock, outBlock.size());
        bytesToWrite = totalSendBytes ;
        outBlock.resize(0);
    }
}

void Server::sendResult(Result result) {
    this->sendPersonList = result.persons;
    this->sendPersonNum = this->sendPersonList.size();
    this->sendNum = -1;
    QDataStream sendOut(&outBlock, QIODevice::WriteOnly);
    sendOut << this->sendPersonNum;
    clientConnection->write(outBlock, outBlock.size());
    outBlock.resize(0);
    qDebug() << "sendResult init. sendNum is" << this->sendNum;
    sendPicture(result);
}


void Server::updateClientProgress(qint64 bytes){
    qDebug() << "updateClientProgress()";
    if(totalSendBytes == 0)
        return;
    bytesWritten += (int) bytes;
    qDebug() << bytesWritten << totalSendBytes << bytesToWrite;
    if (bytesToWrite > 0)
    {
        outBlock = sendFile->read(qMin(bytesToWrite, loadSize));
        bytesToWrite -= (int)clientConnection->write(outBlock,outBlock.size());
        outBlock.resize(0);
    }
    if (bytesToWrite <= 0)
    {
        sendFile->close();
        this->sendNum++;
        sendPerson();
    }
}
