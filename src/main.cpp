#include "QBorderMainWindow.h"
#include "server.h"
#include <QCoreApplication>

int main(int argc, char *argv[]) {
    commonTool.log("Starting FaceMatch application.");
    QCoreApplication a(argc, argv,false);
    QBorderMainWindow* w = new QBorderMainWindow();
    Server* server = new Server();
	srand ( time(NULL) );
    QObject::connect(server, SIGNAL(receivePicture(int, QString)), w, SLOT(findFace(int, QString)));
    QObject::connect(w, SIGNAL(faceFound(Result)), server, SLOT(sendResult(Result)));
    QObject::connect(w, SIGNAL(faceNotFound()), server, SLOT(faceNotFound()));
	return a.exec();
}
